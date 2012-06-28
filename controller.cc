#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include <functional>
#ifdef CPU
#include <thread>
#include <vector>
#endif  // ifdef CPU
#include "appinfo.h"
#include "controller.h"

using namespace std;

using std::string;
using std::get;
using std::make_tuple;
using std::vector;
#ifndef GLFWTHREAD
using std::thread;
using std::mutex;
using std::lock_guard;
#endif  // ifndef GLFWTHREAD
using std::forward;
using std::istringstream;
using std::make_pair;
using cv::VideoCapture;
using vc::Source;
using vc::CameraSource;
using vc::FileSource;
using vc::Feed;
using vc::timing::TickId;
using vc::timing::Clock;
using vc::timing::Timer;
using vc::timing::TimePoint;
using gl::Buffer;
using gl::Texture;
using gl::VertexShader;
using gl::FragmentShader;
using gl::Program;
using gl::Matrix;
using gl::Vector;
using gl::matrix::I;

namespace sc {

const int kMaxStreamWidth = 1920;
const int kMaxStreamHeight = 1080;

#ifdef PBO
const float kOneOverMaxStreamWidth = 1.0f / kMaxStreamWidth;
const float kOneOverMaxStreamHeight = 1.0f / kMaxStreamHeight;
#endif  // ifdef PBO

const float kViewNear = 0.1;
const float kViewFar = 1000;

const Vector<3> kStandardCords[4] = {
  vector<GLfloat> {-1, -1, 0},
  vector<GLfloat> {-1, 1, 0},
  vector<GLfloat> {1, -1, 0},
  vector<GLfloat> {1, 1, 0}
};

const vector<GLfloat> kVertexPositions = {
  -1.0, -1.0,
  -1.0, 1.0,
  1.0, -1.0,
  1.0, 1.0
};

int g_width, g_height;

#ifdef CPU

// this will be used to check if the mapping cache need to be updated.
int g_version = 0;

int g_canvas_width, g_canvas_height;
int g_canvas_version;
int g_stripe_height;

#ifdef __MINGW32__
SYSTEM_INFO sysinfo;
GetSystemInfo(&sysinfo);
const int kCoreCount = sysinfo.dwNumberOfProcessors;
#else  // ifndef __MINGW32__
const int kCoreCount = sysconf(_SC_NPROCESSORS_ONLN);
#endif  // ifndef __MINGW32__

bool inPixel(Vector<3> &vertex, int x, int y) {
  return vertex.data[0] >= x && vertex.data[0] <= x+1 &&
      vertex.data[1] >= y && vertex.data[1] <= y+1;
}

bool inPolygen(int x, int y, Vector<3> vertices[]) {
  int count_positive = 0, count_negative = 0;
  for (int i = 0; i < 4; i++) {
    float v0x = x - vertices[i].data[0];
    float v0y = x - vertices[i].data[1];
    float v1x = vertices[(i+1) & 3].data[0] - vertices[i].data[0];
    float v1y = vertices[(i+1) & 3].data[1] - vertices[i].data[1];
    float product = v0x * v1y - v0y * v1x;
    if (product < 0)
      count_negative++;
    else if (product > 0)
      count_positive++;
  }
  return count_positive == 0 || count_negative == 0;
}

#endif  // ifndef CPU

void WindowResizeListener(int width, int height) {

  g_width = width;
  g_height = height;
  glViewport(0, 0, width, height);

#ifdef CPU
  g_version++;
#endif  // ifndef CPU

}

int WindowCloseHandler() {

  return GL_FALSE;

}

Controller::Controller(int width, int height) {

  // XXX Init GLFW context here, before all the threads start. This is very 
  // important, as GLFW on NVidia machines may have strange behavious, when 
  // GLFW initialized in a thread, and the thread was not the main thread.
  if (!glfwInit()) {
    cerr << "Failed to initialize GLFW." << endl;
    exit(EXIT_FAILURE);
  }
  
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);

