#include <cassert>
#include <iostream>
#include "parameter.h"

using std::cerr;
using std::endl;

namespace gl {

Parameter::Parameter(GLint id) : id_((GLuint)id) {
  assert(id > -1);
}

void Parameter::operator =(GLfloat) {
  cerr << "should not in this function." << __FILE__ << __LINE__ << endl;
}

void Parameter::operator =(GLint) {
  cerr << "should not in this function." << __FILE__ << __LINE__ << endl;
}

void Parameter::operator =(const GLfloat*) {
  cerr << "should not in this function." << __FILE__ << __LINE__ << endl;
}

void Parameter::operator =(const GLint*) {
  cerr << "should not in this function." << __FILE__ << __LINE__ << endl;
}

void Parameter::operator =(const std::vector<GLfloat>&) {
  cerr << "should not in this function." << __FILE__ << __LINE__ << endl;
}

void Parameter::operator =(const std::vector<GLuint>&) {
  cerr << "should not in this function." << __FILE__ << __LINE__ << endl;
}

#define UNIFORM_VECTOR_SINGLE_SETTER(K, DT) \
template<> \
void Uniform<Vector<1, DT>>::operator =(DT data) { \
  glUniform1 ## K(id_, data); \
}

#define UNIFORM_VECTOR_SETTER(N, K, DT) \
template<> \
void Uniform<Vector<N, DT>>::operator =(const DT *data) { \
  glUniform ## N ## K ## v(id_, 1, data); \
} \
template<> \
void Uniform<Vector<N, DT>>::operator =(const std::vector<DT> &data) { \
  glUniform ## N ## K ## v(id_, data.size() / N, &data.front()); \
}
//template<> \
//void Uniform<Vector<N, DT>>::operator =( \
//    const Vector<N, DT> &vec) { \
//  glUniform ## N ## K ## v(id_, 1, vec.data); \
//}

#define UNIFORM_VECTOR_SETTERS(K, DT) \
UNIFORM_VECTOR_SETTER(1, K, DT) \
UNIFORM_VECTOR_SETTER(2, K, DT) \
UNIFORM_VECTOR_SETTER(3, K, DT) \
UNIFORM_VECTOR_SETTER(4, K, DT)
//UNIFORM_VECTOR_SINGLE_SETTER(K, DT) \

UNIFORM_VECTOR_SETTERS(f, GLfloat)
UNIFORM_VECTOR_SETTERS(i, GLint)

#define UNIFORM_MATRIX_SETTER(R, C, DT) \
template<> \
void Uniform<Matrix<R, C, DT>>::operator =(const DT* data) { \
  glUniformMatrix ## R ## x ## C ## fv(id_, 1, false, data);\
} \
template<> \
void Uniform<Matrix<R, C, DT>>::operator =(const std::vector<DT> &data) { \
  glUniformMatrix ## R ## x ## C ## fv(id_, data.size() / (R*C), false, \
      &data.front());\
}

#define UNIFORM_MATRIX_N_SETTER(R, DT) \
template<> \
void Uniform<Matrix<R, R, DT>>::operator =(const DT* data) { \
  glUniformMatrix ## R ## fv(id_, 1, false, data); \
} \
template<> \
void Uniform<Matrix<R, R, DT>>::operator =(const std::vector<DT> &data) { \
  glUniformMatrix ## R ## fv(id_, data.size() / (R*R), false, &data.front()); \
}

UNIFORM_MATRIX_SETTER(2, 3, GLfloat)
UNIFORM_MATRIX_SETTER(2, 4, GLfloat)
UNIFORM_MATRIX_SETTER(3, 2, GLfloat)
UNIFORM_MATRIX_SETTER(3, 4, GLfloat)
UNIFORM_MATRIX_SETTER(4, 2, GLfloat)
UNIFORM_MATRIX_SETTER(4, 3, GLfloat)
UNIFORM_MATRIX_N_SETTER(2, GLfloat)
UNIFORM_MATRIX_N_SETTER(3, GLfloat)
UNIFORM_MATRIX_N_SETTER(4, GLfloat)

#define ATTRIBUTE_VECTOR_SETTER(N, K, DT) \
template<> \
void Attribute<Vector<N, DT>>::operator =(const std::vector<DT> &data) { \
  buffer_.Bind(GL_ARRAY_BUFFER); \
  glBufferData(GL_ARRAY_BUFFER, sizeof(data.front()) * data.size(), \
      &data.front(), GL_STATIC_DRAW); \
  glVertexAttribPointer(id_, N, K, GL_FALSE, 0, 0); \
}
//template<> \
//void Attribute<Vector<N, DT>>::operator =( \
//    const Vector<N, DT> &vec) { \
//  buffer_.Bind(GL_ARRAY_BUFFER); \
//  glBufferData(GL_ARRAY_BUFFER, sizeof(DT) * N, vec.data, GL_STATIC_DRAW); \
//  glVertexAttribPointer(id_, N, K, GL_FALSE, 0, 0); \
//}

#define ATTRIBUTE_VECTOR_SETTERS(K, DT) \
ATTRIBUTE_VECTOR_SETTER(1, K, DT) \
ATTRIBUTE_VECTOR_SETTER(2, K, DT) \
ATTRIBUTE_VECTOR_SETTER(3, K, DT) \
ATTRIBUTE_VECTOR_SETTER(4, K, DT)

ATTRIBUTE_VECTOR_SETTERS(GL_FLOAT, GLfloat)

}  // namespace gl

