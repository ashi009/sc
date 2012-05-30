#ifndef LIB_GL_PROGRAM_H_
#define LIB_GL_PROGRAM_H_

#include <string>
#include <memory>
#include <list>
#include <map>
#include <GL/glew.h>
#include "parameter.h"
#include "shader.h"

namespace gl {

class Program {

 public:
  Program();
  Program(const Program &) = delete;
  Program & operator = (const Program &) = delete;
  ~Program();
  
  void Attach(ShaderResource shader);
  void Detach(ShaderResource shader);
  void Link();
  void Use();

  template<typename T>
  void BindAttribute(const std::string &&name);
  template<typename T>
  void BindUniform(const std::string &&name);
  ParameterSetter GetParameterSetter(const std::string &name);

 private:
  GLuint id_;
  std::list<ShaderResource> shaders_;
  std::map<std::string, ParameterSetter> parameter_setters_;

};

typedef std::shared_ptr<Program> ProgramResource;

}  // namespace gl

#include "program-inl.h"

#endif /* LIB_GL_PROGRAM_H_ */