  g_width = width;
  g_height = height;

  if(!glfwOpenWindow(width, height, 8, 8, 8, 0, 8, 0, GLFW_WINDOW)) {
    glfwTerminate();
    cerr << "Cannot open OpenGL 2.1 render context." << endl;
    exit(EXIT_FAILURE);
  }

  if (glewInit() != GLEW_OK) {
    glfwTerminate();
    cerr << "Cannot initialize GLEW." << endl;
    exit(EXIT_FAILURE);
  }

  glfwSetWindowTitle(APP_TITLE);
  glfwSetWindowSizeCallback(WindowResizeListener);
  glfwSetWindowCloseCallback(WindowCloseHandler);

#ifdef GLFWTHREAD
  lock_ = glfwCreateMutex();
#endif  // ifdef GLFWTHREAD

}

Controller::~Controller() {

#ifdef GLFWTHREAD
  glfwDestroyMutex(lock_);
#endif  // ifdef GLFWTHREAD

  glfwTerminate();

}

void Controller::BeforeStart() {

  // XXX GL_UNPACK_ALIGNMENT = 3 caused some problem when test with a video of
  // 854x480 resolution. After changing its value to 1, the problem resolved.
  // To read more insights, please check the link below.
  // http://www.opengl.org/archives/resources/features/KilgardTechniques/oglpitfall/
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

  glEnable(GL_DEPTH_TEST | GL_TEXTURE_2D);
  glDepthFunc(GL_LEQUAL);

  vertex_shader_ = make_shared<VertexShader>("shader/vertex.glslv");
  fragment_shader_ = make_shared<FragmentShader>("shader/fragment.glslf");
  program_ = make_shared<Program>();
  texture_ = make_shared<Texture>();

  // Attach shaders to program, one for vertex transformation and one for
  // fragment rendering.
  program_->Attach(vertex_shader_);
  program_->Attach(fragment_shader_);
  // Link shaders to one program.
  program_->Link();
  // Ask OpenGL render context to use this program for scence rendering instead
  // of using OpenGL C API's builtin mechanism.
  program_->Use();

  // Binding Parameters to program, in fact this is to binding parameter types
  // to parameter name. Later then one may retrieve a parameter setter by using
  // program's GetParameterSetter(name).
  // XXX These should be done when attach a shader, but for the moment, it has
  // to be done manually.
  program_->BindUniform<Matrix<4, 4>>("uMVMatrix");
  program_->BindUniform<Matrix<4, 4>>("uPMatrix");
  program_->BindUniform<Vector<3, GLfloat>>("uHsbTune");
  program_->BindAttribute<Vector<2, GLfloat>>("aVertexPosition");
  program_->BindAttribute<Vector<2, GLfloat>>("aTextureCoord");

  // Retrieve parameter setters.
  module_matrix_ptr_ = program_->GetParameterSetter("uMVMatrix");
  projection_matrix_ptr_ = program_->GetParameterSetter("uPMatrix");
  module_hsb_tune_ptr_ = program_->GetParameterSetter("uHsbTune");
  vertex_pos_ptr_ = program_->GetParameterSetter("aVertexPosition");
  texture_pos_ptr_ = program_->GetParameterSetter("aTextureCoord");

  // Specify default vertex positions, which is constant.
  *vertex_pos_ptr_ = kVertexPositions;

  texture_->Bind(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

#ifdef PBO

  // Set up a series PBO buffers to minimizing texture MEM-VRAM copying time.
  pixel_buffers_ = make_shared<Buffer>(2);
  pbo_index_ = 0;
  // Prepare texture_storage_ for PBO switching.
  texture_storage_ = new GLubyte[kMaxStreamWidth * kMaxStreamHeight * 3];
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, kMaxStreamWidth, kMaxStreamHeight, 0,
      GL_BGR, GL_UNSIGNED_BYTE, texture_storage_);

#else  // ifdef PBO

  *texture_pos_ptr_ = vector<GLfloat> {
    0.0, 1.0,
    0.0, 0.0,
    1.0, 1.0,
    1.0, 0.0
  };

#ifdef CPU
  *projection_matrix_ptr_ = gl::matrix::I().data;
  *module_matrix_ptr_ = gl::matrix::I().data;
  
  pixel_info_ = nullptr;
  g_canvas_width = 0;
  g_canvas_height = 0;
  g_canvas_version = -1;
  texture_bitmap_ = nullptr;
  
  g_stripe_height = 0;
  
#endif  // ifdef CPU

#endif  // ifdef PBO

  // Call reset to initiate some scence parameters.
  Reset();

}

