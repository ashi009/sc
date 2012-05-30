#include <cassert>
#include "buffer.h"

namespace gl {

BufferAccessor::BufferAccessor(GLuint id) : id_(id) {

}

void BufferAccessor::Bind(GLenum target) {
  glBindBuffer(target, id_);
}

Buffer::Buffer(int number_of_buffers) : BufferAccessor(0), 
    ids_(number_of_buffers) {
  assert(number_of_buffers > 0);
  glGenBuffers(number_of_buffers, &ids_.front());
  id_ = ids_.front();
}

Buffer::~Buffer() {
  glDeleteBuffers(ids_.size(), &ids_.front());
}

BufferAccessor Buffer::operator [](int n) {
  return BufferAccessor(ids_[n]);
}

}  // namespace gl

