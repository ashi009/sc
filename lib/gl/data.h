#ifndef LIB_GL_DATA_H_
#define LIB_GL_DATA_H_

#include <vector>
#include <GL/gl.h>

#define PI 3.1415926535897932384626433832795028841971693993751058

namespace gl {

class Data {

};

template <int Row, typename T>
class Vector;

template<int Row, int Col = Row, typename T = GLfloat>
class Matrix : public Data {
  
  static_assert(Row >= 1 && Row <= 4, "Row should between 1 and 4");
  static_assert(Col >= 1 && Col <= 4, "Col should between 1 and 4");
  
 public:
  Matrix() = default;
  Matrix(const std::vector<T>&);

  Matrix<Row, Col, T> &operator =(const std::vector<T>&);
 
  Matrix<Row, Col, T> operator +(const Matrix<Row, Col, T>&);
  Matrix<Row, Col, T> operator -(const Matrix<Row, Col, T>&);
  Matrix<Row, Col, T> &operator +=(const Matrix<Row, Col, T>&);
  Matrix<Row, Col, T> &operator -=(const Matrix<Row, Col, T>&);
  template <int nCol>
  Matrix<Row, nCol, T> operator *(const Matrix<Col, nCol, T>&);
  Matrix<Row, Col, T> &operator *=(const Matrix<Col, Col, T>&);
  Matrix<Row, Col, T> operator *(T);
  Matrix<Row, Col, T> &operator *=(T);
  
  T &operator [](int);
  
  Matrix<Row, Col, T> &Scale(GLfloat, GLfloat, GLfloat);
  Matrix<Row, Col, T> &Translate(GLfloat, GLfloat, GLfloat);
  
  Matrix<Row, Col, T> &RotX(GLfloat);
  Matrix<Row, Col, T> &RotY(GLfloat);
  Matrix<Row, Col, T> &RotZ(GLfloat);
  
  Vector<Col-1, T> Transform(const Vector<Col-1, T>&);
  Vector<Col-1, T> Inverse(float x, float y);
  
  T data[Row * Col];
  typedef T data_t;
};

// vector form of Matrix:
//    template<int N>
//    using Vector = Matrix<N, 1>;

template<int Row, typename T = GLfloat>
class Vector : public Matrix<Row, 1, T> {

 public:
  template<typename ...Args>
  Vector(Args &&...args);

};

namespace matrix {

Matrix<4> I();
Matrix<4> Perspective(GLfloat, GLfloat, GLfloat, GLfloat);
Matrix<4> Orthographic(GLfloat, GLfloat, GLfloat, GLfloat);
Matrix<4> Scale(GLfloat, GLfloat, GLfloat);
Matrix<4> Translate(GLfloat, GLfloat, GLfloat);
Matrix<4> RotX(GLfloat);
Matrix<4> RotY(GLfloat);
Matrix<4> RotZ(GLfloat);

}  // namespace matrix
}  // namespace gl

#include "data-inl.h"

#endif /* LIB_GL_DATA_H_ */

