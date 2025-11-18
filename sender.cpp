#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

// Display prompt
void display_prompt() {
  std::cout << "> ";
  std::cout.flush();
}

// Get and validate user input
// Returns true if input was successfully read, false if EOF/error
bool get_user_input(std::string& user_input) {
  if (!std::getline(std::cin, user_input)) {
      return false; // EOF or error reading input
  }
  
  user_input = trim(user_input);
  return true;
}


// Parse command and argument from input
bool parse_command(const std::string& user_input, std::string& command, std::string& argument) {
  if (user_input.empty() || user_input[0] != '/') {
      return false;
  }
  
  std::istringstream iss(user_input);
  iss >> command;
  
  // Get the rest as argument (optional)
  std::getline(iss >> std::ws, argument);
  return true;
}

// Handle join command
bool handle_join_command(Connection& conn, const std::string& room_name, std::string& current_room) {
  if (room_name.empty()) {
      std::cerr << "Error: /join requires a room name\n";
      return false;
  }
  
  // Send join message
  Message join_msg(TAG_JOIN, room_name);
  if (!conn.send(join_msg)) {
      std::cerr << "Failed to send join message\n";
      return false;
  }
  
  // Receive response
  Message join_response;
  if (!conn.receive(join_response)) {
      std::cerr << "Failed to receive join response\n";
      return false;
  }
  
  // Process response
  if (join_response.tag == TAG_ERR) {
      std::cerr << join_response.data << "\n";
      return false;
  } else if (join_response.tag == TAG_OK) {
      current_room = room_name;
      return true;
  } else {
      std::cerr << "Unexpected response to join: " << join_response.tag << "\n";
      return false;
  }
}


// Handle leave command
bool handle_leave_command(Connection& conn, std::string& current_room) {
  if (current_room.empty()) {
      std::cerr << "Error: Not currently in a room\n";
      return false;
  }
  
  // Send leave message (payload is ignored but colon required)
  Message leave_msg(TAG_LEAVE, "");
  if (!conn.send(leave_msg)) {
      std::cerr << "Failed to send leave message\n";
      return false;
  }
  
  // Receive response
  Message leave_response;
  if (!conn.receive(leave_response)) {
      std::cerr << "Failed to receive leave response\n";
      return false;
  }
  
  // Process response
  if (leave_response.tag == TAG_ERR) {
      std::cerr << leave_response.data << "\n";
      return false;
  } else if (leave_response.tag == TAG_OK) {
      current_room.clear();
      return true;
  } else {
      std::cerr << "Unexpected response to leave: " << leave_response.tag << "\n";
      return false;
  }
}

// Handle quit command
bool handle_quit_command(Connection& conn, bool& should_quit) {
  // Send quit message (payload is ignored but colon required)
  Message quit_msg(TAG_QUIT, "");
  if (!conn.send(quit_msg)) {
      std::cerr << "Failed to send quit message\n";
      return false;
  }
  
  // Receive response
  Message quit_response;
  if (!conn.receive(quit_response)) {
      std::cerr << "Failed to receive quit response\n";
      return false;
  }
  
  // Process response
  if (quit_response.tag == TAG_ERR) {
      std::cerr << quit_response.data << "\n";
      return false;
  } else if (quit_response.tag == TAG_OK) {
      should_quit = true;
      return true;
  } else {
      std::cerr << "Unexpected response to quit: " << quit_response.tag << "\n";
      return false;
  }
}

// Handle sending a regular message to the current room
bool handle_send_message(Connection& conn, const std::string& current_room, const std::string& message) {
  if (current_room.empty()) {
      std::cerr << "Error: Not currently in a room. Use /join first.\n";
      return false;
  }
  
  Message sendall_msg(TAG_SENDALL, message);
  if (!conn.send(sendall_msg)) {
      std::cerr << "Failed to send message\n";
      return false;
  }
  
  // Receive response
  Message sendall_response;
  if (!conn.receive(sendall_response)) {
      std::cerr << "Failed to receive sendall response\n";
      return false;
  }
  
  // Process response
  if (sendall_response.tag == TAG_ERR) {
      std::cerr << sendall_response.data << "\n";
      return false;
  } else if (sendall_response.tag == TAG_OK) {
      return true;
  } else {
      std::cerr << "Unexpected response to sendall: " << sendall_response.tag << "\n";
      return false;
  }
}


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

  bool should_quit = false;
  std::string current_room; 

  while (!should_quit) {
    //  prompt carrot , make sure its not in a buffer
    display_prompt();
    
    std::string user_input;
    if (!get_user_input(user_input)) {
        break; // EOF or error
    }
    
    if (user_input.empty()) {
        continue; // Skip empty lines
    }

    // case 1: commands start with /
    if (user_input[0] == '/') {
      std::string command, argument;
      if (parse_command(user_input, command, argument)) {
        if (command == "/join") {

          handle_join_command(conn, argument, current_room);
        } 
        else if (command == "/leave") {
          handle_leave_command(conn, current_room);
        }
        else if (command == "/quit") {
          handle_quit_command(conn, should_quit);
        } 
        else {
          std::cerr << "Error: Unknown command '" << command << "'\n";
          std::cerr << "Valid commands: /join, /leave, /quit\n";
        }
      } 
      else {
        std::cerr << "Error: Invalid command format\n";
      }
    } 
    else {
      // regular message
      handle_send_message(conn, current_room, user_input);
    }
  }



  return 0;
}
