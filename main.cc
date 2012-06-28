#include <cstdio>
#include <iostream>
#include <functional>
#include <sstream>
#include <vector>
#include <chrono>
#ifndef GLFWTHREAD
#include <thread>
#else
#include "gl.h"
#endif
#include "cli.h"
#include "vc.h"
#include "appinfo.h"
#include "controller.h"

using namespace std;
using namespace cv;
using namespace cli;
using namespace vc;
using namespace sc;

const string kInvalidMessage = "Invalid input.";
const string kAmbiguousMessage = "Your input is ambiguous.";
const string kFailureMessage = "Failed.";
const string kUnknownMessage = "Type help for usage info.";
const string kQuitMessage = "Ended.";

bool IsValidId(const string &id) {

  if (id.empty()) return false;
  if (!isalpha(id[0])) return false;
  for (int i = 1; i < id.size(); i++)
    if (!isalnum(id[i])) return false;
  return true;

}

bool IsPrefixOf(const string &prefix, const string &str) {

  if (prefix.empty() || prefix.size() > str.size()) return false;
  return str.substr(0, prefix.size()) == prefix;

}

bool ParseId(istream &iss, string &id) {

  return (iss >> id) && IsValidId(id);

}

bool ParseIdChannel(istream &iss, string &id, int &channel) {

  if (iss >> id) {
    auto pos = id.find(':');
    if (pos != string::npos) {
      istringstream(id.substr(pos+1)) >> channel;
      id = id.substr(0, pos);
    } else {
      channel = 0;
    }
    return IsValidId(id) && channel >= 0;
  } else {
    return false;
  }

}

bool ParseMatrix(istream &iss, vector<float> &matrix) {

  for (int i = 0; i < 16; i++)
    if (!(iss >> matrix[i])) return false;
  return true;

}

#ifdef GLFWTHREAD

function<void()> g_payload;
void PayloadProxy(void *arg) {
  g_payload();
}

#endif

