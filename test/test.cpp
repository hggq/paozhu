#include <string>
#include <iostream>
#include "server.h"

int main(int argc, char* argv[])
{

  try
  {
    http::httpserver httpmy;
    char *ptr=nullptr;
    if(argc>1){
      //server.conf filepath or confpath
      ptr=argv[1];
    }
    httpmy.run(ptr);
  }
  catch (std::exception &e)
  {
    std::printf("Exception: %s\n", e.what());
  }
}
