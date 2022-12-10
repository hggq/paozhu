#include<string>
#include<map>
#include<functional>
#include "request.h"
#include "viewso_param.h"
#include "viewmethold_reg.h"
namespace http
{

  VIEW_REG &get_viewmetholdreg()
  {
        static VIEW_REG instance;
        return instance;
  }

}
 
