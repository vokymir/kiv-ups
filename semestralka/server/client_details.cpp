#include "client_details.hpp"
#include <cstddef>
#include <cstdlib>
#include <random>
#include <string>

std::string trim_newline(const std::string &s);

std::string Client_Details::handle_message(const char message[]) {
  std::string msg(message);
  msg = trim_newline(msg);

  switch (mState) {
  case CONNECTED:
    if (msg == "HELLO") {
      mNum = dist(gen);
      mState = SENT_NUM;
      return "NUM:" + std::to_string(mNum) + "\n";
    } else {
      mState = ERROR;
      mActive = false;
      return "ERROR\n";
    }

  case SENT_NUM:
    try {
      int client_num = std::stoi(msg);
      if (client_num == mNum * 2) {
        mState = DONE;
        mActive = false;
        return "OK\n";
      } else {
        mState = WRONG;
        mActive = false;
        return "WRONG\n";
      }
    } catch (...) {
      mState = ERROR;
      mActive = false;
      return "ERROR\n";
    }

  case WRONG:
  case ERROR:
  case DONE:
    mActive = false;
    return std::string("FATAL ERROR: Do not ask me anymore.\n");
    break;
  }

  mActive = false;
  return std::string("FATAL ERROR: Should not even happen.\n");
}

std::string trim_newline(const std::string &s) {
  size_t end = s.find_last_not_of("\r\n\0");
  if (end == std::string::npos)
    return "";
  return s.substr(0, end + 1);
}
