#include <iostream>
#include <string>
#include "server_localvar.h"

namespace http
{
    
     server_loaclvar &get_server_global_var()
     {
        static  server_loaclvar instance;
        return instance;
     }
}

