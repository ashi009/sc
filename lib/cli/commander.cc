#include <iostream>
#include <sstream>
#include <iterator>

#include <sys/ioctl.h>

#include "commander.h"

using std::cerr;
using std::endl;
using std::string;
using std::get;
using std::initializer_list;
using std::max;
using std::istringstream;
using std::vector;
using std::istream_iterator;

namespace cli {

const string kReservedKeys = "*?!";

Commander::Commander(initializer_list<Entry> vals) :
    commands_(vals.begin(), vals.end()) {
    
}

Commander::Commander(initializer_list<DescriptiveEntry> vals) {
  for (auto &item : vals) {
    commands_[item.command] = item.callback;
    if (item.command.find_first_of(kReservedKeys) == string::npos)
      descriptions_.emplace_back(make_pair(item.command, item.descriptions));
  }
}

bool Commander::Parse(const string &command) {

  string cmd = "!";
  
  if (command.find_first_of(kReservedKeys) == string::npos) {
    auto lb = commands_.lower_bound(command);
    auto ub = lb;
    if (lb != commands_.end()) {
      int count = 0;
      if (lb->first != command) {
        string upper_command = command;
        // aba -> abb, to find out upper bound.
        upper_command.back()++;
        ub = commands_.lower_bound(upper_command);
        for (auto it = lb; it != ub; it++)
          count++;
      } else {
        count = 1;
      }
      if (count == 0) {
        cmd = "?";
      } else if (count == 1) {
        cmd = lb->first;
      } else {
        for (auto it = lb; it != ub; it++)
          cerr << "  " << it->first << endl;
        cmd = "*";
      }
    }
  }
  
  auto it = commands_.find(cmd);
  if (it != commands_.end())
    return it->second();
  return false;

}

void Commander::ShowDescriptions() {

  if (descriptions_.empty())
    return;

  winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  int line_width = w.ws_col;
  
  int padding = 0;
  for (auto it = descriptions_.begin(); it != descriptions_.end(); it++)
    padding = max(padding, (int)it->first.size());
  padding += 4;
  
  int blank_len = line_width - padding - 2;
  
  for (auto it = descriptions_.begin(); it != descriptions_.end(); it++) {
  
    cerr << string(padding - it->first.size() - 2, ' ') << it->first;
    cerr << string(2, ' ');
    
    int line_number = 0;
    
    for (string &para : it->second) {

      istringstream iss(para);
      vector<string> words((istream_iterator<string>(iss)), 
          istream_iterator<string>());
      
      for (int i = 0, j; i < words.size(); i = j + 1) {
      
        if (line_number > 0)
          cerr << string(padding, ' ');
        
        int len = blank_len - words[i].size();
        
        j = i;
        
        if (len >= 0) {
        
          for (; j + 1 < words.size() && 
            len >= 1 + words[j + 1].size(); j++)
            len -= 1 + words[j + 1].size();
            
          if (j + 1 == words.size()) {
          // last line of a paragraph
            cerr << words[i];
            for (int k = i + 1; k <= j; k++)
              cerr << string(1, ' ') << words[k];
            
          } else if (j > i) {
          // non-last line of a paragraph, needs to be justified.
          // more than one word in this line.
            int per_padding = len / (j - i) + 1;
            int remain_padding = len % (j - i);
            
            cerr << words[i];
            for (int k = i + 1; k <= j; k++)
              cerr << string(per_padding + 
                  (remain_padding-- > 0 ? 1 : 0), ' ') << words[k];
            
          } else {
          
            cerr << words[i];
            
          }
          
        
        } else {
        
          cerr << words[i].substr(0, blank_len);
          words[i] = words[i].substr(blank_len);
          
          j--;
        
        }
        
        cerr << endl;
        line_number++;
        
      }
      
    }
    
    if (line_number == 0)
      cerr << endl;
    
    cerr << endl;
    
  }

}

}  // namespace cli

