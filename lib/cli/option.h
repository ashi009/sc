#ifndef LIB_CLI_OPTION_H_
#define LIB_CLI_OPTION_H_

#include <sstream>
#include <string>
#include <functional>
#include <initializer_list>
#include <map>

namespace cli {

class Option {
 public:
  typedef std::function<void(std::istringstream &)> Callback;
  typedef std::pair<std::string, Callback> Entry;
  Option(std::initializer_list<Entry>);
  void Parse(int, char*[]);
 private:
  std::map<std::string, Callback> options_;
};

}  // namespace cli

#endif /* LIB_CLI_OPTION_H_ */

