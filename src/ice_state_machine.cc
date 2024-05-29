//
// Created by Martin Økter on 29/05/2024.
//

#include "../include/video-stream/ice_state_machine.h"


IceStateMachine::IceStateMachine(State* initial_state) : current_state_(initial_state) {}

IceStateMachine::~IceStateMachine() {
  if (current_state_) {
    delete current_state_;
  }
}

void IceStateMachine::Execute() {
  if (current_state_) {
    current_state_->Execute(*this);
  }
}

void IceStateMachine::ChangeState(State* new_state) {
  if (current_state_) {
    current_state_->Exit(*this);
    delete current_state_;
  }
  current_state_ = new_state;
  if (current_state_) {
    current_state_->Enter(*this);
  }
}