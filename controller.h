#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#ifndef GLFWTHREAD
#include <mutex>
#endif
#include <unordered_map>
#include <tuple>
#include <vector>
#include "appinfo.h"
#include "gl.h"
#include "vc.h"

namespace sc {

enum CameraMode : int {
  kPerspective = 0,
  kOrthographic
};

enum PBOStatus : int {
  kUnuesd = 0,
  kAllocated,
  kFilled,
  kCopied
};

class Controller : public vc::timing::Ticker {

 public:
  Controller(int width, int height);
  Controller(const Controller&) = delete;
  Controller &operator = (const Controller&) = delete;
  virtual ~Controller();

  struct Parameter {
    gl::Matrix<4> matrix;
    gl::Vector<3> hsb_tune;
  };
  
  typedef std::unordered_map<std::string, std::tuple<vc::Source*, 
      vc::timing::Timer*, std::vector<Parameter>>> Collection;
  typedef std::function<void(const Collection&)> DataAccessor;

  void BeforeStart();
  void BeforeEnd();
  bool Tick(vc::timing::TickId, const vc::timing::TimePoint&);

  void Reset();
  bool AddSource(const std::string&, const std::string&);
  bool RemoveSource(const std::string&);
  vc::Source *GetSource(const std::string&);

  bool SetMatrix(const std::string&, int, const std::vector<float>&);
  bool SetColorTune(const std::string&, int, float, float, float);
  bool DelaySource(const std::string&, int);

#ifdef CPU
  void matrix_world(const std::vector<float>&);
  void matrix_post(const std::vector<float>&);
  void mode(CameraMode);
  void background_color(const std::vector<float>&);
#else  // ifndef CPU
  void matrix_world(const std::vector<float> &m) { matrix_world_ = m; }
  void matrix_post(const std::vector<float> &m) { matrix_post_ = m; }
  void mode(CameraMode new_mode) { mode_ = new_mode; }
  void background_color(const std::vector<float> &m) { background_color_ = m; }
#endif  // ifndef CPU

  const Collection &data() { return data_; }
  const gl::Matrix<4> &matrix_world() { return matrix_world_; }
  const gl::Matrix<4> &matrix_post() { return matrix_post_; }
  CameraMode mode() { return mode_; }
  const gl::Vector<3> background_color() { return background_color_; }

 private:
#ifndef GLFWTHREAD
  std::mutex lock_;
#else  // ifdef GLFWTHREAD
  GLFWmutex lock_;
#endif  // ifdef GLFWTHREAD
  Collection data_;
  gl::Matrix<4> matrix_world_, matrix_post_;
  CameraMode mode_;
  gl::Vector<3> background_color_;

  gl::ShaderResource vertex_shader_, fragment_shader_;
  gl::ProgramResource program_;
  gl::TextureResource texture_;
  gl::ParameterSetter module_matrix_ptr_, projection_matrix_ptr_;
  gl::ParameterSetter module_hsb_tune_ptr_;
  gl::ParameterSetter vertex_pos_ptr_, texture_pos_ptr_;
  
#ifdef CPU

  struct PixelInfo {
    int step;       // 2's step-th power (-1, suggest an empty pixel)
    vc::Feed* feed; // pixel's source feed.
    int indices[8]; // pixels in the refering image frame.
    float z;
  };
  PixelInfo *pixel_info_;
  GLubyte canvas_bg_[3];
  GLubyte *texture_bitmap_;
  
  void Preprocess();
  void Paint(int);

#else  // ifndef CPU
  
#ifdef PBO

  GLubyte *texture_storage_;
  gl::BufferResource pixel_buffers_;
  
  struct PBOInfo {
    int index;
    int width, height;
    float normalized_width, normalized_height;
    float *matrix, *hsb_tune;
    void *mapped_address, *data;
    PBOStatus status;
  };
  PBOInfo pbo_info_[2];
  int pbo_index_;

  bool CopyPBO(PBOInfo&);
  bool FillPBO(PBOInfo&);
  bool RenderPBO(PBOInfo&);

#endif  // ifdef PBO

#endif  // ifdef CPU

};

}  // namespace sc

#endif /* CONTROLLER_H_ */

