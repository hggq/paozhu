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
	namespace oauth_client{
 		 std::string page([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\">\n  <head>\n    <meta charset=\"utf-8\">\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n    <title>OAuth Client Demo</title>\n    <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  </head>\n  <body>\n    <div class=\"container py-4\">\n      <h2>OAuth 2.1 Client Demo</h2>\n      <p class=\"text-muted\">流程 client_credentials → authorize → exchange → refresh → introspect → revoke\n        全部用 C++ httpclient 串起来，默认端点指向本项目自己的 OAuth 服务器。</p>\n\n      <div class=\"card mb-3\">\n        <div class=\"card-header\">OAuth Server Endpoints</div>\n        <div class=\"card-body\">\n          <div class=\"row g-2\">\n            <div class=\"col-md-6\"><input class=\"form-control\" id=\"authorize_url\" placeholder=\"authorize URL\" value=\"";
 echo<<obj["authorize_url"].as_string(); 
 			 echo<<"\"></div>\n            <div class=\"col-md-6\"><input class=\"form-control\" id=\"token_url\"      placeholder=\"token URL\"      value=\"";
 echo<<obj["token_url"].as_string(); 
 			 echo<<"\"></div>\n            <div class=\"col-md-6\"><input class=\"form-control\" id=\"introspect_url\" placeholder=\"introspect URL\" value=\"";
 echo<<obj["introspect_url"].as_string(); 
 			 echo<<"\"></div>\n            <div class=\"col-md-6\"><input class=\"form-control\" id=\"revoke_url\"     placeholder=\"revoke URL\"     value=\"";
 echo<<obj["revoke_url"].as_string(); 
 			 echo<<"\"></div>\n          </div>\n        </div>\n      </div>\n\n      <div class=\"card mb-3\">\n        <div class=\"card-header\">Client Credentials</div>\n        <div class=\"card-body\">\n          <div class=\"row g-2\">\n            <div class=\"col-md-4\"><input class=\"form-control\" id=\"client_id\"     placeholder=\"client_id\"     value=\"";
 echo<<obj["client_id"].as_string(); 
 			 echo<<"\"></div>\n            <div class=\"col-md-4\"><input class=\"form-control\" id=\"client_secret\" placeholder=\"client_secret\" value=\"";
 echo<<obj["client_secret"].as_string(); 
 			 echo<<"\"></div>\n            <div class=\"col-md-4\"><input class=\"form-control\" id=\"scope\"          placeholder=\"scope (space)\" value=\"";
 echo<<obj["scope"].as_string(); 
 			 echo<<"\"></div>\n          </div>\n          <div class=\"mt-2\">\n            <button class=\"btn btn-primary\" onclick=\"stepCc()\">1. client_credentials</button>\n            <button class=\"btn btn-primary\" onclick=\"stepBuild()\">2. build authorize URL</button>\n            <button class=\"btn btn-primary\" onclick=\"stepExchange()\">3. exchange code</button>\n            <button class=\"btn btn-primary\" onclick=\"stepRefresh()\">4. refresh</button>\n            <button class=\"btn btn-primary\" onclick=\"stepIntrospect()\">5. introspect</button>\n            <button class=\"btn btn-primary\" onclick=\"stepRevoke()\">6. revoke</button>\n          </div>\n        </div>\n      </div>\n\n      <div class=\"card mb-3\">\n        <div class=\"card-header\">Run Output</div>\n        <div class=\"card-body\">\n          <pre id=\"out\" class=\"bg-dark text-light p-3\" style=\"max-height:400px;overflow:auto\"></pre>\n        </div>\n      </div>\n    </div>\n\n<script>\nfunction g(id){return document.getElementById(id).value;}\nasync function post(path, extra) {\n  const fd = new URLSearchParams();\n  fd.set('client_id',     g('client_id'));\n  fd.set('client_secret', g('client_secret'));\n  fd.set('authorize_url', g('authorize_url'));\n  fd.set('token_url',     g('token_url'));\n  fd.set('introspect_url',g('introspect_url'));\n  fd.set('revoke_url',    g('revoke_url'));\n  fd.set('scope',         g('scope'));\n  Object.entries(extra||{}).forEach(([k,v])=>fd.set(k,v));\n  const r = await fetch(path, {method:'POST', headers:{'Content-Type':'application/x-www-form-urlencoded'}, body:fd.toString()});\n  const j = await r.json().catch(()=>({text: await r.text()}));\n  document.getElementById('out').textContent = JSON.stringify({status:r.status, ...j}, null, 2);\n  return j;\n}\nfunction stepCc(){ post('/oauth_client/client_credentials'); }\nfunction stepBuild(){ post('/oauth_client/build_authorize',{redirect_uri:'https://example.com/callback'}); }\nfunction stepExchange(){\n  const code = prompt('Paste auth code from authorize callback:');\n  const verifier = prompt('Paste code_verifier from step 2 output:');\n  post('/oauth_client/exchange_code',{code:code, code_verifier:verifier, redirect_uri:'https://example.com/callback'});\n}\nfunction stepRefresh(){\n  const rt = prompt('Paste refresh_token:');\n  post('/oauth_client/refresh',{refresh_token:rt});\n}\nfunction stepIntrospect(){\n  const t = prompt('Paste token:');\n  const hint = prompt('token_type_hint (access_token / refresh_token / empty):','');\n  post('/oauth_client/introspect',{token:t, token_type_hint:hint});\n}\nfunction stepRevoke(){\n  const t = prompt('Paste token:');\n  const hint = prompt('token_type_hint (access_token / refresh_token / empty):','');\n  post('/oauth_client/revoke',{token:t, token_type_hint:hint});\n}\n</script>\n  </body>\n</html>\n";

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
    