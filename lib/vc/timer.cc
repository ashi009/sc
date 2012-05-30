#include <iostream>

#include <cmath>
#include "timer.h"

using namespace std;

using std::max;
using std::ref;
using std::chrono::duration_cast;
#ifndef GLFWTHREAD
using std::thread;
using std::this_thread::sleep_until;
#endif

namespace vc {
namespace timing {

Timer::Timer(Ticker *ticker) : running_(false), ticker_(ticker) {

}

Timer::~Timer() {
  Stop();
#ifndef GLFWTHREAD
  if (thread_.joinable())
    thread_.join();
#else
  glfwWaitThread(thread_, GLFW_WAIT);
#endif
}

void Timer::Start(double defined_tps = 0) {
#ifndef GLFWTHREAD
  if (!running_.exchange(true)) {
    tps_ = defined_tps;
    thread_ = thread(defined_tps > 0 ? StaticTimer : DynamicTimer, ref(*this));
  }
#else
  // as the timer should not be accessed by more than one thread, ignore ABA
  // problem here, for there is no atomic type in GLFWTHREAD.
  if (!running_) {
    running_ = true;
    thread_ = glfwCreateThread(defined_tps > 0 ? StaticTimer : DynamicTimer,
        (void*)this);
  }
#endif
}

void Timer::Stop() {
#ifndef GLFWTHREAD
  running_.exchange(false);
#else
  running_ = false;
#endif
}

#ifndef GLFWTHREAD
void Timer::StaticTimer(Timer &self) {
#else
void Timer::StaticTimer(void *self_ptr) {
  
  Timer *self = self_ptr;
#endif

  self.ticker_->BeforeStart();
  
  TickId expect_tick = 0, actual_tick;
  TimePoint expect_time = Clock::now(), wake_time;
  self.epoch_ = expect_time;

  ClockDuration duration_to_now;

  while (self.running_) {

    wake_time = Clock::now();

    duration_to_now = duration_cast<ClockDuration>(wake_time - self.epoch_);
    actual_tick = static_cast<TickId>(
        duration_to_now.count() * self.tps_ / kSecOnClock);

    if (!self.ticker_->Tick(actual_tick, self.epoch_)) {
      self.running_ = false;
      break;
    }

    // Calculate average tick time, this will be used to evaluate system load.
    self.tick_time_avg_.Append(
        duration_cast<ClockDuration>(Clock::now() - wake_time).count());

    // Calculate average tick skew number, this will be used to evaluate system
    // load. This number is slightly different from tick_time_avg, for it is a
    // time irrelvent number, say 2 frames drops or 0.5 frams drops, instead of
    // time skew of 10ms or 20ms.
    self.tick_skew_avg_.Append(
        actual_tick - expect_tick);

    // Calculate average time skew number between wake_time and expect_time,
    // this number can be used to evaluate system load.
    self.time_skew_avg_.Append(
        duration_cast<ClockDuration>(wake_time - expect_time).count());

    // Calcuate expect tick based on actual tick number, this helps when tick
    // skew happens.
    expect_tick = actual_tick + 1;
    // Calculate expect time based on local epoch and expect tick number. and 
    // ask thread to sleep until expect time, this will prevent tick skew from 
    // accumulating.
    expect_time = self.epoch_ + ClockDuration(
        static_cast<ClockRep>(ceil(expect_tick * kSecOnClock / self.tps_)));

#ifndef GLFWTHREAD
    sleep_until(expect_time);
#else
    glfwSleep(static_cast<chrono::milliseconds>(expect_time - Clock::now()) / 
        1000.0);
#endif

  }
  
  self.ticker_->BeforeEnd();

}

#ifndef GLFWTHREAD
void Timer::DynamicTimer(Timer &self) {
#else
void Timer::DynamicTimer(void *self_ptr) {
  
  Timer *self = self_ptr;
#endif
  
  self.ticker_->BeforeStart();
  
  TickId expect_frame = 0;
  TimePoint expect_time = Clock::now(), wake_time;
  self.epoch_ = expect_time;
  
  while (self.running_) {

    wake_time = Clock::now();
    
    // If the ticker returned false, stop looping immediately
    if (!self.ticker_->Tick(expect_frame, self.epoch_)) {
      self.running_ = false;
      break;
    }
    
    self.tick_time_avg_.Append(
        duration_cast<ClockDuration>(Clock::now() - wake_time).count());

    double tick_time = max(self.tick_time_avg_.Get(), 5.0);

    // Advance expect_time by 1, for we can never know the accurate frame rage.
    expect_frame++;
    // Calculate expect_time based on wake_time and recent average tick time.
    expect_time = wake_time + ClockDuration(
        static_cast<ClockRep>(ceil(tick_time)));

#ifndef GLFWTHREAD
    sleep_until(expect_time);
#else
    glfwSleep(static_cast<chrono::milliseconds>(expect_time - Clock::now()) / 
        1000.0);
#endif

  }
  
  self.ticker_->BeforeEnd();
  
}

}  // namespace timing
}  // namespace vc

