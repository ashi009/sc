#ifndef LIB_GL_SHADER_H_
#define LIB_GL_SHADER_H_

#include <memory>
#include <string>
#include <GL/glew.h>

namespace gl {

class Program;

// Shader Resource. Sample usage:
//    ShaderResource vertex(new VertexShader("vertext.glslv"));
//    ShaderResource vertex = std::make_shared<VertexShader>("vertext.glslv");
// Shader Resource cannot be copied, due to the resource's lifetime. If copy
// a Shader Resource, they would share the same shader id and when one Shader
// Resource be deconstructed, the resource will be deallocated. At the same time
// the other Shader's resource will become invalid. So I made Shader uncopyable,
// to make sure there is no accidentally copying.
// The only valid way to use Shader is to use it as ShaderResource, which is a
// shared_ptr<Shader> instance.
// Q: Why not make Shader a string-like object to hide its pointer property?
// A: As Shader Resource is allocated in GPU space, so I use this explict design
//    which preserved literal correctness.
class Shader {

 public:
  Shader(const std::string &path, GLenum shader_type);
  Shader(const Shader &) = delete;
  Shader & operator = (const Shader &) = delete;
  ~Shader();
  
  const std::string &source() { return source_; }

 protected:
  GLuint id_;

 private:
  std::string source_;
  
  friend class Program;

};

class VertexShader : public Shader {
 public:
  VertexShader(const std::string &path);
};

class FragmentShader : public Shader {
 public:
  FragmentShader(const std::string &path);
};

typedef std::shared_ptr<Shader> ShaderResource;

} // namespace gl

#endif /* LIB_GL_SHADER_H_ */

