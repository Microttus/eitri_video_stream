#include <iostream>

#include "../include/video-stream/video_streamer.h"
#include "../include/video-stream/ice_state_machine.h"

int main(int argc, char* argv[]) {
  std::cout << "Hello, World!" << std::endl;

  VideoStreamer video_streamer_;

  //video_streamer_.test2();
  video_streamer_.initTest();

  return 0;
}
