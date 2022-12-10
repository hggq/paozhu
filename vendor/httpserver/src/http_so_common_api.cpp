
#include <string>
#include <string_view>
#include <memory>
#include "request.h"
#include <mysqlx/xdevapi.h>
#include "httppeer.h"
#include "http_so_common_api.h" 
 namespace http { 
 
 clientapi* clientapi::singleton =clientapi::instance();

 }