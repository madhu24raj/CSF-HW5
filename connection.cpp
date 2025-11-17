#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
    // m_fdbuf not initialized bc no valid file desc yet 
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  // TODO: call rio_readinitb to initialize the rio_t object

  rio_readinitb(&m_fdbuf, m_fd);

}

void Connection::connect(const std::string &hostname, int port) {
  // TODO: call open_clientfd to connect to the server
  //int open_clientfd(const char *hostname, const char *port);

  //convert everything to proper formal
  std::string port_str = std::to_string(port);
  const char* port_cstr = port_str.c_str();
  const char* hostname_cstr = hostname.c_str();

  m_fd = open_clientfd(hostname_cstr, port_cstr);

  if (m_fd < 0) {
    m_last_result = EOF_OR_ERROR;
    return;
  }


  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, m_fd);

  m_last_result = SUCCESS;

}

Connection::~Connection() {
  // TODO: close the socket if it is open
  close();
}

bool Connection::is_open() const {
  // TODO: return true if the connection is open
  return !(m_fd < 0);
}

void Connection::close() {
  // TODO: close the connection if it is open
  if (is_open()) {
    Close(m_fd);
    m_fd = -1;
  }
}

bool Connection::send(const Message &msg) {
  // TODO: send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if (!is_open()) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  //ssize_t rio_writen(int fd, const void *usrbuf, size_t n);
  // message format is tag:payload\n
  std::string msgstr = msg.tag + ":" + msg.data + "\n";

  const char* usrbuf = msgstr.c_str();
  size_t n = msgstr.size();

  if (n > Message::MAX_LEN) {
    m_last_result = INVALID_MSG;
    return false;
  }

  ssize_t n_written = rio_writen(m_fd, usrbuf, n);
  if (n_written != n) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  m_last_result = SUCCESS;
  return true;


}

bool Connection::receive(Message &msg) {
  // TODO: receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  


  
}
