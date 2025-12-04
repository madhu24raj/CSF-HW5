#include "guard.h"
#include "message.h"
#include "message_queue.h"
#include "user.h"
#include "room.h"

Room::Room(const std::string &room_name)
  : room_name(room_name) {
  // TODO: initialize the mutex
  pthread_mutex_init(&lock, nullptr);
}

Room::~Room() {
  // TODO: destroy the mutex
  pthread_mutex_destroy(&lock);
}

void Room::add_member(User *user) {
  // TODO: add User to the room
  pthread_mutex_lock(&lock);
  members.insert(user);
  pthread_mutex_unlock(&lock);
}

void Room::remove_member(User *user) {
  // TODO: remove User from the room
  pthread_mutex_lock(&lock);
  members.erase(user);
  pthread_mutex_unlock(&lock);
}

void Room::broadcast_message(const std::string &sender_username, const std::string &message_text) {
  // TODO: send a message to every (receiver) User in the room
  pthread_mutex_lock(&lock);
  std::string payload = room_name + ":" + sender_username + ":" + message_text;
  for (User* user: members) {
    Message* message = new Message(TAG_DELIVERY, payload);
    user->mqueue.enqueue(message);
  }
  pthread_mutex_unlock(&lock);

}
