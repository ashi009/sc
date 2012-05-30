#ifndef LIB_GL_PARAMETER_INL_H_
#define LIB_GL_PARAMETER_INL_H_

namespace gl {

template<typename T>
Uniform<T>::Uniform(GLint id) : Parameter(id) {

}

template<int R, int C, typename T>
Uniform<Matrix<R, C, T>>::Uniform(GLint id) : Parameter(id) {

}

template<typename T>
Attribute<T>::Attribute(GLint id) : Parameter(id) {
  glEnableVertexAttribArray(id_);
}

template<typename T>
Attribute<T>::~Attribute() {
  glDisableVertexAttribArray(id_);
}

}  // namespace gl

#endif /* LIB_GL_PARAMETER_INL_H_ */

