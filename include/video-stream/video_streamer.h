#include <string>
#include <memory>
#include <opencv2/opencv.hpp>
#include <gst/gst.h>

class VideoStreamer {
 public:
  VideoStreamer();
  ~VideoStreamer();

  int Config();
  int Stream();
  int Unconfig();

  std::string GetEnvironmentVariable(const std::string& env_var, std::string default_value);

 private:
  std::string rcs_ip;
  int com_port;
  int cam_ind;

  int width;
  int height;
  int fps;

  std::unique_ptr<cv::VideoCapture> cap_ptr;
  cv::Mat bgrFrame;

  GstElement *pipeline, *appsrc, *videoconvert, *x264enc, *rtph264pay, *udpsink;
  GstBus *bus;
  GstMessage *msg;
};
