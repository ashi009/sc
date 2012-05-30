#include <cmath>
#include "data.h"

using std::vector;

namespace gl {

template<>
Matrix<4> &Matrix<4>::Scale(GLfloat x, GLfloat y, GLfloat z) {
  if (x != 1) for (int i = 0; i < 16; i += 4) data[i] *= x;
  if (y != 1) for (int i = 1; i < 16; i += 4) data[i] *= y;
  if (z != 1) for (int i = 2; i < 16; i += 4) data[i] *= z;
  return *this;
}

template<>
Matrix<4> &Matrix<4>::Translate(GLfloat x, GLfloat y, GLfloat z) {
  if (x != 0) for (int i = 0; i < 16; i += 4) data[i] += x * data[i+3];
  if (y != 0) for (int i = 1; i < 16; i += 4) data[i] += y * data[i+2];
  if (z != 0) for (int i = 2; i < 16; i += 4) data[i] += z * data[i+1];
  return *this;
}

template<>
Matrix<4> &Matrix<4>::RotX(GLfloat deg) {
  if (deg != 0) {
    deg *= PI / 180;
    GLfloat sbuf[16];
    memcpy(sbuf, data, sizeof(data));
    GLfloat c = cos(deg), s = sin(deg);
    for (int i = 1; i < 16; i += 4) data[i] = sbuf[i] * c - sbuf[i+1] * s;
    for (int i = 2; i < 16; i += 4) data[i] = sbuf[i-1] * s + sbuf[i] * c;
  }
  return *this;
}

template<>
Matrix<4> &Matrix<4>::RotY(GLfloat deg) {
  if (deg != 0) {
    deg *= PI / 180;
    GLfloat sbuf[16];
    memcpy(sbuf, data, sizeof(data));
    GLfloat c = cos(deg), s = sin(deg);
    for (int i = 0; i < 16; i += 4) data[i] = sbuf[i] * c + sbuf[i+2] * s;
    for (int i = 2; i < 16; i += 4) data[i] = sbuf[i] * c - sbuf[i-2] * s;
  }
  return *this;
}

template<>
Matrix<4> &Matrix<4>::RotZ(GLfloat deg) {
  if (deg != 0) {
    deg *= PI / 180;
    GLfloat sbuf[16];
    memcpy(sbuf, data, sizeof(data));
    GLfloat c = cos(deg), s = sin(deg);
    for (int i = 0; i < 16; i += 4) data[i] = sbuf[i] * c - sbuf[i+1] * s;
    for (int i = 1; i < 16; i += 4) data[i] = sbuf[i-1] * s + sbuf[i] * c;
  }
  return *this;
}

template<>
template<>
Vector<3> Matrix<4>::Transform(const Vector<3> &m) {
  Vector<3> res;
  GLfloat w = 1 / (m.data[0] * data[3] + m.data[1] * data[7] + 
      m.data[2] * data[11] + data[15]);
  for (int i = 0; i < 3; ++i)
    res.data[i] = (m.data[0] * data[i] + m.data[1] * data[4+i] + 
        m.data[2] * data[8+i] + data[12+i]) * w;
  return res;
}

namespace matrix {

Matrix<4> I() {
  return Matrix<4>(vector<GLfloat> {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  });
}

Matrix<4> Scale(GLfloat x, GLfloat y, GLfloat z) {
  return Matrix<4>(vector<GLfloat> {
    x, 0, 0, 0,
    0, y, 0, 0,
    0, 0, z, 0,
    0, 0, 0, 1
  });
}

Matrix<4> Translate(GLfloat x, GLfloat y, GLfloat z) {
  return Matrix<4>(vector<GLfloat> {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    x, y, z, 1
  });
}

Matrix<4> RotX(GLfloat deg) { 
  deg *= PI / 180;
  GLfloat c = cos(deg), s = sin(deg);
  return Matrix<4>(vector<GLfloat> {
    1, 0, 0, 0,
    0, c, s, 0,
    0, -s, c, 0,
    0, 0, 0, 1
  });
}

Matrix<4> RotY(GLfloat deg) {
  deg *= PI / 180;
  GLfloat c = cos(deg), s = sin(deg);
  return Matrix<4>(vector<GLfloat> {
    c, 0, -s, 0,
    0, 1, 0, 0,
    s, 0, c, 0,
    0, 0, 0, 1
  });
}

Matrix<4> RotZ(GLfloat deg) {
  deg *= PI / 180;
  GLfloat c = cos(deg), s = sin(deg);
  return Matrix<4>(vector<GLfloat> {
    c, s, 0, 0,
    -s, c, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  });
}

Matrix<4> Perspective(GLfloat fovy, GLfloat aspect, GLfloat near, GLfloat far) {
  fovy *= PI / 360;
  GLfloat a = 1 / (far - near);
  GLfloat cot = cos(fovy) / sin(fovy);
  return Matrix<4>(vector<GLfloat> {
    cot / aspect, 0, 0, 0,
    0, cot, 0, 0,
    0, 0, -(near + far) * a, -1,
    0, 0, -2 * near * far * a, 1
  });
}

Matrix<4> Orthographic(GLfloat width, GLfloat height, GLfloat near, GLfloat far) {
  GLfloat a = 1 / (far - near);
  return Matrix<4>(vector<GLfloat> {
    2 / width, 0, 0, 0,
    0, 2 / height, 0, 0,
    0, 0, -2 * a, 0,
    0, 0, -(far + near) * a, 1
  });
}

}  // namespace matrix
}  // namespace gl
