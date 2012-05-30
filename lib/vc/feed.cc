#include "feed.h"

using cv::Mat;
using vc::timing::TimePoint;
using std::chrono::milliseconds;
#ifndef GLFWTHREAD
using std::mutex;
using std::lock_guard;
#endif
using std::forward;

namespace vc {

Feed::Feed(Source* feeder_source) :
    source_(feeder_source), 
    current_frame_(nullptr) {
#ifdef GLFWTHREAD
  lock_ = glfwCreateMutex();
#endif
}

Feed::~Feed() {
  if (current_frame_)
    delete current_frame_;
  while (!frame_queue_.empty()) {
    delete frame_queue_.front();
    frame_queue_.pop();
  }
#ifdef GLFWTHREAD
  glfwDestroyMutex(lock_);
#endif
}

void Feed::Enque(Frame *frame) {
#ifndef GLFWTHREAD
  lock_guard<mutex> sync(lock_);
  frame_queue_.push(frame);
#else
  glfwLockMutex(lock_);
  frame_queue_.push(frame);
  glfwUnlockMutex(lock_);
#endif
}

bool Feed::Query(const TimePoint &time_point) {
#ifndef GLFWTHREAD
  // Use non blocking retriving method for low response time.
  if (lock_.try_lock()) {
#else
  // glfw left me no choices, but to use a block wait algorithm.
  glfwLockMutex(lock_);
#endif
    TimePoint delayed_time_point = time_point - milliseconds(source_->delay());
    // make a temporary frame pointer.
    Frame *frame = nullptr;
    // pull from queue to find the latest frame <= delayed_time_point;
    while (!frame_queue_.empty() && 
        frame_queue_.front()->first <= delayed_time_point) {
      // frame useless frame
      if (frame) delete frame;
      frame = frame_queue_.front();
      frame_queue_.pop();
    }
    // if we have found such a frame
    if (frame) {
      // update cached frame, and free existing one.
      if (current_frame_)
        delete current_frame_;
      current_frame_ = frame;
      // unlock
#ifndef GLFWTHREAD
      lock_.unlock();
#else
      glfwUnlockMutex(lock_);
#endif
      return true;
    }
#ifndef GLFWTHREAD
    lock_.unlock();
#else
    glfwUnlockMutex(lock_);
#endif
    return false;
#ifndef GLFWTHREAD
  }
  return false;
#endif
}

Mat Feed::Snapshot() {
#ifndef GLFWTHREAD
  lock_guard<mutex> sync(lock_);
#else
  glfwLockMutex(lock_);
#endif
  if (!current_frame_) {
#ifdef GLFWTHREAD
    glfwUnlockMutex(lock_);
#endif
    return Mat();
  }
#ifndef GLFWTHREAD
  return current_frame_->second.clone();
#else
  Mat res = current_frame_->second.clone();
  glfwUnlockMutex(lock_);
  return res;
#endif
}

}  // namespace vc
