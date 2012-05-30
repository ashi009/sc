#ifndef LIB_VC_SOURCE_H_
#define LIB_VC_SOURCE_H_

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "clock.h"
#include "ticker.h"

namespace vc {

class Feed;

class Source : public timing::Ticker {
 public:
  Source(const std::string&);
  Source(int, int);
  virtual ~Source();

  void delay(int new_delay) { delay_ = new_delay; }

  double fps() { return cap_.get(CV_CAP_PROP_FPS); }
  int width() { return width_; }
  int height() { return height_; }
  int delay() { return delay_; }
  bool ok() { return ok_; }
  const std::string &source_uri() { return source_uri_; }
  std::vector<Feed*> &feeds() { return feeds_; }

 protected:
  void Init();

  cv::VideoCapture cap_;
  int width_, height_;
  int delay_;
  int channels_;
  bool ok_;
  std::string source_uri_;
  std::vector<Feed*> feeds_;
};

class CameraSource : public Source {
 public:
  CameraSource(int, int);
  bool Tick(const timing::TickId, const timing::TimePoint&);

 private:
};

class FileSource : public Source {
 public:
  FileSource(const std::string&);
  bool Tick(const timing::TickId, const timing::TimePoint&);

};

}  // namespace vc

#endif /* LIB_VC_SOURCE_H_ */

