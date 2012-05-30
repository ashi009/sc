#ifndef LIB_VC_CLOCK_H_
#define LIB_VC_CLOCK_H_

#include <chrono>

namespace vc {
namespace timing {

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds ClockDuration;
typedef Clock::time_point TimePoint;
typedef ClockDuration::rep ClockRep;
typedef int64_t TickId;

const ClockRep kSecOnClock = std::chrono::duration_cast<ClockDuration>(
    std::chrono::seconds(1)).count();

}  // namespace timing
}  // namespace vc

#endif /* LIB_VC_CLOCK_H_ */

