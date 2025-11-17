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

  if (!is_open()) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  //A message must be a single line of text with no newline characters contained within it.
  // need to use the rio_readlineb
  // ssize_t	rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

  size_t maxlen = Message::MAX_LEN + 1;
  char* usrbuf = new char[maxlen]; //c string has null terminator too so need +1

  ssize_t n_read = rio_readlineb(&m_fdbuf, usrbuf, maxlen);

  if (n_read <= 0) {
    m_last_result = EOF_OR_ERROR; //EOF for 0 and error for -1
    return false;
  }

  std::string line(usrbuf);

  //buffer looks like "tag:data\n\0"
  while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
    line.pop_back();
  }

  //split the line into tag and data
  size_t colon_pos = line.find(':');
  if (colon_pos == std::string::npos) {
    m_last_result = INVALID_MSG;
    return false;
  }

  msg.tag = line.substr(0, colon_pos);
  msg.data = line.substr(colon_pos + 1);
  
  m_last_result = SUCCESS;
  return true;
}
