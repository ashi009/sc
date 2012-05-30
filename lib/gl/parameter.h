#ifndef LIB_GL_PARAMETER_H_
#define LIB_GL_PARAMETER_H_

#include <vector>
#include <GL/glew.h>
#include "buffer.h"
#include "data.h"

namespace gl {

class Parameter {
 public:
  GLuint id() { return id_; }
  virtual void operator =(GLfloat);
  virtual void operator =(GLint);
  virtual void operator =(const GLfloat*);
  virtual void operator =(const GLint*);
  virtual void operator =(const std::vector<GLfloat>&);
  virtual void operator =(const std::vector<GLuint>&);
 protected:
  Parameter(GLint id);
  GLuint id_;
};

template<typename T>
class Uniform : public Parameter {
 public:
  Uniform(GLint id);
  virtual void operator =(const typename T::data_t*);
  virtual void operator =(const std::vector<typename T::data_t>&);
};

template<int R, int C, typename T>
class Uniform<Matrix<R, C, T>> : public Parameter {
 public:
  Uniform(GLint id);
  virtual void operator =(const T*);
  virtual void operator =(const std::vector<T>&);
};

template<typename T>
class Attribute : public Parameter {
 public:
  Attribute(GLint id);
  ~Attribute();
  virtual void operator =(const std::vector<typename T::data_t>&);
 private:
  Buffer buffer_;
};

typedef std::shared_ptr<Parameter> ParameterSetter;

}  // namespace gl

#include "parameter-inl.h"

#endif /* LIB_GL_PARAMETER_H_ */