void Controller::BeforeEnd() {

  Reset();

  // Free all the resources, since all the destroying work will be invalid
  // after exit OpenGL render context.
  vertex_shader_.reset();
  fragment_shader_.reset();
  module_matrix_ptr_.reset();
  projection_matrix_ptr_.reset();
  module_hsb_tune_ptr_.reset();
  vertex_pos_ptr_.reset();
  texture_pos_ptr_.reset();
  program_.reset();
  texture_.reset();
  
#ifdef CPU
  if (texture_bitmap_) delete[] texture_bitmap_;
  if (pixel_info_) delete[] pixel_info_;
#else  // ifndef CPU

#ifdef PBO
  pixel_buffers_.reset();
  delete[] texture_storage_;
#endif  // ifdef PBO

#endif  // ifndef CPU

}

bool Controller::Tick(TickId actual_frame, const TimePoint &epoch) {

#ifndef GLFWTHREAD
  lock_guard<mutex> sync(lock_);
#else  // ifdef GLFWTHREAD
  glfwLockMutex(lock_);
#endif  // ifdef GLFWTHREAD
  TimePoint now = Clock::now();

  glfwSwapBuffers();
  glClearColor(background_color_[0], background_color_[1],
      background_color_[2], 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef CPU
  if (g_version != g_canvas_version)
    Preprocess();
#else  // ifndef CPU
  // Calculates projection matrix.
  Matrix<4> projection_matrix = matrix_post_ * (mode_ == kPerspective ?
        gl::matrix::Perspective(45,  static_cast<float>(g_width) / g_height,
            kViewNear, kViewFar) :
        gl::matrix::Orthographic(g_width, g_height, -kViewFar, kViewFar)
      ) * matrix_world_;
  // Setss projection_matrix.
  *projection_matrix_ptr_ = projection_matrix.data;
#endif  // ifndef CPU

  for (auto it = data_.begin(); it != data_.end(); it++) {

    auto *source = get<0>(it->second);
    auto &feeds = source->feeds();

#ifdef CPU

    for (int i = 0; i < feeds.size(); i++) {
      // Query a feed to update their current_image.
      feeds[i]->Query(now);
    }

#else  // ifndef CPU

    auto &params = get<2>(it->second);

    int width = source->width();
    int height = source->height();

#ifdef PBO
    float normalized_width = width * kOneOverMaxStreamWidth;
    float normalized_height = height * kOneOverMaxStreamHeight;
    int next_index = pbo_index_;
#endif  // ifdef PBO

    for (int i = 0; i < feeds.size(); i++) {
      // Query a feed to update their current_image.
      feeds[i]->Query(now);
      // Get feed's current_image data.
      auto data = feeds[i]->current_image();

#ifdef PBO
      // Always check if current pbo needs to be copied to texture.
      if (CopyPBO(pbo_info_[pbo_index_]))
        next_index = pbo_index_ ^ 1;
#endif  // ifdef PBO

      // If there are no data in the feed's current_image ptr, skip it.
      if (!data)
        continue;

      // Check if the feed's target is off the screen, if so, skip it.
      Matrix<4> final_matrix = projection_matrix * params[i].matrix;
      Vector<3> pos_inits = final_matrix.Transform(kStandardCords[0]);
      float x_min = pos_inits.data[0], x_max = pos_inits.data[0];
      float y_min = pos_inits.data[1], y_max = pos_inits.data[1];
      float z_min = pos_inits.data[2], z_max = pos_inits.data[2];
      for (int j = 1; j < 4; j++) {
        Vector<3> pos = final_matrix.Transform(kStandardCords[j]);
        if (pos.data[0] < x_min) x_min = pos.data[0];
        else if (pos.data[0] > x_max) x_max = pos.data[0];
        if (pos.data[1] < y_min) y_min = pos.data[1];
        else if (pos.data[1] > y_max) y_max = pos.data[1];
        if (pos.data[2] < z_min) z_min = pos.data[2];
        else if (pos.data[2] > z_max) z_max = pos.data[2];
      }
      if (x_max == x_min || y_max == y_min ||
          x_max <= -1 || x_min >= 1 ||
          y_max <= -1 || y_min >= 1 ||
          z_max < -1 || z_min > 1)
        continue;

#ifdef PBO
      // Allocated a memory buffer, then ask to fill if with data.
      (*pixel_buffers_)[next_index].Bind(GL_PIXEL_UNPACK_BUFFER);
      glBufferData(GL_PIXEL_UNPACK_BUFFER, width * height * 3, nullptr,
          GL_STREAM_DRAW);
      auto mapped_address = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
      if (mapped_address) {
        pbo_info_[next_index] = {
          next_index,
          width , height,
          normalized_width, normalized_height,
          params[i].matrix.data, params[i].hsb_tune.data,
          mapped_address, data,
          kAllocated
        };
        FillPBO(pbo_info_[next_index]);
      }

      glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
      if (RenderPBO(pbo_info_[pbo_index_]))
        pbo_index_ = next_index;
#else  // ifndef PBO

      *module_matrix_ptr_ =  params[i].matrix.data;
      *module_hsb_tune_ptr_ = params[i].hsb_tune.data;
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR,
          GL_UNSIGNED_BYTE, data);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#endif  // ifndef PBO

    }
    
#endif  // ifndef CPU

  }

#ifdef CPU

  vector<thread> workers;
  for (int i = 0, y = 0; i < kCoreCount; i++, y += g_stripe_height)
    workers.emplace_back(thread(&Controller::Paint, this, y));
  for (int i = 0; i < kCoreCount; i++)
    workers[i].join();

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, g_canvas_width, g_canvas_height, 0,
      GL_RGB, GL_UNSIGNED_BYTE, texture_bitmap_);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#else  // ifndef CPU

