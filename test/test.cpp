#include <string>
#include <iostream>
#include "server.h"

int main(int argc, char* argv[])
{

  try
  {
    http::httpserver httpmy;
    std::string argv_str;
    if(argc>1){
      //server.conf filepath or confpath
      argv_str.append(argv[1]);
    }
    httpmy.run(argv_str);
  }
  catch (std::exception &e)
  {
    std::printf("Exception: %s\n", e.what());
  }
}
