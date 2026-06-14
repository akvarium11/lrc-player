#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

using namespace std;

string argument = {};
bool byLine = 0;

void sysClear(){
  #ifdef defined(_WIN32) || defined(_WIN64)
    system("cls");
  #else 
    system("clear");
  #endif
}

void logger(int type, string message){
  vector<string> types = {"[ERROR] ", "[LOG] "};
  std::cerr << types[type] << message << endl;
}

int main(int argc, char* argv[]) {
    ma_engine engine;
    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        logger(0, "Failed to initialize audio engine!");
        return 1;
    }

    // Args
    string musicFile = "";
    string lrcFile = "";

    for (int i = 1; i < argc; ++i) {
        string argument = argv[i];

        if (argument == "--clear" || argument == "-c") {
            byLine = true;
        } 
        else if (argument == "--music" || argument == "-m") {
            if (i + 1 < argc) {
                musicFile = argv[i + 1];
                i++;
            } else {
                logger(0, "Missing file after --music flag!");
                return 1;
            }
        }
        else if (argument == "--lrc" || argument == "-l") {
            if (i + 1 < argc) {
                lrcFile = argv[i + 1];
                i++;
            } else {
                logger(0, "Error: Missing file after --lrc flag!");
                return 1;
            }
          }
      }
    if(musicFile == "") musicFile = "test.mp3";
    if(lrcFile == "") lrcFile = "test.lrc";

    std::ifstream file(lrcFile); 
    if (!file.is_open()) {
      logger(0, "Cant open file!");
      ma_engine_uninit(&engine);
      return 1;
    }

    result = ma_engine_play_sound(&engine, musicFile.c_str(), NULL);
    if (result != MA_SUCCESS) {
        logger(0, "Failed to play music, make sure its .mp3 file!");
        ma_engine_uninit(&engine);
        return 1;
    }

    sysClear();

    std::string line;
    long long previous_ms = 0;
    std::string current_text = "";
    bool has_cached_line = false;

    while (true) {
        if (!has_cached_line && !std::getline(file, line)) {
            break;
        }
        has_cached_line = false;

        size_t open_bracket = line.find('[');
        size_t close_bracket = line.find(']');

        if (open_bracket != std::string::npos && close_bracket != std::string::npos && close_bracket > open_bracket) {
            size_t time_length = close_bracket - open_bracket - 1;
            std::string time_str = line.substr(open_bracket + 1, time_length);
            current_text = "";
            
            if (close_bracket + 2 < line.length()) { 
                current_text = line.substr(close_bracket + 2);
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

                long long next_total_ms = total_ms;
                std::string next_line;
                
                while (std::getline(file, next_line)) {
                    size_t next_open = next_line.find('[');
                    size_t next_close = next_line.find(']');
                    if (next_open != std::string::npos && next_close != std::string::npos && next_close > next_open) {
                        size_t next_time_len = next_close - next_open - 1;
                        std::string next_time_str = next_line.substr(next_open + 1, next_time_len);
                        size_t next_colon = next_time_str.find(':');
                        size_t next_dot = next_time_str.find('.');
                        
                        if (next_colon != std::string::npos && next_dot != std::string::npos) {
                            std::string n_min = next_time_str.substr(0, next_colon);
                            std::string n_sec = next_time_str.substr(next_colon + 1, next_dot - next_colon - 1);
                            std::string n_ms  = next_time_str.substr(next_dot + 1);
                            
                            int nm = std::stoi(n_min);
                            int ns = std::stoi(n_sec);
                            int nms = std::stoi(n_ms);
                            if (n_ms.length() == 2) nms *= 10;
                            else if (n_ms.length() == 1) nms *= 100;
                            
                            next_total_ms = (nm * 60000LL) + (ns * 1000LL) + nms;
                            line = next_line;
                            has_cached_line = true;
                            break;
                        }
                    }
                }

                long long duration = next_total_ms - total_ms;
                if (!has_cached_line) {
                    duration = 3000;
                }
                if (!current_text.empty() && duration > 0) {
                    long long char_delay = duration / current_text.length();
                    if(byLine == 1) sysClear();
                    for (char c : current_text) {
                        std::cout << c << std::flush;
                        std::this_thread::sleep_for(std::chrono::milliseconds(char_delay));
                    }
                } else {
                    std::cout << current_text;
                    if (duration > 0) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
                    }
                }
                std::cout << std::endl;

                previous_ms = next_total_ms;
                
            } else {
                logger(0, "Wrong time format in line: " + line);
            }
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ma_engine_uninit(&engine);
    return 0;
}

