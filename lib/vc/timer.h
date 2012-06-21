#ifndef LIB_VC_TIMER_H_
#define LIB_VC_TIMER_H_

#ifndef GLFWTHREAD
#include <thread>
#include <atomic>
#else
#include "../gl.h"
#endif
#include "clock.h"
#include "winaverage.h"
#include "ticker.h"

namespace vc {
namespace timing {

// Timer class runs a Ticker class as payload in a constant or dynamic frequent.
// Sample usage:
//   Timer timer(new Ticker());
//   timer.Start(50);
// If the parameter in Start() was 0 or ignored, the timer would dynamically 
// changes the tps, to adapt the Ticker's needs.
class Timer {
 public:
  Timer(Ticker *);
  Timer(const Timer&) = delete;
  Timer& operator = (const Timer&) = delete;
  virtual ~Timer();

  void Start(double);
  void Stop();

  double tps() { return kSecOnClock / tick_time_avg_.Get(); }
  double tick_time_avg() { return tick_time_avg_.Get(); }
  double time_skew_avg() { return time_skew_avg_.Get(); }
  double tick_skew_avg() { return tick_skew_avg_.Get(); }

 private:
#ifndef GLFWTHREAD
  static void StaticTimer(Timer&);
  static void DynamicTimer(Timer&);
#else
  static void StaticTimer(void*);
  static void DynamicTimer(void*);
#endif

  double tps_;
  TimePoint epoch_;
  utils::WindowedAverage<TickId> tick_skew_avg_;
  utils::WindowedAverage<ClockRep> time_skew_avg_, tick_time_avg_;
#ifndef GLFWTHREAD
  std::atomic_bool running_;
#else
  bool running_;
#endif
  Ticker *ticker_;
#ifndef GLFWTHREAD
  std::thread thread_;
#else
  GLFWthread thread_;
#endif

};

}  // namespace timing
}  // namespace vc

#endif /* LIB_VC_TIMING_TIMER_H_ */

