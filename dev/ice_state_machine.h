//
// Created by Martin Økter on 29/05/2024.
//

#ifndef VIDEO_STREAM_SRC_ICESTATEMACHINE_HH_
#define VIDEO_STREAM_SRC_ICESTATEMACHINE_HH_

#include "state.h"

class IceStateMachine {
 public:
  IceStateMachine(State* initial_state);
  ~IceStateMachine();

  void Execute();
  void ChangeState(State* new_state);

  virtual bool OnUnconfigured() = 0;
  virtual bool OnStreaming() = 0;
  virtual bool OnShutdown() = 0;

  virtual bool OnActivate() = 0;
  virtual bool OnDeactivate() = 0;
  virtual bool OnStartUp() = 0;

 private:
  State* current_state_;
};

#endif //VIDEO_STREAM_SRC_ICESTATEMACHINE_HH_
