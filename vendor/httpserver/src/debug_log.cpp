
#include <string>
#include <string_view>
#include <memory>
#include "debug_log.h" 
 namespace http { 
 
 debug_log* debug_log::singleton =debug_log::instance();
 debug_log::ctxmarCollector debug_log::gc;
 }