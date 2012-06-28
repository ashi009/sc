#ifndef LIB_CLI_COMMANDER_H_
#define LIB_CLI_COMMANDER_H_

#include <sstream>
#include <string>
#include <functional>
#include <initializer_list>
#include <map>
#include <vector>

namespace cli {

class Commander {
 public:
  typedef std::function<bool(std::istringstream&)> Callback;
  struct DescriptiveEntry {
    std::string command;
    std::vector<std::string> descriptions;
    Callback callback;
  };
  typedef std::pair<std::string, Callback> Entry;
  
  Commander(std::initializer_list<Entry>);
  Commander(std::initializer_list<DescriptiveEntry>);
  
  bool Parse(const std::string&);
  void ShowDescriptions();

 private:
  std::map<std::string, Callback> commands_;
  std::vector<std::pair<std::string, std::vector<std::string>>> descriptions_;
  
};

}

#endif /* LIB_CLI_COMMANDER_H_ */

