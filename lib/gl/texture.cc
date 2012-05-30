#include <cassert>
#include "texture.h"

namespace gl {

TextureAccessor::TextureAccessor(GLuint id) : id_(id) {

}

void TextureAccessor::Bind(GLenum target) {
  glBindTexture(target, id_);
}

Texture::Texture(int number_of_texture) : TextureAccessor(0), 
    ids_(number_of_texture) {
  assert(number_of_texture > 0);
  glGenTextures(number_of_texture, &ids_.front());
  id_ = ids_.front();
}

Texture::~Texture() {
  glDeleteTextures(ids_.size(), &ids_.front());
}

TextureAccessor Texture::operator [](int n) {
  return TextureAccessor(ids_[n]);
}

}  // namespace gl

