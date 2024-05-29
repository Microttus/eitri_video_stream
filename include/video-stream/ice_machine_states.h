//
// Created by Martin Økter on 29/05/2024.
//

#ifndef VIDEO_STREAM_SRC_ICE_MACHINE_STATES_H_
#define VIDEO_STREAM_SRC_ICE_MACHINE_STATES_H_


#include "ice_state_machine.h"
#include "state.h"

// Define the Unconfigured state
class Unconfigured : public State {
 public:
  void Enter(IceStateMachine& state_machine) override;
  void Execute(IceStateMachine& state_machine) override;
  void Exit(IceStateMachine& state_machine) override;
};

// Define the Active state
class Activate : public State {
 public:
  void Enter(IceStateMachine& state_machine) override;
  void Execute(IceStateMachine& state_machine) override;
  void Exit(IceStateMachine& state_machine) override;
};

// Define the Streaming state
class Streaming : public State {
 public:
  void Enter(IceStateMachine& state_machine) override;
  void Execute(IceStateMachine& state_machine) override;
  void Exit(IceStateMachine& state_machine) override;
};

// Define the Deactivate state
class Deactivate : public State {
 public:
  void Enter(IceStateMachine& state_machine) override;
  void Execute(IceStateMachine& state_machine) override;
  void Exit(IceStateMachine& state_machine) override;
};

// Define the Shutdown state
class Shutdown : public State {
 public:
  void Enter(IceStateMachine& state_machine) override;
  void Execute(IceStateMachine& state_machine) override;
  void Exit(IceStateMachine& state_machine) override;
};

// Define the StartUp state
class StartUp : public State {
 public:
  void Enter(IceStateMachine& state_machine) override;
  void Execute(IceStateMachine& state_machine) override;
  void Exit(IceStateMachine& state_machine) override;
};

#endif //VIDEO_STREAM_SRC_ICE_MACHINE_STATES_H_
