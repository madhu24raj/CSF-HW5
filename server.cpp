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

  std::stringer username = conn.data;

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
    pass;
  } else {
    pass;
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
}
