//
// Created by Martin Økter on 29/05/2024.
//

#include "ice_machine_states.h"

void Unconfigured::Enter(IceStateMachine &state_machine) {

}
void Unconfigured::Execute(IceStateMachine& state_machine) {
  bool success = state_machine.OnUnconfigured();
  if (success) {
    state_machine.ChangeState(new Activate());
  }
}
void Unconfigured::Exit(IceStateMachine& state_machine) {
}



void Activate::Enter(IceStateMachine& state_machine) {
  //std::cout << "Entering Activate" << std::endl;
}
void Activate::Execute(IceStateMachine& state_machine) {
  //std::cout << "Executing Activate" << std::endl;
  state_machine.OnActivate();
  state_machine.ChangeState(new Streaming());
}
void Activate::Exit(IceStateMachine& state_machine) {
  //std::cout << "Exiting Activate" << std::endl;
}



void Streaming::Enter(IceStateMachine& state_machine) {
  //std::cout << "Entering Streaming" << std::endl;
}
void Streaming::Execute(IceStateMachine& state_machine) {
  //std::cout << "Executing Streaming" << std::endl;
  bool success = state_machine.OnStreaming();
  if (success) {
    state_machine.ChangeState(new Deactivate());
  }
}
void Streaming::Exit(IceStateMachine& state_machine) {
  //std::cout << "Exiting Streaming" << std::endl;
}


void Deactivate::Enter(IceStateMachine& state_machine) {
  //std::cout << "Entering Deactivate" << std::endl;
}
void Deactivate::Execute(IceStateMachine& state_machine) {
  //std::cout << "Executing Deactivate" << std::endl;
  state_machine.OnDeactivate();
  state_machine.ChangeState(new Unconfigured());
}
void Deactivate::Exit(IceStateMachine& state_machine) {
  //std::cout << "Exiting Deactivate" << std::endl;
}


void StartUp::Enter(IceStateMachine& state_machine) {
  //std::cout << "Entering Deactivate" << std::endl;
}
void StartUp::Execute(IceStateMachine& state_machine) {
  //std::cout << "Executing Deactivate" << std::endl;
  state_machine.OnStartUp();
  state_machine.ChangeState(new Unconfigured());
}
void StartUp::Exit(IceStateMachine& state_machine) {
  //std::cout << "Exiting Deactivate" << std::endl;
}



void Shutdown::Enter(IceStateMachine& state_machine) {
  //std::cout << "Entering Shutdown" << std::endl;
}
void Shutdown::Execute(IceStateMachine& state_machine) {
  //std::cout << "Executing Shutdown" << std::endl;
  state_machine.OnShutdown();
}
void Shutdown::Exit(IceStateMachine& state_machine) {
  //std::cout << "Exiting Shutdown" << std::endl;
}