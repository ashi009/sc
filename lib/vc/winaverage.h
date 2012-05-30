#ifndef LIB_VC_WINAVERAGE_H_
#define LIB_VC_WINAVERAGE_H_

#include <type_traits>
#include <vector>

namespace vc {
namespace utils {

// Windowed Average Class, calculates the average of last N values in O(1).
// Usage Example:
//    WindowedAverage<double, 10> wavg;
//    for (int i = 0; i < 100; i++) wavg.Append(i);
//    wavg.Get();   
// Which outputs 94.5.
// Note: this Class is not threadsafe, so you may not allow to Append(value)
//    from more than one thread. XXX  But by using atomic fields could solve 
//    this problem.
template<typename T>
class WindowedAverage {
  static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, 
      "T should be a numeric type.");
 public:
  WindowedAverage(int);
  void Append(T);
  double Get();
 private:
  std::vector<T> windows_;
  double sum_;
  int cur_window_;
};

}  // namespace utils
}  // namespace vc

#include "winaverage-inl.h"

#endif /* LIB_VC_WINAVERAGE_H_ */