int main(int argc, char *argv[]) {

  string line, command, id;
  int channel;
  vector<float> matrix(16);

  bool is_running = true, is_spawn = false;
  int width = 640, height = 480;
  
  Option option({
    {
      "--spawn", [&](istringstream &iss) {
        is_spawn = true;
      }
    }, {
      "--size", [&](istringstream &iss) {
        char ch;
        if (!(iss >> width >> ch >> height) || width < 1 || height < 1) {
          width = 640;
          height = 480;
        }
      }
    }
  });
  option.Parse(argc, argv);

  Controller controller(width, height);
  timing::Timer render_timer(&controller, false);

  Commander cameramode_commander({
    {
      "perspective", [&](istringstream &iss) {
        controller.mode(kPerspective);
        return true;
      }
    }, {
      "orthographic", [&](istringstream &iss) {
        controller.mode(kOrthographic);
        return true;
      }
    }
  });

  Commander commander({
    {
      "?", {}, [&](istringstream &iss) {
        cerr << kUnknownMessage << endl;
        return true;
      }
    }, {
      "*", {}, [&](istringstream &iss) {
        cerr << kAmbiguousMessage << endl;
        return true;
      }
    }, {
      "!", {}, [&](istringstream &iss) {
        return false;
      }
    }, {
      "help", {
        "Show this message."
      }, [&](istringstream &iss) {
        commander.ShowDescriptions();
        return true;
      }
    }, {
      "quit", {
        "Quit."
      }, [&](istringstream &iss) {
        is_running = false;
        return true;
      }
    }, {
      "reset", {
        "Reset scence. Close all opened streams."
      }, [&](istringstream &iss) {
        controller.Reset();
        return true;
      }
    }, {
      "open", {
        "{id} {camera:{device id}:{number of channels} | {uri}}",
        "Open a stream and bind it to {id}. {id} should start with letter and "
        "containing only letters and numbers.",
        "examples:",
        "open live camera:0:1",
        "open glee /media/glee.mp4"
      }, [&](istringstream &iss) {
        if (!ParseId(iss, id))
          return false;
        string source;
        char ch;
        if (iss >> ch) {
          iss.putback(ch);
          getline(iss, source);
          if (controller.AddSource(id, source)) {
            auto source = controller.GetSource(id);
            cout << source->width() << 'x' << source->height() << endl;
          } else {
            cout << kFailureMessage << endl;
          }
          return true;
        }
        return false;
      }
    }, {
      "close", {
        "{id}",
        "Close a stream with given id."
      }, [&](istringstream &iss) {
        if (!ParseId(iss, id))
          return false;
        if (!controller.RemoveSource(id))
          cout << kFailureMessage << endl;
        return true;
      }
    }, {
      "delay", {
        "{id} {duration}",
        "Delay a stream for 0 to 2000 milliseconds.",
        "example:",
        "delay live 2000"
      }, [&](istringstream &iss) {
        if (!ParseId(iss, id))
          return false;
        int duration;
        iss >> duration;
        if (duration < 0 || duration > 2000)
          return false;
        controller.DelaySource(id, duration);
        return true;
      }
#ifndef CPU
    }, {
      "tunecolor", {
        "{id[:channel]} {hue} {saturation} {brightness}",
        "Tunning a stream's or channel's color in HSB color space. {hue} "
        "should ranged from -180 to 180, {saturation} and {brightness} should "
        "ranged from -1 to 1.",
        "If only {id} was provide, setting would apply to that stream's all "
        "channels. For file sources, {channel} can always be ignored.",
        "example:",
        "tunecolor live:1 0 0 0.2"
      }, [&](istringstream &iss) {
        if (!ParseIdChannel(iss, id, channel))
          return false;
        float hue, saturation, brightness;
        iss >> hue >> saturation >> brightness;
        if (hue < -180 || hue > 180 ||
            saturation < -1 || saturation > 1 ||
            brightness < -1 || brightness > 1)
          return false;
        if (!controller.SetColorTune(id, channel, hue, saturation, brightness))
          cout << kFailureMessage << endl;
        return true;
      }
#endif  // ifndef CPU
    }, {
      "matrix", {
        "{id[:channel]} {matrix}",
        "Set module transform matrix for a stream or channel.",
        "{matrix} is a 4x4 homographic transform matrix, and given in "
        "column-major order. The orginal image was mapped to (-1, -1) to (1, 1).",
        "example:",
        "matrix glee 320 0 0 0 0 240 0 0 0 0 1 0 0 0 0 1"
      }, [&](istringstream &iss) {
        if (!ParseIdChannel(iss, id, channel) || !ParseMatrix(iss, matrix))
          return false;
        if (!controller.SetMatrix(id, channel, matrix))
          cout << kFailureMessage << endl;
        return true;
      }
    }, {
      "worldmatrix", {
        "{matrix}",
        "Set world transform matrix for camera.",
        "This matrix used to move the center of world to the desired point, "
        "and angle for camera to see.",
        "The final projection matrix comes from T = P * C * W, where W is the "
        "world transform matrix, C is the camera transform matrix, and P is "
        "the post transform matrix.",
        "Default to 4x4 identity matrix."
      }, [&](istringstream &iss) {
        if (!ParseMatrix(iss, matrix))
          return false;
        controller.matrix_world(matrix);
        return true;
      }
    }, {
      "postmatrix", {
        "{matrix}",
        "Set post transform matrix for camera.",
        "This matrix used to transform the image camera sees, mainly used for "
        "zooming.",
        "Default to 4x4 identity matrix."
      }, [&](istringstream &iss) {
        if (!ParseMatrix(iss, matrix))
          return false;
        controller.matrix_post(matrix);
        return true;
      }
    }, {
      "cameramode", {
        "{perspective | orthographic}",
        "Set camara projection mode.",
        "For 3D scence use perspective, 2D use orthographic."
      }, [&](istringstream &iss) {
        string mode_name;
        if (iss >> mode_name)
          return cameramode_commander.Parse(mode_name);
        return false;
      }
    }, {
      "bgcolor", {
        "{#xxxxxx | {red} {green} {blue}}",
        "Set background color. You may use either hex format or decimal format"
        "{red}, {green} and {blue} should between 0 and 1.",
        "example:",
        "bgcolor #cccccc"
      }, [&](istringstream &iss) {
        char ch;
        if ((iss >> ch) && ch == '#') {
          int color;
          if (iss >> hex >> color) {
            controller.background_color(vector<float> {
              ((color >> 16) & 255) / 255.f,
              ((color >> 8) & 255) / 255.f,
              (color & 255) / 255.f
            });
            return true;
          }
        } else {
          iss.putback(ch);
          float red, green, blue;
          if (iss >> red >> green >> blue) {
            if (red < 0 || red > 1 || green < 0 || green > 1 || blue < 0 ||
                blue > 1)
              return false;
            controller.background_color(vector<float> { red, green, blue });
            return true;
          }
        }
        return false;
      }
    }, {
      "snapshot", {
        "{path}",
        "Make a snapshot for current streams. Snapshots will be saved to "
        "specified folder, and named as {path}/{id}_{channel}.jpg."
      }, [&](istringstream &iss) {
        char ch;
        if (!(iss >> ch))
          return false;
        iss.putback(ch);
        string path;
        getline(iss, path);
        if (path.back() != '/')
          path += '/';
        bool err = false;
        auto data = controller.data();
        for (auto it = data.begin(); it != data.end(); it++) {
          ostringstream oss;
          oss << path << it->first << '_';
          auto base_pos = oss.tellp();
          auto feeds = get<0>(it->second)->feeds();
          for (int i = 0; i < feeds.size(); i++) {
            Mat image = feeds[i]->Snapshot();
            oss.seekp(base_pos);
            oss << i+1;
            cout << oss.str() << endl;
            if (!imwrite(oss.str() + ".jpg", image, vector<int> {
              CV_IMWRITE_JPEG_QUALITY, 60
            })) err = true;
            Mat thumb_image;
            resize(image, thumb_image, {74, 74 * image.rows / image.cols});
            if (!imwrite(oss.str() + "_thumb.jpg", thumb_image, vector<int> {
              CV_IMWRITE_JPEG_QUALITY, 80
            })) err = true;
          }
        }
        if (err)
          cout << kFailureMessage << endl;
        return true;
      }
    }, {
      "profile", {
        "Display current profile."
      }, [&](istringstream &iss) {
        // print enviornment params.
        cout << "cameramode " << (controller.mode() == kPerspective ?
            "perspective" : "orthographic") << endl;
        cout << "worldmatrix";
        auto matrix_world = controller.matrix_world();
        for (int i = 0; i < 16; i++)
          cout << ' ' << matrix_world[i];
        cout << endl;
        cout << "postmatrix";
        auto matrix_post = controller.matrix_post();
        for (int i = 0; i < 16; i++)
          cout << ' ' << matrix_post[i];
        cout << endl;
        cout << "bgcolor";
        auto background_color = controller.background_color();
        for (int i = 0; i < 3; i++)
          cout << ' ' << background_color[i];
        cout << endl;
        // print each sources
        auto data = controller.data();
        for (auto it = data.begin(); it != data.end(); it++) {
          auto source = get<0>(it->second);
          cout << "open " << it->first << ' ' << source->source_uri() << endl;
          cout << "delay " << it->first << ' ' << source->delay() << endl;
          auto &parms = get<2>(it->second);
          // print each channels
          for (int i = 0; i < parms.size(); i++) {
            cout << "matrix " << it->first << ':' << i+1;
            for (int j = 0; j < 16; j++)
              cout << ' ' << parms[i].matrix[j];
            cout << endl;
            cout << "tunecolor " << it->first << ':' << i+1;
            for (int j = 0; j < 3; j++)
              cout << ' ' << parms[i].hsb_tune[j];
            cout << endl;
          }
        }
        return true;
      }
    }, {
      "perfinfo", {
        "Display current performance information."
      }, [&](istringstream &iss) {
        cout << render_timer.tps() << ' ' << render_timer.time_skew_avg() <<
            ' ' << render_timer.tick_skew_avg() << endl;
        auto data = controller.data();
        for (auto it = data.begin(); it != data.end(); it++) {
          auto source = get<0>(it->second);
          auto timer = get<1>(it->second);
          cout << it->first << ' ' << source->fps() << ' ' << timer->tps() <<
              ' ' << timer->time_skew_avg() << ' ' << timer->tick_skew_avg() <<
              endl;
        }
        return true;
      }
    }, {
      "sleep", {
        "{duration}",
        "Sleep for specified duration of time."
      }, [&](istringstream &iss) {
        double duration;
        iss >> duration;
        if (duration < 0)
          return false;
#ifndef GLFWTHREAD
        this_thread::sleep_for(chrono::milliseconds((int)duration));
#else  // ifdef GLFWTHREAD
        glfwSleep(duration / 1000.0);
#endif  // ifdef GLFWTHREAD
        return true;
      }
    }
  });

  cerr << APP_TITLE << endl;

#ifndef GLFWTHREAD
  thread io_thread([&]() {
#else  // ifdef GLFWTHREAD
  g_payload = [&]() {
#endif  // ifdef GLFWTHREAD

    while (is_running && cin.good()) {

      cerr << "(sc) ";
      getline(cin, line);
      if (!commander.Parse(line))
        cerr << kInvalidMessage << endl;

      // if application is working in spawn mode, output a spiliter (u001f) 
      // between messages.
      if (is_spawn) {
        cout << static_cast<char>(31);
        cerr << static_cast<char>(31);
      }

    }
    
    render_timer.Stop();

#ifndef GLFWTHREAD
  });
#else  // ifdef GLFWTHREAD
  };
  GLFWthread io_thread = glfwCreateThread(PayloadProxy, nullptr);
#endif  // ifdef GLFWTHREAD

  // render at 30fps
  render_timer.Start(30);

#ifndef GLFWTHREAD
  io_thread.join();
#else  // ifdef GLFWTHREAD
  glfwWaitThread(io_thread, GLFW_WAIT);
  glfwDestroyThread(io_thread);
#endif  // ifdef GLFWTHREAD

  cout << kQuitMessage << endl;

}