#ifdef PBO
  if (RenderPBO(pbo_info_[pbo_index_]))
    pbo_index_ ^= 1;
#endif  // ifdef PBO

#endif  // ifdef CPU

#ifdef GLFWTHREAD
  glfwUnlockMutex(lock_);
#endif  // ifdef GLFWTHREAD

  return true;
}

#ifdef CPU

void Controller::Preprocess() {

  g_canvas_version = g_version;
  
  if (g_canvas_height != g_height || g_canvas_width != g_width) {
    g_canvas_height = g_height;
    g_canvas_width = g_width;
    if (texture_bitmap_) delete[] texture_bitmap_;
    texture_bitmap_ = new GLubyte[g_canvas_width * g_canvas_height * 3];
    if (pixel_info_) delete[] pixel_info_;
    pixel_info_ = new PixelInfo[g_canvas_width * g_canvas_height];
    g_stripe_height = g_canvas_height / kCoreCount;
  }

  for (int i = g_canvas_width * g_canvas_height - 1; i >= 0; i--) {
    pixel_info_[i].z = 2;
    pixel_info_[i].step = -1;
  }

  canvas_bg_[0] = 255 * background_color_.data[0];
  canvas_bg_[1] = 255 * background_color_.data[1];
  canvas_bg_[2] = 255 * background_color_.data[2];

  // Calculates projection matrix.
  Matrix<4> projection_matrix = matrix_post_ * (mode_ == kPerspective ?
        gl::matrix::Perspective(45, static_cast<float>(g_canvas_width) /
            g_canvas_height, kViewNear, kViewFar) :
        gl::matrix::Orthographic(g_canvas_width, g_canvas_height,
            -kViewFar, kViewFar)
      ) * matrix_world_;

  Vector<3> polygen_vertices[4];

  PixelInfo *pixel_info;

  for (auto it = data_.begin(); it != data_.end(); it++) {

    auto *source = get<0>(it->second);
    auto &feeds = source->feeds();
    auto &params = get<2>(it->second);

    int width = source->width();
    int height = source->height();

    for (int i = 0; i < feeds.size(); i++) {

      Matrix<4> matrix =
          gl::matrix::Scale(g_canvas_width / 2.0f, -g_canvas_height / 2.0f, 1.0) *
          gl::matrix::Translate(1.0, -1.0, 0.0) *
          projection_matrix * params[i].matrix *
          gl::matrix::Translate(-1.0, 1.0, 0.0) *
          gl::matrix::Scale(2.0f / width, -2.0f / height, 1.0);

      for (int y = 0, index = 0; y < g_canvas_height; y++)
      for (int x = 0; x < g_canvas_width; x++, index++) {

        polygen_vertices[0] = matrix.Inverse(x, y);
        polygen_vertices[1] = matrix.Inverse(x+1, y);
        polygen_vertices[2] = matrix.Inverse(x+1, y+1);
        polygen_vertices[3] = matrix.Inverse(x, y+1);

        float x_min = polygen_vertices[0].data[0];
        float x_max = polygen_vertices[0].data[0];
        float y_min = polygen_vertices[0].data[1];
        float y_max = polygen_vertices[0].data[1];
        float z_min = polygen_vertices[0].data[2];
        float z_max = polygen_vertices[0].data[2];
        float z_avg = polygen_vertices[0].data[2];

        for (int j = 1; j < 4; j++) {
          if (polygen_vertices[j].data[0] < x_min)
            x_min = polygen_vertices[j].data[0];
          else if (polygen_vertices[j].data[0] > x_max)
            x_max = polygen_vertices[j].data[0];
          if (polygen_vertices[j].data[1] < y_min)
            y_min = polygen_vertices[j].data[1];
          else if (polygen_vertices[j].data[1] > y_max)
            y_max = polygen_vertices[j].data[1];
          if (polygen_vertices[j].data[2] < z_min)
            z_min = polygen_vertices[j].data[2];
          else if (polygen_vertices[j].data[2] > z_max)
            z_max = polygen_vertices[j].data[2];
          z_avg += polygen_vertices[j].data[2];
        }
        if (x_max == x_min || y_max == y_min ||
            x_max < 0.0 || x_min > width ||
            y_max < 0.0 || y_min > height ||
            z_max < -1.0 || z_min > 1.0)
          continue;
        z_avg *= 0.25;

        pixel_info = &pixel_info_[index];
        if (z_avg > pixel_info->z)
          continue;

        int count = 0;

        for (int sy = max(static_cast<int>(floor(y_min)), 0),
            sy_max = min(static_cast<int>(ceil(y_max)), height),
            sx_min = max(static_cast<int>(floor(x_min)), 0),
            sx_max = min(static_cast<int>(floor(x_max)), width);
            count < 8 && sy < sy_max; sy++)
        for (int sx = sx_min; count < 8 && sx < sx_max; sx++)
          if (inPixel(polygen_vertices[0], sx, sy) ||
              inPixel(polygen_vertices[1], sx, sy) ||
              inPixel(polygen_vertices[2], sx, sy) ||
              inPixel(polygen_vertices[3], sx, sy) ||
              inPolygen(sx, sy, polygen_vertices) ||
              inPolygen(sx+1, sy, polygen_vertices) ||
              inPolygen(sx+1, sy+1, polygen_vertices) ||
              inPolygen(sx, sy+1, polygen_vertices))
            pixel_info->indices[count++] = (sy * width + sx) * 3;

        if (count > 0) {
          pixel_info->feed = feeds[i];
          pixel_info->z = z_avg;
          if (count == 8)
            pixel_info->step = 3;
          else if (count >= 4)
            pixel_info->step = 2;
          else if (count >= 2)
            pixel_info->step = 1;
          else
            pixel_info->step = 0;
        }
      }

    }

  }

}

