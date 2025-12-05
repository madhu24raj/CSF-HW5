#include <pthread.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <set>
#include <vector>
#include <cctype>
#include <cassert>
#include "message.h"
#include "connection.h"
#include "user.h"
#include "room.h"
#include "guard.h"
#include "server.h"

////////////////////////////////////////////////////////////////////////
// Server implementation data types
////////////////////////////////////////////////////////////////////////

// TODO: add any additional data types that might be helpful
//       for implementing the Server member functions

struct workerValues {
  Server* server;
  int client_fd;
};

////////////////////////////////////////////////////////////////////////
// Client thread functions
////////////////////////////////////////////////////////////////////////

namespace {

void *worker(void *arg) {
  pthread_detach(pthread_self());

  // TODO: use a static cast to convert arg from a void* to
  //       whatever pointer type describes the object(s) needed
  //       to communicate with a client (sender or receiver)
  workerValues* args = static_cast<workerValues*>(arg);
  Server* server = args->server;
  int client_fd = args->client_fd;
  delete args;
  // TODO: read login message (should be tagged either with
  //       TAG_SLOGIN or TAG_RLOGIN), send response
  Connection conn(client_fd);

  Message login; 
  if (!conn.receive(login)) {
    return nullptr;
  }

  std::string username = login.data;

  if (login.tag != TAG_SLOGIN && login.tag != TAG_RLOGIN) {
    Message error_msg = Message(TAG_ERR, "invalid login");
    conn.send(error_msg);
    return nullptr;
  }

  Message accepted_msg = Message(TAG_OK, "login successful");
  conn.send(accepted_msg);


  // TODO: depending on whether the client logged in as a sender or
  //       receiver, communicate with the client (implementing
  //       separate helper functions for each of these possibilities
  //       is a good idea)
  if (login.tag == TAG_SLOGIN) {
    server->receiver_chat(conn, username);
  } else {
    return;
  }

  return nullptr;
}


}

////////////////////////////////////////////////////////////////////////
// Server member function implementation
////////////////////////////////////////////////////////////////////////

Server::Server(int port)
  : m_port(port)
  , m_ssock(-1) {
  // TODO: initialize mutex
  pthread_mutex_init(&m_lock, nullptr);
}

Server::~Server() {
  // TODO: destroy mutex
  pthread_mutex_destroy(&m_lock);
  for (std::pair<const std::string, Room*> &p: m_rooms) {
    delete p.second;
  }
  if (m_ssock >= 0) {
    close(m_ssock);
  }
}

bool Server::listen() {
  // TODO: use open_listenfd to create the server socket, return true
  //       if successful, false if not
  std::string port_str = std::to_string(m_port);
  const char* port_cstr = port_str.c_str();
  m_ssock = open_listenfd(port_cstr);
  if (m_ssock < 0) {
    return false;
  }
  return true;

  
}

void Server::handle_client_requests() {
  // TODO: infinite loop calling accept or Accept, starting a new
  //       pthread for each connected client
}

Room *Server::find_or_create_room(const std::string &room_name) {
  // TODO: return a pointer to the unique Room object representing
  //       the named chat room, creating a new one if necessary
  pthread_mutex_lock(&m_lock);
  // look to see if room exists
  for (const std::pair<const std::string, Room*> &p: m_rooms) {
    Room* room = p.second;
    // if found, unlock and return
    if (room->get_room_name() == room_name) {
      pthread_mutex_unlock(&m_lock);
      return room;
    }
  }
  // create new room
  Room* room = new Room(room_name);
  // add new room to the RoomMap
  m_rooms[room_name] = room;
  // unlock
  pthread_mutex_unlock(&m_lock);
  return room;

}

void Server::receiver_chat(Connection &conn, const std::string &username) {
  User* user = new User(username);
  Room* curr_room = nullptr;
  Message msg;
  if (!conn.receive(msg)) {
    delete user;
    return;
  }
  if (msg.tag != TAG_JOIN) {
    Message error_msg = Message(TAG_ERR, "expected join");
    conn.send(error_msg);
  }
  std::string room_name = msg.data;
  Room* room = find_or_create_room(room_name);
  room->add_member(user);
  curr_room = room;
  Message accepted_msg = Message(TAG_OK, "joined");
  conn.send(accepted_msg);
  while (1) {
    // gets new message
    Message *new_msg = user->mqueue.dequeue();
    if (new_msg == nullptr) {
      continue;
    }
    // send to receiver
    bool result = conn.send(*new_msg);
    delete new_msg;
    // receiver send failed
    if (!result) {
      // check if current room to remove
      if (curr_room) {
        curr_room->remove_member(user);
      }
      delete user;
      return;
    }
  }

}

void Server::send_chat(Connection &conn, const std::string &username) {
  User* user = new User(username);
  Room* curr_room = nullptr;

  while (1) {
    Message msg;
    // check if we receive message
    if (!conn.receive(msg)) {
      // check if current room exists so we can remove the user
      if (curr_room) {
        curr_room->remove_member(user);
      }
      return;
    }

    // check for join
    if (msg.tag == TAG_JOIN) {
      std::string room_name = msg.data;
      // leave current room if exists
      if (curr_room) {
        curr_room->remove_member(user);
      }
      // join new room
      curr_room = find_or_create_room(room_name);
      curr_room->add_member(user);
      Message accepted_msg = Message(TAG_OK, "joined");
      conn.send(accepted_msg);
      continue;
    }

    // check for leave
    if (msg.tag == TAG_LEAVE) {
      // check if not in a room 
      if (!curr_room) {
        Message error_msg = Message(TAG_ERR, "not in a room");
        conn.send(error_msg);
        continue;
      }
    }

    // check for sendall to users
    if (msg.tag == TAG_SENDALL) {
      // check if not in a room
      if (!curr_room) {
        Message error_msg = Message(TAG_ERR, "not in a room");
        conn.send(error_msg);
        continue;
      } else {
        // broadcast send message
        curr_room->broadcast_message(username, msg.data);
        Message accepted_msg = Message(TAG_OK,"sent");
        conn.send(accepted_msg);
        continue;
      }

    }

    // check for sender wanting to quit
    if (msg.tag == TAG_QUIT) {
      // remove user from room if exists
      if (curr_room) {
        curr_room->remove_member(user);
      }
      delete user;
      Message accepted_msg = Message(TAG_OK, "bye");
      conn.send(accepted_msg);
      return;
    }

    // some unknown error
    Message unknown_err = Message(TAG_ERR, "invalid command");
    conn.send(unknown_err);
  }
}