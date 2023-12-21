
#include <string>
#include <string_view>
#include <memory>
#include "request.h"
#include "httppeer.h"
#include "http_so_common_api.h"
namespace http
{
#ifdef ENABLE_BOOST
clientapi *clientapi::singleton = clientapi::instance();
#endif

}// namespace http