void Controller::Paint(int y_begin) {
  
  Controller::PixelInfo *pixel_info = pixel_info_;
  pixel_info += y_begin * g_canvas_width;
  
  GLubyte *texture_pixel_data, *source_pixel_data, *source_image;
  for (int y = y_begin, index = y * g_canvas_width * 3,
      y_max = min(y_begin + g_stripe_height, g_canvas_height); 
      y < y_max; y++)
  for (int x = 0; x < g_canvas_width; x++, index += 3, pixel_info++) {
    texture_pixel_data = &texture_bitmap_[index];
    if (pixel_info->step == -1) {
      memcpy(texture_pixel_data, canvas_bg_, 3);
    } else {
      texture_pixel_data[0] = 0;
      texture_pixel_data[1] = 0;
      texture_pixel_data[2] = 0;
      source_image = (GLubyte*)pixel_info->feed->current_image();
      if (source_image) {
        for (int i = (1 << pixel_info->step) - 1; i >= 0; i--) {
          source_pixel_data = &source_image[pixel_info->indices[i]];
          texture_pixel_data[0] += source_pixel_data[2] >> pixel_info->step;
          texture_pixel_data[1] += source_pixel_data[1] >> pixel_info->step;
          texture_pixel_data[2] += source_pixel_data[0] >> pixel_info->step;
        }
      }
    }
  }

}

