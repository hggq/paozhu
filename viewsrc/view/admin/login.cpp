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
//This file create by paozhu Fri, 16 May 2025 06:02:27 GMT
namespace http {

namespace view {
	namespace admin{
 		 std::string login([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\" data-bs-theme=\"auto\">\n\n<head>\n  <meta charset=\"utf-8\">\n  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n  <meta name=\"description\" content=\"\">\n  <title>登录</title>\n\n  <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  <link rel=\"stylesheet\" href=\"/assets/icons/font/bootstrap-icons.css\">\n\n  <link href=\"/css/dashboard.css\" rel=\"stylesheet\">\n<style>\nhtml,\nbody {\n  height: 100%;\n}\n.form-signin {\n  max-width: 330px;\n  padding: 1rem;\n}\n\n.form-signin .form-floating:focus-within {\n  z-index: 2;\n}\n</style>\n</head>\n\n<body class=\"d-flex align-items-center py-4 bg-body-tertiary\">\n\n  <div class=\"container-fluid\">\n<main class=\"form-signin w-100 m-auto\">\n  <form action=\"/admin/loginpost\" method=\"post\">\n    <h1 class=\"h3 mb-3 fw-normal\">Please sign in</h1>\n    <div class=\"form-floating\">\n      <input type=\"text\" class=\"form-control\" id=\"username\" name=\"username\" value=\"admin\" placeholder=\"admin or name@example.com\">\n      <label for=\"floatingInput\">Admin name</label>\n    </div>\n    <div class=\"form-floating\">\n      <input type=\"password\" class=\"form-control\" id=\"password\" name=\"password\" value=\"123456\" placeholder=\"Password\">\n      <label for=\"floatingPassword\">Password</label>\n    </div>\n\n    <div class=\"form-check text-start my-3\">\n      <input class=\"form-check-input\" type=\"checkbox\" value=\"1\" id=\"flexCheckDefault\" name=\"remberme\">\n      <label class=\"form-check-label\" for=\"flexCheckDefault\">\n        Remember me\n      </label>\n    </div>\n    <button class=\"btn btn-primary w-100 py-2\" type=\"submit\">Sign in</button>\n    <p class=\"mt-5 mb-3 text-body-secondary\">&copy; 2017–2023</p>\n    <p class=\"mt-5 mb-3 text-body-secondary\">by paozhu</p>\n  </form>\n</main>\n\n  </div>\n\n  <script src=\"/assets/dist/js/bootstrap.bundle.min.js\"></script>\n</body>\n\n</html>";

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
    