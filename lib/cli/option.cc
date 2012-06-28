#include "option.h"

using std::string;
using std::getline;
using std::initializer_list;
using std::istringstream;

namespace cli {

Option::Option(initializer_list<Entry> vals) :
    options_(vals.begin(), vals.end()) {

}

void Option::Parse(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    istringstream iss(argv[i]);
    string option;
    getline(iss, option, '=');
    auto it = options_.find(option);
    if (it != options_.end()) {
      if (iss.peek() == '=')
        iss.get();
      it->second(iss);
    }
  }
}

}  // namespace cli
