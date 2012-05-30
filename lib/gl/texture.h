#ifndef LIB_GL_TEXTURE_H_
#define LIB_GL_TEXTURE_H_

#include <memory>
#include <vector>
#include <GL/glew.h>

namespace gl {

class Texture;

class TextureAccessor {

 public:
  TextureAccessor(GLuint);
  void Bind(GLenum);

 private:
  GLuint id_;
  
  friend class Texture;

};

class Texture : public TextureAccessor {

 public:
  Texture(int = 1);
  Texture(const Texture&) = delete;
  Texture& operator =(const Texture&) = delete;
  ~Texture();
  
  TextureAccessor operator [](int);
  
 protected:
  std::vector<GLuint> ids_;

};


typedef std::shared_ptr<Texture> TextureResource;

}  // namespace gl

#endif /* LIB_GL_TEXTURE_H_ */

