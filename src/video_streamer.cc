#include <iostream>
#include <opencv2/opencv.hpp>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

#include "../include/video-stream/video_streamer.h"

VideoStreamer::VideoStreamer() {
  std::cout << "Configuring video stream using GStreamer" << std::endl;

  rcs_ip = "127.0.0.1"; // Replace with the actual IP or environment variable
  cam_ind = 0; // Replace with the actual camera index or environment variable
  com_port = 5000; // Replace with the actual port or environment variable

  cap_ptr = std::make_unique<cv::VideoCapture>(cam_ind);
}

VideoStreamer::~VideoStreamer() {
  Unconfig();
}

int VideoStreamer::Config() {
  if (!cap_ptr->isOpened()) {
    std::cerr << "Error: Could not open camera" << std::endl;
    return -1;
  }

  width = static_cast<int>(cap_ptr->get(cv::CAP_PROP_FRAME_WIDTH));
  height = static_cast<int>(cap_ptr->get(cv::CAP_PROP_FRAME_HEIGHT));
  fps = static_cast<int>(cap_ptr->get(cv::CAP_PROP_FPS));
  if (fps == 0) fps = 30;

  gst_init(nullptr, nullptr);

  // Create GStreamer elements
  pipeline = gst_pipeline_new("video-streamer");
  appsrc = gst_element_factory_make("appsrc", "source");
  videoconvert = gst_element_factory_make("videoconvert", "convert");
  x264enc = gst_element_factory_make("x264enc", "encoder");
  rtph264pay = gst_element_factory_make("rtph264pay", "payloader");
  udpsink = gst_element_factory_make("udpsink", "sink");

  if (!pipeline || !appsrc || !videoconvert || !x264enc || !rtph264pay || !udpsink) {
    std::cerr << "Error: One or more GStreamer elements could not be created." << std::endl;
    return -1;
  }

  // Set up the appsrc element
  g_object_set(G_OBJECT(appsrc), "caps",
               gst_caps_new_simple("video/x-raw",
                                   "format", G_TYPE_STRING, "BGR",
                                   "width", G_TYPE_INT, width,
                                   "height", G_TYPE_INT, height,
                                   "framerate", GST_TYPE_FRACTION, fps, 1,
                                   NULL), NULL);
  g_object_set(G_OBJECT(appsrc), "block", TRUE, NULL);

  // Set up the udpsink element
  g_object_set(G_OBJECT(udpsink), "host", rcs_ip.c_str(), "port", com_port, NULL);

  // Build the pipeline
  gst_bin_add_many(GST_BIN(pipeline), appsrc, videoconvert, x264enc, rtph264pay, udpsink, NULL);
  if (!gst_element_link_many(appsrc, videoconvert, x264enc, rtph264pay, udpsink, NULL)) {
    std::cerr << "Error: GStreamer elements could not be linked." << std::endl;
    gst_object_unref(pipeline);
    return -1;
  }

  gst_element_set_state(pipeline, GST_STATE_PLAYING);
  return 0;
}

int VideoStreamer::Stream() {
  *cap_ptr >> bgrFrame;
  if (bgrFrame.empty()) return 0;

  GstBuffer *buffer = gst_buffer_new_allocate(NULL, bgrFrame.total() * bgrFrame.elemSize(), NULL);
  gst_buffer_fill(buffer, 0, bgrFrame.data, bgrFrame.total() * bgrFrame.elemSize());

  GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(gst_clock_get_time(gst_element_get_clock(pipeline)), 1, 1000000);
  GstAppSrc *appsrc_cast = GST_APP_SRC(appsrc);
  gst_app_src_push_buffer(appsrc_cast, buffer);


  return 1;
}

int VideoStreamer::Unconfig() {
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(pipeline);
  return 1;
}

std::string VideoStreamer::GetEnvironmentVariable(const std::string& env_var, std::string default_value) {
  const char* value = std::getenv(env_var.c_str());
  if (value == nullptr) {
    std::cout << "Could not find " << env_var << " in environment, using default " << default_value << std::endl;
    return default_value;
  }
  std::string str(value);
  std::cout << env_var << " successfully set to " << str << std::endl;
  return str;
}
