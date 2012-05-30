#ifndef LIB_GL_DATA_INL_H_
#define LIB_GL_DATA_INL_H_

#include <functional>
#include <cstring>
#include <cmath>

namespace gl {

template<int Row, int Col, typename T>
Matrix<Row, Col, T>::Matrix(const std::vector<T> &m) {
  for (int i = 0; i < Row * Col; ++i)
    data[i] = m[i];
}

template<int Row, int Col, typename T>
Matrix<Row, Col, T> &Matrix<Row, Col, T>::operator =(const std::vector<T> &m) {
  for (int i = 0; i < Row * Col; ++i)
    data[i] = m[i];
  return *this;
}

template<int Row, int Col, typename T>
Matrix<Row, Col, T> Matrix<Row, Col, T>::operator +(
    const Matrix<Row, Col, T> &m) {
  Matrix<Row, Col, T> res;
  for (int i = 0; i < Row * Col; ++i)
    res.data[i] = data[i] + m.data[i];
  return res;
}

template<int Row, int Col, typename T>
Matrix<Row, Col, T> Matrix<Row, Col, T>::operator -(
    const Matrix<Row, Col, T> &m) {
  Matrix<Row, Col, T> res;
  for (int i = 0; i < Row * Col; ++i)
    res.data[i] = data[i] - m.data[i];
  return res;
}

template<int Row, int Col, typename T>
Matrix<Row, Col, T> &Matrix<Row, Col, T>::operator +=(
    const Matrix<Row, Col, T> &m) {
  for (int i = 0; i < Row * Col; ++i)
    data[i] += m.data[i];
  return *this;
}

template<int Row, int Col, typename T>
Matrix<Row, Col, T> &Matrix<Row, Col, T>::operator -=(
    const Matrix<Row, Col, T> &m) {
  for (int i = 0; i < Row * Col; ++i)
    data[i] -= m.data[i];
  return *this;
}

template <int Row, int Col, typename T>
template <int nCol>
Matrix<Row, nCol, T> Matrix<Row, Col, T>::operator *(
    const Matrix<Col, nCol, T> &m) {
  Matrix<Row, nCol, T> res;
  for (int i = 0; i < Row; ++i)
  for (int j = 0; j < nCol; ++j) {
    T &buf = res.data[nCol * j + i];
    buf = 0;
    for (int k = 0; k < Col; ++k) {
      buf += data[Col * k + i] * m.data[Col * j + k];
    }
  }
  return res;
}

template <int Row, int Col, typename T>
Matrix<Row, Col, T> &Matrix<Row, Col, T>::operator *=(
    const Matrix<Col, Col, T> &m) {
  Matrix<Row, Col, T> cur(*this);
  for (int i = 0; i < Row; ++i)
  for (int j = 0; j < Col; ++j) {
    T &buf = data[Col * j + i];
    buf = 0;
    for (int k = 0; k < Col; ++k) {
      buf += cur.data[Col * k + i] * m.data[Col * j + k];
    }
  }
  return *this;
}

template<int Row, int Col, typename T>
Matrix<Row, Col, T> Matrix<Row, Col, T>::operator *(T k) {
  Matrix<Row, Col, T> res;
  for (int i = 0; i < Row * Col; ++i)
    res.data[i] = data[i] * k;
  return res;
}

template<int Row, int Col, typename T>
Matrix<Row, Col, T> &Matrix<Row, Col, T>::operator *=(T k) {
  for (int i = 0; i < Row * Col; ++i)
    data[i] *= k;
  return *this;
}

template<int Row, int Col, typename T>
T &Matrix<Row, Col, T>::operator [](int n) {
  return data[n];
}

template<int Row, typename T>
template<typename ...Args>
Vector<Row, T>::Vector(Args &&...args) : 
  Matrix<Row, 1, T>(std::forward<Args>(args)...) {

}

}  // namespace gl

#endif /* LIB_GL_DATA_INL_H_ */

