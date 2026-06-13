#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

using namespace std;

void logger(int type, string message){
  vector<string> types = {"[ERROR] ", "[LOG] "};
  std::cerr << types[type] << message << endl;
}

int main() {
    ma_engine engine;
    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        logger(0, "Failed to initialize audio engine!");
        return 1;
    }

    std::ifstream file("test.lrc"); 
    if (!file.is_open()) {
      logger(0, "Cant open file!");
      ma_engine_uninit(&engine);
      return 1;
    }

    result = ma_engine_play_sound(&engine, "test.mp3", NULL);
    if (result != MA_SUCCESS) {
        logger(0, "Failed to play test.mp3!");
        ma_engine_uninit(&engine);
        return 1;
    }

    std::string line;
    long long previous_ms = 0;

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
        
        size_t colon_pos = time_str.find(':');
        size_t dot_pos = time_str.find('.');

        if (colon_pos != std::string::npos && dot_pos != std::string::npos) {
            std::string min_str = time_str.substr(0, colon_pos);
            std::string sec_str = time_str.substr(colon_pos + 1, dot_pos - colon_pos - 1);
            std::string ms_str  = time_str.substr(dot_pos + 1);

            int minutes = std::stoi(min_str);
            int seconds = std::stoi(sec_str);
            int ms_raw  = std::stoi(ms_str);

            if (ms_str.length() == 2) {
                ms_raw *= 10;
            } 
            else if (ms_str.length() == 1) {
                ms_raw *= 100;
            }

            long long total_ms = (minutes * 60000LL) + (seconds * 1000LL) + ms_raw;
            long long wait_time = total_ms - previous_ms;
            
            if (wait_time > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
            }

            std::cout << text_str << std::endl;
            previous_ms = total_ms;
            
        } else {
            logger(0, "Wrong time format in line: " + line);
        }
      }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ma_engine_uninit(&engine);
    return 0;
}

