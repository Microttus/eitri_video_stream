//
// Created by Martin Økter on 26/06/2024.
//

#ifndef VIDEO_STREAM_SRC_VIDEO_CAPTURE_H_
#define VIDEO_STREAM_SRC_VIDEO_CAPTURE_H_

#include <opencv2/opencv.hpp>

class VideoCapture {
 public:
  VideoCapture();
  ~VideoCapture();

  bool IsConfigured();


};

#endif //VIDEO_STREAM_SRC_VIDEO_CAPTURE_H_
