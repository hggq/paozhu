#if defined(ENABLE_VIEWSO) || defined(ENABLE_MODELSO)
#include <string>
#include <string_view>
#include <memory>
#include "request.h"
#include "httppeer.h"
#include "http_so_common_api.h"
namespace http
{
clientapi *clientapi::singleton = clientapi::instance();

}// namespace http
#endif