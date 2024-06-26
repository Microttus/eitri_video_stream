//
// Created by Martin Økter on 29/05/2024.
//

#ifndef VIDEO_STREAM_SRC_VIDEO_STREAMER_H_
#define VIDEO_STREAM_SRC_VIDEO_STREAMER_H_

#include <string>

#include <opencv2/opencv.hpp>


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

class VideoStreamer {
 public:
  VideoStreamer();
  ~VideoStreamer() = default;

  int Config();
  int Stream();
  int Unconfig();

  void WriteSDP(const char* ip, int port)

  int test2();
  int initTest();

 private:
  std::string robot_ip = "127.0.0.1";
  int robot_port = 23513;

  int count = 0;
  cv::VideoCapture cap;
  int cameraIndex;

  int frameIndex = 0;
  cv::Mat bgrFrame;
  AVFormatContext* formatContext;
  AVCodecContext* codecContext;
  AVStream* stream;
  AVFrame* frame;
  AVFrame* yuvFrame;
  SwsContext* swsContext;

  int width;
  int height;
  int fps;


};

#endif //VIDEO_STREAM_SRC_VIDEO_STREAMER_H_
