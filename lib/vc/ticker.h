#ifndef LIB_VC_TICKER_H_
#define LIB_VC_TICKER_H_

#include "clock.h"

namespace vc {
namespace timing {

class Timer;

class Ticker {
 public:
  virtual ~Ticker();
 protected:
  Ticker();
  virtual void BeforeStart();
  virtual void BeforeEnd();
  virtual bool Tick(const TickId, const TimePoint&) = 0;
  friend class Timer;
};

}  // namespace timing
}  // namespace vc

#endif /* LIB_VC_TICKER_H_ */

