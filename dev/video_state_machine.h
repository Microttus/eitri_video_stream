//
// Created by Martin Økter on 29/05/2024.
//

#ifndef VIDEO_STREAM_SRC_VIDEO_STATE_MACHINE_H_
#define VIDEO_STREAM_SRC_VIDEO_STATE_MACHINE_H_

#include "ice_state_machine.h"
#include "ice_machine_states.h"

class VideoStateMachine : public IceStateMachine {
 public:
  VideoStateMachine() : IceStateMachine(new StartUp()) { }
  ~VideoStateMachine() = default;

  bool OnUnconfigured() override;
  bool OnStreaming() override;
  bool OnShutdown() override;
  bool OnActivate() override;
  bool OnDeactivate() override;
  bool OnStartUp() override;

};

#endif //VIDEO_STREAM_SRC_VIDEO_STATE_MACHINE_H_
