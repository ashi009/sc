#include <sstream>
#include <chrono>
#include "source.h"
#include "feed.h"

using namespace std;

using std::ostringstream;
using std::string;
using std::move;
using std::forward;
using std::chrono::milliseconds;
using cv::VideoCapture;
using cv::Mat;
using vc::timing::TimePoint;
using vc::timing::TickId;
using vc::timing::Clock;
using vc::Feed;

namespace vc {

Source::Source(int device_id, int number_of_channels) :
    cap_(device_id), width_(0), height_(0), delay_(0), 
    channels_(number_of_channels), ok_(false) {
  Init();
  ostringstream oss;
  oss << "camera:" << device_id << ":" << number_of_channels;
  source_uri_ = oss.str();
}

Source::Source(const string &uri) :
    cap_(uri), width_(0), height_(0), delay_(0),
    channels_(1), ok_(false), source_uri_(uri) {
  Init();
}

Source::~Source() {
  for (int i = 0; i < feeds_.size(); i++)
    delete feeds_[i];
}

void Source::Init() {
  if (cap_.isOpened()) {
    width_ = static_cast<int>(cap_.get(CV_CAP_PROP_FRAME_WIDTH));
    height_ = static_cast<int>(cap_.get(CV_CAP_PROP_FRAME_HEIGHT));
    for (int i = 0; i < channels_; i++)
      feeds_.push_back(new Feed(this));
    ok_ = true;
  }
}

CameraSource::CameraSource(int device_id, int number_of_channels) :
    Source(device_id, number_of_channels) {

}

bool CameraSource::Tick(const TickId actual_frame, const TimePoint &epoch) {

  // If there was no more frames.
  if (!cap_.grab()) return false;

  TimePoint now = Clock::now();
  // Retrieve frames from every channel.
  for (int i = 0; i < feeds_.size(); i++) {
    Mat img;
    // decode frame
    cap_.retrieve(img, i);
    feeds_[i]->Enque(new Feed::Frame(now, img.clone()));
  }

  return true;

}

FileSource::FileSource(const string &uri) : Source(uri) {

}

bool FileSource::Tick(const TickId actual_frame, const TimePoint &epoch) {

  typedef milliseconds::rep TimeRep;
  
  if (cap_.get(CV_CAP_PROP_FRAME_COUNT) > 0 && 
      cap_.get(CV_CAP_PROP_POS_FRAMES) > -1) {
    
    TickId frames = 0;
    
    // skip left behind frames.
    while ((frames = static_cast<TickId>(cap_.get(CV_CAP_PROP_POS_FRAMES))) < 
        actual_frame) {
      // if no more available stop ticking
      if (!cap_.grab()) return false;
    }
    
    // go too far from we need, skip this tick
    if (frames > actual_frame)
      return true;
    
  } else {
  
    // fallback method to grab next frame.
    if (!cap_.grab()) return false;
  
  }
  
  Mat img;
  // decode frame.
  cap_.retrieve(img);
  
  TimePoint time = epoch;
  TimeRep codec_time = static_cast<TimeRep>(cap_.get(CV_CAP_PROP_POS_MSEC));
  
  // If cannot have the accurate time from stream, fallback to our calcuated
  // frame time.
  if (codec_time == 0) {
    codec_time = static_cast<TimeRep>(actual_frame * 1000 / 
        cap_.get(CV_CAP_PROP_FPS));
  }
  
  time += milliseconds(codec_time);
  
  feeds_[0]->Enque(new Feed::Frame(time, img.clone()));
  
  return true;
  
}


}  // namespace vc

