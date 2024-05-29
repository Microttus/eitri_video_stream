//
// Created by Martin Økter on 29/05/2024.
//

#include "../include/video-stream/video_streamer.h"

#include <iostream>
#include <opencv2/opencv.hpp>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

VideoStreamer::VideoStreamer() {
  std::cout << "Hei" << std::endl;
}

int VideoStreamer::initTest(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
    return -1;
  }

  const char* ip = argv[1];
  int port = std::stoi(argv[2]);
  char outputURL[256];
  snprintf(outputURL, sizeof(outputURL), "rtp://%s:%d", ip, port);

  // Initialize libavformat and network
  avformat_network_init();

  // Set up the video capture
  cv::VideoCapture cap(0);
  if (!cap.isOpened()) {
    std::cerr << "Error: Could not open camera" << std::endl;
    return -1;
  }

  // Get the frame properties
  int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
  int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
  int fps = static_cast<int>(cap.get(cv::CAP_PROP_FPS));
  if (fps == 0) fps = 30; // Default to 30 fps if camera doesn't provide FPS

  // Allocate the AVFormatContext
  AVFormatContext* formatContext = avformat_alloc_context();
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
  formatContext->oformat = outputFormat;

  // Open the output URL
  if (avio_open(&formatContext->pb, outputURL, AVIO_FLAG_WRITE) < 0) {
    std::cerr << "Error: Could not open output URL" << std::endl;
    return -1;
  }

  // Create a new video stream
  AVStream* stream = avformat_new_stream(formatContext, nullptr);
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

  AVCodecContext* codecContext = avcodec_alloc_context3(codec);
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
  SwsContext* swsContext = sws_getContext(width, height, AV_PIX_FMT_BGR24, width, height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);
  if (!swsContext) {
    std::cerr << "Error: Could not create scaling context" << std::endl;
    return -1;
  }

  // Allocate frames
  AVFrame* frame = av_frame_alloc();
  AVFrame* yuvFrame = av_frame_alloc();
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

  int frameIndex = 0;
  cv::Mat bgrFrame;

  while (true) {
    // Capture a frame from the camera
    cap >> bgrFrame;
    if (bgrFrame.empty()) break;

    // Convert the frame to AVFrame
    av_image_fill_arrays(frame->data, frame->linesize, bgrFrame.data, AV_PIX_FMT_BGR24, width, height, 1);

    // Convert the frame from BGR to YUV
    sws_scale(swsContext, frame->data, frame->linesize, 0, height, yuvFrame->data, yuvFrame->linesize);

    yuvFrame->pts = frameIndex++;

    // Encode the frame
    AVPacket pkt = {0};
    av_init_packet(&pkt);

    if (avcodec_send_frame(codecContext, yuvFrame) == 0) {
      while (avcodec_receive_packet(codecContext, &pkt) == 0) {
        pkt.stream_index = stream->index;
        av_interleaved_write_frame(formatContext, &pkt);
        av_packet_unref(&pkt);
      }
    }
  }

  // Write the trailer
  av_write_trailer(formatContext);

  // Clean up
  av_frame_free(&frame);
  av_frame_free(&yuvFrame);
  sws_freeContext(swsContext);
  avcodec_close(codecContext);
  avcodec_free_context(&codecContext);
  avio_close(formatContext->pb);
  avformat_free_context(formatContext);

  return 0;
}
