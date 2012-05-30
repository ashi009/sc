#ifndef LIB_VC_WINAVERAGE_INL_H_
#define LIB_VC_WINAVERAGE_INL_H_

namespace vc {
namespace utils {

template<typename T>
WindowedAverage<T>::WindowedAverage(int N = 50) : 
  windows_(N, 0), sum_(0), cur_window_(0) {
}

template<typename T>
void WindowedAverage<T>::Append(T value) {
  sum_ += value - windows_[cur_window_];
  windows_[cur_window_] = value;
  if (++cur_window_ >= static_cast<int>(windows_.size()))
    cur_window_ = 0;
}
  
template<typename T>
double WindowedAverage<T>::Get() {
  return sum_ / windows_.size();
}

}  // namespace utils
}  // namespace vc

#endif /* LIB_VC_WINAVERAGE_INL_H_ */