void Controller::matrix_world(const std::vector<float> &m) {
#ifndef GLFWTHREAD
  lock_guard<mutex> sync(lock_);
#else  // ifdef GLFWTHREAD
  glfwLockMutex(lock_);
#endif  // ifdef GLFWTHREAD
  matrix_world_ = m;
  g_version++;
#ifdef GLFWTHREAD
  glfwUnlockMutex(lock_);
#endif  // ifdef GLFWTHREAD
}

void Controller::matrix_post(const std::vector<float> &m) {
#ifndef GLFWTHREAD
  lock_guard<mutex> sync(lock_);
#else  // ifdef GLFWTHREAD
  glfwLockMutex(lock_);
#endif  // ifdef GLFWTHREAD
  matrix_post_ = m;
  g_version++;
#ifdef GLFWTHREAD
  glfwUnlockMutex(lock_);
#endif  // ifdef GLFWTHREAD
}

void Controller::mode(CameraMode new_mode) {
#ifndef GLFWTHREAD
  lock_guard<mutex> sync(lock_);
#else  // ifdef GLFWTHREAD
  glfwLockMutex(lock_);
#endif  // ifdef GLFWTHREAD
  mode_ = new_mode;
  g_version++;
#ifdef GLFWTHREAD
  glfwUnlockMutex(lock_);
#endif  // ifdef GLFWTHREAD
}

void Controller::background_color(const std::vector<float> &m) {
#ifndef GLFWTHREAD
  lock_guard<mutex> sync(lock_);
#else  // ifdef GLFWTHREAD
  glfwLockMutex(lock_);
#endif  // ifdef GLFWTHREAD
  background_color_ = m; 
  g_version++;
#ifdef GLFWTHREAD
  glfwUnlockMutex(lock_);
#endif  // ifdef GLFWTHREAD
}

#else  // ifndef CPU

#ifdef PBO

