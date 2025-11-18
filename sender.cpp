#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  Connection conn;

  // TODO: connect to server
  conn.connect(server_hostname, server_port);

  if (!conn.is_open()) {
    std::cerr << "Failed to connect to server\n";
    return 1;
  }


  // TODO: send slogin message


  Message slogin(TAG_SLOGIN, username);
  if (!conn.send(slogin)) {
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


  // TODO: loop reading commands from user, sending messages to
  //       server as appropriate

  return 0;
}
