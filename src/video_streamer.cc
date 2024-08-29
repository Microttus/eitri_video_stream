//
// Created by Martin Økter on 29/05/2024.
//

#include "../include/video-stream/video_streamer.h"

#include <iostream>
#include <opencv2/opencv.hpp>
#include <fstream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

VideoStreamer::VideoStreamer(){
  std::cout << "Configuring video stream" << std::endl;

  rcs_ip = GetEnvironmentVariable("RCS_IP", "127.0.0.1");
  cam_ind = std::stoi(GetEnvironmentVariable("CAMIND", "0"));
  com_port = std::stoi(GetEnvironmentVariable("COMPORT", "23513"));

  //cap = cam_ind;
  cap_ptr = std::make_unique<cv::VideoCapture>(cam_ind);
}

VideoStreamer::~VideoStreamer() {
  Unconfig();
}

int VideoStreamer::Config() {
  const char* ip = rcs_ip.c_str();
  int port = com_port;
  char outputURL[256];
  snprintf(outputURL, sizeof(outputURL), "rtp://%s:%d", ip, port);

  // Initialize libavformat and network
  avformat_network_init();

  // Set up the video capture
  if (!cap_ptr->isOpened()) {
    std::cerr << "Error: Could not open camera" << std::endl;
    return -1;
  }

  // Get the frame properties
  width = static_cast<int>(cap_ptr->get(cv::CAP_PROP_FRAME_WIDTH));
  height = static_cast<int>(cap_ptr->get(cv::CAP_PROP_FRAME_HEIGHT));
  fps = static_cast<int>(cap_ptr->get(cv::CAP_PROP_FPS));
  if (fps == 0) fps = 30; // Default to 30 fps if camera doesn't provide FPS

  // Allocate the AVFormatContext
  formatContext = avformat_alloc_context();
  if (!formatContext) {
    std::cerr << "Error: Could not allocate format context" << std::endl;
    return -1;
  }

  // Set up the output format
  const AVOutputFormat* outputFormat = av_guess_format("rtp", nullptr, nullptr);
  if (!outputFormat) {
    std::cerr << "Error: Could not guess output format" << std::endl;
    return -1;
  }
  formatContext->oformat = const_cast<AVOutputFormat*>(outputFormat);

  // Open the output URL
  if (avio_open(&formatContext->pb, outputURL, AVIO_FLAG_WRITE) < 0) {
    std::cerr << "Error: Could not open output URL" << std::endl;
    return -1;
  }

  // Create a new video stream
  stream = avformat_new_stream(formatContext, nullptr);
  if (!stream) {
    std::cerr << "Error: Could not create new stream" << std::endl;
    return -1;
  }

  // Set up the codec context
  auto* codec = const_cast<AVCodec *>(avcodec_find_encoder(AV_CODEC_ID_H264));
  if (!codec) {
    std::cerr << "Error: Could not find H.264 encoder" << std::endl;
    return -1;
  }

  codecContext = avcodec_alloc_context3(codec);
  if (!codecContext) {
    std::cerr << "Error: Could not allocate codec context" << std::endl;
    return -1;
  }

  codecContext->codec_id = AV_CODEC_ID_H264;
  codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
  codecContext->width = width;
  codecContext->height = height;
  codecContext->time_base = {1, fps};
  codecContext->framerate = {fps, 1};
  codecContext->gop_size = 12; // Group of pictures size
  codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

  // Set codec parameters for streaming
  AVDictionary* codecOptions = nullptr;
  av_dict_set(&codecOptions, "preset", "ultrafast", 0);
  av_dict_set(&codecOptions, "tune", "zerolatency", 0);

  // Open the codec
  if (avcodec_open2(codecContext, codec, &codecOptions) < 0) {
    std::cerr << "Error: Could not open codec" << std::endl;
    return -1;
  }

  // Set up the stream codec parameters
  if (avcodec_parameters_from_context(stream->codecpar, codecContext) < 0) {
    std::cerr << "Error: Could not initialize stream codec parameters" << std::endl;
    return -1;
  }

  stream->time_base = codecContext->time_base;

  // Write the stream header
  if (avformat_write_header(formatContext, nullptr) < 0) {
    std::cerr << "Error: Could not write stream header" << std::endl;
    return -1;
  }

  // Set up the scaling context
  swsContext = sws_getContext(width, height, AV_PIX_FMT_BGR24, width, height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);
  if (!swsContext) {
    std::cerr << "Error: Could not create scaling context" << std::endl;
    return -1;
  }

  // Allocate frames
  frame = av_frame_alloc();
  yuvFrame = av_frame_alloc();
  if (!frame || !yuvFrame) {
    std::cerr << "Error: Could not allocate frames" << std::endl;
    return -1;
  }

  // Set up the YUV frame
  yuvFrame->format = codecContext->pix_fmt;
  yuvFrame->width = codecContext->width;
  yuvFrame->height = codecContext->height;
  if (av_frame_get_buffer(yuvFrame, 32) < 0) {
    std::cerr << "Error: Could not allocate YUV frame buffer" << std::endl;
    return -1;
  }

  return 1;
}

