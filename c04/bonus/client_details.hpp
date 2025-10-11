#pragma once

#include <random>
#include <string>

enum Client_State {
  CONNECTED,
  SENT_NUM,
  WRONG,
  ERROR,
  DONE,
};

class Client_Details {
public:
  /*Returns message to sent back.
   * WARN: Side effect: May change member variable mActive.*/
  std::string handle_message(const char message[]);
  Client_Details() {}
  ~Client_Details() = default;

  bool is_active() { return mActive; }

private:
  Client_State mState = CONNECTED;
  int mNum = -1;
  bool mActive = true;
  std::default_random_engine gen{std::random_device{}()};
  std::uniform_int_distribution<int> dist{1, 100};
};
