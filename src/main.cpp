#include <iostream>
#include <csignal>
#include <atomic>

#include "../include/video-stream/video_streamer.h"
#include "../include/video-stream/ice_state_machine.h"

std::atomic<bool> shutdown_requested(false);

inline void StopHandler(int) {
  shutdown_requested.store(true);
}

inline void SetupSignalHandlers() {
  std::signal(SIGINT, StopHandler);
  std::signal(SIGTERM, StopHandler);
}

int main(int argc, char* argv[]) {

  std::cout << "Started Video-Streaming!" << std::endl;

  // Create video stream state machine object pointer
  std::unique_ptr<VideoStreamer> video_streamer_ = std::make_unique<VideoStreamer>();

  SetupSignalHandlers();

  video_streamer_->VideoStreamConfig();

  while (!shutdown_requested.load()) {
    video_streamer_->VideoStream();
  }

  video_streamer_.reset();

  std::cerr << "Main Func Exited" << std::endl;

  return 0;
}