int VideoStreamer::Stream() {
  // Capture a frame from the camera
  std::cout << count << std::endl;

  *cap_ptr >> bgrFrame;
  if (bgrFrame.empty()) {return 0;}

  // Display the frame using OpenCV
  cv::imshow("Captured Frame", bgrFrame);
  if (cv::waitKey(1) >= 0) return 0; // Exit on any key press

  // Convert the frame to AVFrame
  av_image_fill_arrays(frame->data, frame->linesize, bgrFrame.data, AV_PIX_FMT_BGR24, width, height, 1);

  // Convert the frame from BGR to YUV
  sws_scale(swsContext, frame->data, frame->linesize, 0, height, yuvFrame->data, yuvFrame->linesize);

  yuvFrame->pts = frameIndex++;

  // Encode the frame
  AVPacket pkt = {nullptr};
  av_packet_unref(&pkt);
  pkt.data = nullptr;
  pkt.size = 0;

  if (avcodec_send_frame(codecContext, yuvFrame) == 0) {
    while (avcodec_receive_packet(codecContext, &pkt) == 0) {
      pkt.stream_index = stream->index;
      av_interleaved_write_frame(formatContext, &pkt);
      av_packet_unref(&pkt);
    }
  }
  return 1;
}

int VideoStreamer::Unconfig() {
  // Release Camera
  cap_ptr->release();
  cap_ptr.reset();

  // Write the trailer
  av_write_trailer(formatContext);

  // Clean up
  av_frame_free(&frame);
  av_frame_free(&yuvFrame);
  sws_freeContext(swsContext);
  avcodec_free_context(&codecContext);
  avio_close(formatContext->pb);
  avformat_free_context(formatContext);

  return 1;
}

void VideoStreamer::WriteSDP(const char* ip, int port) {
  std::ofstream sdpFile("stream.sdp");
  if (!sdpFile.is_open()) {
    std::cerr << "Error: Could not open SDP file for writing" << std::endl;
    return;
  }

  sdpFile << "v=0\n";
  sdpFile << "o=- 0 0 IN IP4 " << ip << "\n";
  sdpFile << "s=Video Stream\n";
  sdpFile << "c=IN IP4 " << ip << "\n";
  sdpFile << "t=0 0\n";
  sdpFile << "m=video " << port << " RTP/AVP 96\n";
  sdpFile << "a=rtpmap:96 H264/90000\n";
  sdpFile << "a=fmtp:96 packetization-mode=1; sprop-parameter-sets=";

  for (int i = 0; i < formatContext->nb_streams; ++i) {
    AVCodecParameters *codecpar = formatContext->streams[i]->codecpar;
    if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO && codecpar->extradata_size > 0) {
      sdpFile.write((const char*)codecpar->extradata, codecpar->extradata_size);
      break;
    }
  }

  sdpFile << "\n";
  sdpFile.close();
}

std::string VideoStreamer::GetEnvironmentVariable(const std::string& env_var, std::string default_value) {
  const char* value = std::getenv(env_var.c_str());
  if (value == nullptr) {
    std::cout << "Could not find" << env_var << "in environment, using default " << default_value << std::endl;
    return default_value;
  }
  std::string str(value);
  std::cout << env_var << " successfully set to " << str << std::endl;
  return str;
}

int VideoStreamer::initTest() {

  Config();
  WriteSDP(rcs_ip.c_str(), com_port);
  count = 0;

  while (count++ < 100) {
    Stream();
  }

  Unconfig();

  return 0;
}

int VideoStreamer::VideoStream() {
  Config();
  WriteSDP(rcs_ip.c_str(), com_port);

  Stream();

  return 0;
}