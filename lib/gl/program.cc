#include <cstdlib>
#include <cassert>
#include <iostream>
#include "program.h"

using std::string;
using std::cerr;
using std::endl;

namespace gl {

Program::Program() : id_(glCreateProgram()) {

}

Program::~Program() {
  glDeleteProgram(id_);
}

void Program::Attach(ShaderResource shader) {
  glAttachShader(id_, shader->id_);
  shaders_.push_back(shader);
}

void Program::Detach(ShaderResource shader) {
  glDetachShader(id_, shader->id_);
  shaders_.remove(shader);
}

// Link program.  This method will throw an exception, when having linking
// problems.  It is intented to throw rather than return bool, as it *should
// not* have any runtime error when release.
//
// This method is equivalent to:
//    glLinkProgram(programId);
void Program::Link() {

  // try to link program.
  glLinkProgram(id_);

  // check linking result.
  GLint status;
  glGetProgramiv(id_, GL_LINK_STATUS, &status);

  // if failed to link
  if (!status) {
    GLint length;
    // retrieve linking info's length.
    glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &length);
    char buffer[length];
    // retrieve linking info.
    glGetShaderInfoLog(id_, length, &length, buffer);
    cerr << buffer << endl;
    abort();
  }

}

void Program::Use() {
  glUseProgram(id_);
}

ParameterSetter Program::GetParameterSetter(const string &name) {
  auto it = parameter_setters_.find(name);
  assert(it != parameter_setters_.end());
  return it->second;
}

}  // namespace gl

