//
// Created by Martin Økter on 29/05/2024.
//

#ifndef VIDEO_STREAM_INCLUDE_STATE_H_
#define VIDEO_STREAM_INCLUDE_STATE_H_

class IceStateMachine;

// Base class for states
class State {
 public:
  virtual ~State() {}
  virtual void Enter(IceStateMachine& state_machine){};
  virtual void Execute(IceStateMachine& state_machine) = 0;
  virtual void Exit(IceStateMachine& state_machine){};
};


#endif //VIDEO_STREAM_INCLUDE_STATE_H_
