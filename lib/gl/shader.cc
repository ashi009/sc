#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <iterator>
#include <GL/glew.h>
#include "shader.h"

using std::cerr;
using std::endl;
using std::string;
using std::ifstream;
using std::istreambuf_iterator;

namespace gl {

Shader::Shader(const string &path, GLenum shader_type) :
    id_(glCreateShader(shader_type)) {

  ifstream ifs(path.c_str());

  if (!ifs.good()) {
    cerr << "Cannot read specified shader: " + path << endl;
    abort();
  }

  // use () to make sure string source() not being interperated as a function
  // declaration.
  source_ = string((istreambuf_iterator<char>(ifs)), 
      istreambuf_iterator<char>());

  // prepare arguments for glShaderSource.
  const char *sources[] = { source_.c_str() };
  const int sources_lengths[] = { (int)source_.size() };

  // attach source to shader id.
  glShaderSource(id_, 1, sources, sources_lengths);

  // compile GLSL source.
  glCompileShader(id_);

  // check if shader is compiled.
  GLint status;
  glGetShaderiv(id_, GL_COMPILE_STATUS, &status);

  // if shader cannot be compiled.
  if (!status) {
    GLint length;
    // retrieve compiling info's length.
    glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &length);
    char buffer[length];
    // retrieve compiling info.
    glGetShaderInfoLog(id_, length, &length, buffer);
    cerr << buffer << endl;
    abort();
  }

}

Shader::~Shader() {
  glDeleteShader(id_);
}

VertexShader::VertexShader(const string &path) : 
    Shader(path, GL_VERTEX_SHADER) {

}

FragmentShader::FragmentShader(const string &path) : 
    Shader(path, GL_FRAGMENT_SHADER) {

}

}  // namespace gl

