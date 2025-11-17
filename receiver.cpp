#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

struct ParsedDelivery {

  std::string room;
  std::string sender;
  std::string message;
  bool valid = false;
}



ParsedDelivery parse_delivery_payload(const std::string& payload) {
  ParsedDelivery result;

  std::vector<std::string> parts = split(payload, ':');
  
  if (parts.size() < 3) {
      result.valid = false;
      return result;
  }
  
  result.room = parts[0];
  result.sender = parts[1];
  
  // reconstruc msg from remaining parts
  for (size_t i = 2; i < parts.size(); i++) {
      if (i > 2) result.message += ":";
      result.message += parts[i];
  }
  
  result.valid = true;
  return result;
}

int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  Connection conn;

  // TODO: connect to server
  conn.connect(server_hostname, server_port);

  if (!conn.is_open()) {
    std::cerr << "Failed to connect to server\n";
    return 1;
  }


  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)

  Message rlogin(TAG_RLOGIN, username);
  if (!conn.send(rlogin)) {
    std::cerr << "Failed to send rlogin message\n";
    return 1;
  }

  Message response1;
  if (!conn.receive(response1)) {
    std::cerr << "Failed to receive response for rlogin message\n";
    return 1;
  }

  if (response1.tag != TAG_OK) {
    if (response1.tag == TAG_ERR) {
      std::cerr << response1.data << "\n";
      return 1;
    } 
    else {
      std::cerr << "Unexpected response for rlogin message" << response1.tag << "\n";
      return 1;
    }
  }

  Message join(TAG_JOIN, room_name);
  if (!conn.send(join)) {
    std::cerr << "Failed to send join message\n";
    return 1;
  }

  Message response2;
  if (!conn.receive(response2)) {
    std::cerr << "Failed to receive response for join message\n";
    return 1;
  }

  if (response2.tag != TAG_OK) {
    if (response2.tag == TAG_ERR) {
      std::cerr << response2.data << "\n";
      return 1;
    } 
    else {
      std::cerr << "Unexpected response for join message" << response2.tag << "\n";
      return 1;
    }
  }

  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)

  while(1) {
    Message delivery;
    if (!conn.receive(delivery)) {
      break; //normal breaks in connection, could be voluntary
    }

    if (delivery.tag == TAG_DELIVERY) {
      ParsedDelivery delivery = parse_delivery_payload(delivery_msg.data);
      if (delivery.valid) {
        std::cout << delivery.sender << ": " << delivery.message << "\n";
      } else {
        std::cerr << "Error: Failed to parse delivery message payload\n";
      }
      
    }

  }


  return 0;
}
