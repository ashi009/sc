#ifndef LIB_GL_PROGRAM_INL_H_
#define LIB_GL_PROGRAM_INL_H_

namespace gl {

template<typename T>
void Program::BindAttribute(const std::string &&name) {
  parameter_setters_[name] = std::make_shared<Attribute<T>>(
      glGetAttribLocation(id_, name.c_str()));
}

template<typename T>
void Program::BindUniform(const std::string &&name) {
  parameter_setters_[name] = std::make_shared<Uniform<T>>(
      glGetUniformLocation(id_, name.c_str()));
}

}  // namespace gl

#endif /* LIB_GL_PROGRAM_INL_H_ */

