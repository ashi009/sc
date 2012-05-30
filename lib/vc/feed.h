#ifndef LIB_VC_FEED_H_
#define LIB_VC_FEED_H_

#include <queue>
#include <map>
#ifndef GLFWTHREAD
#include <mutex>
#else
#include "../gl.h"
#endif
#include "clock.h"
#include "source.h"

namespace vc {

class Feed {

 public:
  Feed(Source *);
  virtual ~Feed();
  
  typedef std::pair<timing::TimePoint, cv::Mat> Frame;
  
  void Enque(Frame*);
  bool Query(const timing::TimePoint&);
  
  Source *source() { return source_; }
  // this method is not snyced, make sure only one thread access it.
  void *current_image() { 
    return current_frame_ ? current_frame_->second.data : nullptr;
  }
  cv::Mat Snapshot();

 private:
  Source *source_;
#ifndef GLFWTHREAD
  std::mutex lock_;
#else
  GLFWmutex lock_;
#endif
  Frame* current_frame_;
  std::queue<Frame*> frame_queue_;

};

}  // namespace vc

#endif /* LIB_VC_FEED_H_ */
