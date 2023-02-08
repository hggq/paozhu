#ifndef _PROJECT_HTTP2_FLOW_H_
#define _PROJECT_HTTP2_FLOW_H_

#include <iostream>
#include <string>
#include <chrono>
#include "httppeer.h"

namespace http
{
  struct http2sendblock_t
  {
  public:
    http2sendblock_t() : fp(nullptr, &std::fclose){};

  public:
    std::string filename;
    unsigned long long offset = 0; // cur send
    unsigned long long total_size = 0;
    std::chrono::time_point<std::chrono::steady_clock> last_time;
    unsigned long long last_size = 0;
    unsigned long long pre_size = 0;
    unsigned long long pre_count = 0;
    unsigned int stream_id;

    std::unique_ptr<std::FILE, decltype(&std::fclose)> fp;
    std::shared_ptr<httppeer> peer;
    bool isfinish;
  };
}
#endif
