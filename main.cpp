#include <iostream>
#include <fstream>
#include <string>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

using namespace std;

// 0 - ERROR
// 1 - LOG
void logger(int type, string message){
  vector<string> types= {"[ERROR] ", "[LOG] "};
  std::cerr << types[type] << message << endl;
}

void waiting(){
  cout << ".\n";
  this_thread::sleep_for(chrono::milliseconds(1000));
  cout << "..\n";
  this_thread::sleep_for(chrono::milliseconds(1000));
  cout << "...\n";
  this_thread::sleep_for(chrono::milliseconds(1000));
}

int main() {
    std::ifstream file("test.lrc"); 

    if (!file.is_open()) {
      logger(0, "Cant open file!");
      return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
      size_t open_bracket = line.find('[');
      size_t close_bracket = line.find(']');

      if (open_bracket != std::string::npos && close_bracket != std::string::npos && close_bracket > open_bracket) {
        size_t time_length = close_bracket - open_bracket - 1;
        std::string time_str = line.substr(open_bracket + 1, time_length);
        std::string text_str = "";
        if (close_bracket + 2 < line.length()) { 
            text_str = line.substr(close_bracket + 2);
        }
        logger(1, time_str);
        logger(1, text_str);
        cout << "=======\n";
      } else {
        logger(0, "Wrong line format!");
      }
    }
  }

    /* Test animation
    while(true){
      waiting();
    }*/


