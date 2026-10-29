#include<iostream>
#include <cstdio>
#include <string>
#include <sstream>
#include <map> 
#include <vector>
#include <ctime>
#include <array>
#include <sys/stat.h>
#include <cstdlib>
#include "request.h"
#include "datetime.h"
#include "cookie.h"
#include "urlcode.h"
#include "loadviewso.h"
#include "viewso_param.h"
#include "http_so_common_api.h"
#include "viewsrc.h"
//This file create by paozhu
namespace http {

namespace view {
	namespace oauth{
 		 std::string consent([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\">\n  <head>\n    <meta charset=\"utf-8\">\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n    <title>Authorize access</title>\n    <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  </head>\n  <body class=\"text-center\">\n    <div class=\"container text-center\">\n      <div class=\"row\">\n        <div class=\"col-3\"></div>\n        <div class=\"col-6\">\n          <h2 id=\"oauth-consent-title\">Authorize ";
 echo<<obj["client_name"].as_string(); 
 			 echo<<"</h2>\n          <form action=\"";
 echo<<obj["submit"].as_string(); 
 			 echo<<"\" method=\"post\">\n            <input type=\"hidden\" name=\"response_type\" value=\"code\">\n            <input type=\"hidden\" name=\"csrf_token\" value=\"";
 echo<<obj["csrf_token"].as_string(); 
 			 echo<<"\">\n            <input type=\"hidden\" name=\"client_id\" value=\"";
 echo<<obj["client_id"].as_string(); 
 			 echo<<"\">\n            <input type=\"hidden\" name=\"redirect_uri\" value=\"";
 echo<<obj["redirect_uri"].as_string(); 
 			 echo<<"\">\n            <input type=\"hidden\" name=\"scope\" value=\"";
 echo<<obj["scope"].as_string(); 
 			 echo<<"\">\n            <input type=\"hidden\" name=\"state\" value=\"";
 echo<<obj["state"].as_string(); 
 			 echo<<"\">\n            <input type=\"hidden\" name=\"code_challenge\" value=\"";
 echo<<obj["code_challenge"].as_string(); 
 			 echo<<"\">\n            <input type=\"hidden\" name=\"code_challenge_method\" value=\"";
 echo<<obj["code_challenge_method"].as_string(); 
 			 echo<<"\">\n            <div class=\"row mb-3\">\n              <label class=\"col-sm-4 col-form-label\">Application</label>\n              <div class=\"col-sm-8 text-start\">\n                <span class=\"form-text\">";
 echo<<obj["client_name"].as_string(); 
 			 echo<<"</span>\n              </div>\n            </div>\n            <div class=\"row mb-3\">\n              <label class=\"col-sm-4 col-form-label\">Callback</label>\n              <div class=\"col-sm-8 text-start\">\n                <span class=\"form-text\">";
 echo<<obj["redirect_uri"].as_string(); 
 			 echo<<"</span>\n              </div>\n            </div>\n            <div class=\"row mb-3\">\n              <label class=\"col-sm-4 col-form-label\">Requested scope</label>\n              <div class=\"col-sm-8 text-start\">\n                <span class=\"form-text\">";
 echo<<obj["scope"].as_string(); 
 			 echo<<"</span>\n              </div>\n            </div>\n            <button type=\"submit\" class=\"btn btn-primary\">Allow access</button>\n          </form>\n          <p class=\"text-muted mt-3\">\n            To refuse, close this page. No code is issued and the callback address receives nothing.\n          </p>\n        </div>\n      </div>\n    </div>\n  </body>\n</html>\n";

                    }
                    catch(const char *e)
                    {
                        echo << e;
                        return echo.str();
                    }
                  return echo.str();
             }

         }
        }
       }
    