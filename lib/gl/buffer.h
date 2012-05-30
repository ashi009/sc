#ifndef LIB_GL_BUFFER_H_
#define LIB_GL_BUFFER_H_

#include <memory>
#include <vector>
#include <GL/glew.h>

namespace gl {

class Buffer;

class BufferAccessor {
  
 public:
  BufferAccessor(GLuint);
  void Bind(GLenum);

 private:
  GLuint id_;

  friend class Buffer;

};

class Buffer : public BufferAccessor {

 public:
  Buffer(int = 1);
  Buffer(const Buffer&) = delete;
  Buffer &operator =(const Buffer&) = delete;
  ~Buffer();
  
  BufferAccessor operator [](int);
  
 protected:
  std::vector<GLuint> ids_;

};

typedef std::shared_ptr<Buffer> BufferResource;

}  // namespace gl

#endif /* LIB_GL_BUFFER_H_ */