bool Controller::FillPBO(Controller::PBOInfo &pbo_info) {

  if (pbo_info.status == kAllocated) {

    // XXX Actual copy can be done in a second thread, which is accupying
    // several more thread to do the copy work, and after the copy is done
    // push back to a copied queue, waiting to be moved to vmem.
    // Then render the moved texture with bound info.

    memcpy(pbo_info.mapped_address, pbo_info.data,
        pbo_info.width * pbo_info.height * 3);
    pbo_info.status = kFilled;

    return true;

  }

  return false;

}

bool Controller::CopyPBO(Controller::PBOInfo &pbo_info) {

  if (pbo_info.status == kFilled) {

    (*pixel_buffers_)[pbo_info.index].Bind(GL_PIXEL_UNPACK_BUFFER);

    // Release pointer
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

    // Copy PBO data to texture_storage_, this is weired indeed.
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pbo_info.width, pbo_info.height,
        GL_BGR, GL_UNSIGNED_BYTE, 0);

    pbo_info.status = kCopied;

    // return true for the moment.
    return true;

  }

  return false;

}

bool Controller::RenderPBO(Controller::PBOInfo &pbo_info) {

  if (pbo_info.status == kCopied) {

    *module_matrix_ptr_ = pbo_info.matrix;
    *module_hsb_tune_ptr_ = pbo_info.hsb_tune;

    // Specify texture positions.
    // XXX As OpenGL's coordinate system is different from screen coordindate
    // system, so it needs to be fliped in y-axis to make image looks portrait.
    *texture_pos_ptr_ = vector<GLfloat> {
      0.0, pbo_info.normalized_height,
      0.0, 0.0,
      pbo_info.normalized_width, pbo_info.normalized_height,
      pbo_info.normalized_width, 0.0
    };

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    pbo_info.status = kUnuesd;

    return true;
  }

  return false;

}

#endif  // ifdef PBO

#endif  // ifndef CPU

void Controller::Reset() {

  while (!data_.empty())
    RemoveSource(data_.begin()->first);

  // Set up world and post matrix.
  matrix_world_ = I();
  matrix_post_ = I();

  mode_ = kPerspective;
  background_color_ = vector<float> { 0, 0, 0 };
#ifdef CPU
  g_version++;
#endif  // ifdef CPU

}

bool Controller::AddSource(const string &name, const string &uri) {

  if (data_.find(name) != data_.end())
    return false;

  Source *source;
  int channels = 1;

  istringstream iss(uri);
  string protocol;
  if (getline(iss, protocol, ':')) {
    if (protocol == "camera") {
      char ch;
      int id;
      if (!(iss >> id >> ch >> channels))
        return false;
      if (id < 0 || channels < 1)
        return false;
      source = new CameraSource(id, channels);
    } else {
      source = new FileSource(uri);
    }
  } else {
    return false;
  }

  if (source->ok() &&
      source->width() > 0 && source->width() <= kMaxStreamWidth &&
      source->height() > 0 && source->height() <= kMaxStreamHeight) {
    Timer *timer = new Timer(source);
    timer->Start(source->fps());
#ifndef GLFWTHREAD
    lock_guard<mutex> sync(lock_);
    data_[name] = make_tuple(source, timer, vector<Parameter>(channels));
#ifdef CPU
    g_version++;
#endif  // ifdef CPU
#else  // ifdef GLFWTHREAD
    glfwLockMutex(lock_);
    data_[name] = make_tuple(source, timer, vector<Parameter>(channels));
#ifdef CPU
    g_version++;
#endif  // ifdef CPU
    glfwUnlockMutex(lock_);
#endif  // ifdef GLFWTHREAD
    return true;
  }
  delete source;
  return false;
}

