
#include <string>
#include "http2_define.h"

namespace http
{
  std::string PZ_HTTP2_H2_ALPN = "\x2h2";
  std::string PZ_HTTP2_H2 = "h2";

  std::string PZ_HTTP2_H2_16_ALPN = "\x5h2-16";
  std::string PZ_HTTP2_H2_16 = "h2-16";

  std::string PZ_HTTP2_H2_14_ALPN = "\x5h2-14";
  std::string PZ_HTTP2_H2_14 = "h2-14";

  std::string PZ_HTTP2_H1_1_ALPN = "\x8http/1.1";
  std::string PZ_HTTP2_H1_1 = "http/1.1";

  void close_file(FILE *fp)
  {
    fclose(fp);
  }
}