bool Controller::RemoveSource(const string &name) {

  auto it = data_.find(name);
  if (it == data_.end())
    return false;
#ifndef GLFWTHREAD
  lock_guard<mutex> sync(lock_);
#else  // ifdef GLFWTHREAD
  glfwLockMutex(lock_);
#endif  // ifdef GLFWTHREAD
  Source* source = get<0>(it->second);
  Timer* timer = get<1>(it->second);
  delete timer;
  delete source;
  data_.erase(it);
#ifdef CPU
  g_version++;
#endif  // ifdef CPU
#ifdef GLFWTHREAD
  glfwUnlockMutex(lock_);
#endif  // ifdef GLFWTHREAD
  return true;

}

Source *Controller::GetSource(const string &name) {

  auto it = data_.find(name);
  if (it == data_.end())
    return nullptr;
  return get<0>(it->second);

}

bool Controller::DelaySource(const string &name, int new_delay) {

  auto it = data_.find(name);
  if (it == data_.end())
    return false;
#ifndef GLFWTHREAD
  lock_guard<mutex> sync(lock_);
#else  // ifdef GLFWTHREAD
  glfwLockMutex(lock_);
#endif  // ifdef GLFWTHREAD
  Source *source = get<0>(it->second);
  source->delay(new_delay);
#ifdef CPU
  g_version++;
#endif  // ifdef CPU
#ifdef GLFWTHREAD
  glfwUnlockMutex(lock_);
#endif  // ifdef GLFWTHREAD
  return true;

}

bool Controller::SetMatrix(const string &name, int channel,
    const vector<float> &m) {
  auto it = data_.find(name);
  if (it == data_.end())
    return false;
#ifndef GLFWTHREAD
  lock_guard<mutex> sync(lock_);
#else  // ifdef GLFWTHREAD
  glfwLockMutex(lock_);
#endif  // ifdef GLFWTHREAD
  vector<Parameter> &params = get<2>(it->second);
  if (channel <= 0) {
    for (int i = 0; i < params.size(); i++)
      params[i].matrix = m;
#ifdef CPU
    g_version++;
#endif  // ifdef CPU
#ifdef GLFWTHREAD
    glfwUnlockMutex(lock_);
#endif  // GLFWTHREAD
    return true;
  } else if (channel <= params.size()) {
    params[channel - 1].matrix = m;
#ifdef CPU
    g_version++;
#endif  // ifdef CPU
#ifdef GLFWTHREAD
    glfwUnlockMutex(lock_);
#endif  // ifdef GLFWTHREAD
    return true;
  }
#ifdef GLFWTHREAD
  glfwUnlockMutex(lock_);
#endif  // ifdef GLFWTHREAD
  return false;
}

bool Controller::SetColorTune(const string &name, int channel, float hue,
    float saturation, float brightness) {

  auto it = data_.find(name);
  if (it == data_.end())
    return false;
#ifndef GLFWTHREAD
  lock_guard<mutex> sync(lock_);
#else  // ifdef GLFWTHREAD
  glfwLockMutex(lock_);
#endif  // ifdef GLFWTHREAD
  vector<Parameter> &params = get<2>(it->second);
  if (channel <= 0) {
    for (int i = 0; i < params.size(); i++)
      params[i].hsb_tune = vector<float> {
        hue, saturation, brightness
      };
#ifdef CPU
    g_version++;
#endif  // ifdef CPU
#ifdef GLFWTHREAD
    glfwUnlockMutex(lock_);
#endif  // ifdef GLFWTHREAD
    return true;
  } else if (channel <= params.size()) {
    params[channel - 1].hsb_tune = vector<float> {
      hue, saturation, brightness
    };
#ifdef CPU
    g_version++;
#endif  // ifdef CPU
#ifdef GLFWTHREAD
    glfwUnlockMutex(lock_);
#endif  // ifdef GLFWTHREAD
    return true;
  }
#ifdef GLFWTHREAD
  glfwUnlockMutex(lock_);
#endif  // ifdef GLFWTHREAD
  return false;

}

}  // namespace sc

