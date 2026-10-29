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
	namespace github{
 		 std::string list([[maybe_unused]] const struct view_param &vinfo,[[maybe_unused]] http::obj_val &obj)
			{
 
                     std::ostringstream echo;
                     try
                     {
                    
        
 			 echo<<"<!doctype html>\n<html lang=\"en\">\n  <head>\n    <meta charset=\"utf-8\">\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n    <title>GitHub bindings</title>\n    <link href=\"/assets/dist/css/bootstrap.min.css\" rel=\"stylesheet\">\n  </head>\n  <body>\n    <div class=\"container py-4\">\n      <h2>GitHub bindings</h2>\n\n      ";
 if(obj["key_ok"].to_int()==0){ 
 			 echo<<"      <div class=\"alert alert-danger\">\n        No AES master key is configured. Put a 64-hex-character <code>crypto_key</code> in\n        <code>conf/github.conf</code> before binding an account: without it the\n        <code>client_secret</code> cannot be encrypted, and this page will refuse to start a binding.\n      </div>\n      ";
 } 
 			 echo<<"\n      <div class=\"card mb-4\">\n        <div class=\"card-header\">Add a binding</div>\n        <div class=\"card-body\">\n          <form action=\"/github/bind\" method=\"post\">\n            <div class=\"mb-3 row\">\n              <label class=\"col-sm-3 col-form-label\" for=\"client_id\">OAuth App client_id</label>\n              <div class=\"col-sm-9\">\n                <input class=\"form-control\" id=\"client_id\" name=\"client_id\" maxlength=\"64\" required>\n              </div>\n            </div>\n            <div class=\"mb-3 row\">\n              <label class=\"col-sm-3 col-form-label\" for=\"client_secret\">client_secret</label>\n              <div class=\"col-sm-9\">\n                <input class=\"form-control\" id=\"client_secret\" name=\"client_secret\" type=\"password\" maxlength=\"128\" required>\n                <div class=\"form-text\">Encrypted with AES-256-GCM before it is stored, and never shown again.</div>\n              </div>\n            </div>\n            <div class=\"mb-3 row\">\n              <label class=\"col-sm-3 col-form-label\" for=\"redirect_uri\">redirect_uri</label>\n              <div class=\"col-sm-9\">\n                <input class=\"form-control\" id=\"redirect_uri\" name=\"redirect_uri\" maxlength=\"255\" required>\n                <div class=\"form-text\">\n                  Must be byte-identical to the URL registered in the OAuth App. Register this callback path:\n                  <code>/github/callback</code>. Requested scope:\n                  <code>";
 echo<<obj["scopes"].to_string(); 
 			 echo<<"</code>\n                </div>\n              </div>\n            </div>\n            <button class=\"btn btn-primary\" type=\"submit\">Authorize on GitHub</button>\n            <div class=\"form-text mt-2\">\n              Nothing is written to the database until GitHub returns a code and the token exchange succeeds.\n              Re-authorizing an existing App replaces its secret and its tokens.\n            </div>\n          </form>\n        </div>\n      </div>\n\n      <div class=\"card mb-4\">\n        <div class=\"card-header\">Bound accounts</div>\n        <div class=\"card-body\">\n          ";
 if(obj["accounts"].as_array().empty()){ 
 			 echo<<"          <p class=\"text-muted\">No accounts bound for this tenant yet.</p>\n          ";
 } else { 
 			 echo<<"          <table class=\"table table-striped align-middle\">\n            <thead>\n              <tr>\n                <th>GitHub</th>\n                <th>client_id</th>\n                <th>Token</th>\n                <th>Scope</th>\n                <th>Last sync</th>\n                <th>Next sync</th>\n                <th>Fails</th>\n                <th>repos</th>\n                <th></th>\n              </tr>\n            </thead>\n            <tbody>\n              ";
 for(auto &a:obj["accounts"].as_array()){ 
 			 echo<<"              <tr>\n                <td>\n                  ";
 if(a["gh_login"].to_string()!=""){ 
 			 echo<<"";
 echo<<a["gh_login"].to_string(); 
 			 echo<<"";
 } else { 
 			 echo<<"<span class=\"text-muted\">not fetched</span>";
 } 
 			 echo<<"                </td>\n                <td><code>";
 echo<<a["client_id"].to_string(); 
 			 echo<<"</code></td>\n                <td>\n                  ";
 unsigned int st=(unsigned int)a["token_state"].to_int(); 
 			 echo<<"                  ";
 if(st==0){ 
 			 echo<<"<span class=\"badge bg-secondary\">exchanging token</span>\n                  ";
 }else if(st==1){ 
 			 echo<<"<span class=\"badge bg-success\">usable</span>\n                  ";
 }else if(st==2){ 
 			 echo<<"<span class=\"badge bg-warning text-dark\">near expiry</span>\n                  ";
 }else if(st==3){ 
 			 echo<<"<span class=\"badge bg-danger\">revoked or expired</span>\n                  ";
 }else if(st==4){ 
 			 echo<<"<span class=\"badge bg-dark\">unbinding or failed</span>\n                  ";
 }else{ 
 			 echo<<"";
 echo<<a["token_state"].to_string(); 
 			 echo<<"";
 } 
 			 echo<<"                  ";
 if(a["access_expires_at"].to_int()==0){ 
 			 echo<<"<div class=\"form-text\">never expires</div>";
 } 
 			 echo<<"                </td>\n                <td><small>";
 echo<<a["scopes"].to_string(); 
 			 echo<<"</small></td>\n                <td>\n                  ";
 if(a["last_sync_ok"].to_int()==1){ 
 			 echo<<"<span class=\"text-success\">ok</span>";
 }else{ 
 			 echo<<"<span class=\"text-muted\">-</span>";
 } 
 			 echo<<"                  <div class=\"form-text\">";
 echo<<a["last_sync_text"].to_string(); 
 			 echo<<"</div>\n                  ";
 if(a["last_error"].to_string()!=""){ 
 			 echo<<"<div class=\"form-text text-danger\">";
 echo<<a["last_error"].to_string(); 
 			 echo<<"</div>";
 } 
 			 echo<<"                </td>\n                <td><small>";
 echo<<a["next_sync_text"].to_string(); 
 			 echo<<"</small></td>\n                <td>";
 echo<<a["sync_fails"].to_string(); 
 			 echo<<"</td>\n                <td><a class=\"btn btn-sm btn-outline-secondary\" href=\"/github/list?id=";
 echo<<a["id"].to_string(); 
 			 echo<<"\">repos</a></td>\n                <td>\n                  <button class=\"btn btn-sm btn-outline-danger\" type=\"button\"\n                          onclick=\"ghUnbind(";
 echo<<a["id"].to_string(); 
 			 echo<<", this)\">Unbind</button>\n                </td>\n              </tr>\n              ";
 } 
 			 echo<<"            </tbody>\n          </table>\n          ";
 } 
 			 echo<<"        </div>\n      </div>\n\n      ";
 if(obj["picked_id"].to_int()>0){ 
 			 echo<<"      <div class=\"card\">\n        <div class=\"card-header\">\n          Repositories of account #";
 echo<<obj["picked_id"].to_string(); 
 			 echo<<"          <a class=\"btn btn-sm btn-outline-secondary float-end\"\n             href=\"/github/list?id=";
 echo<<obj["picked_id"].to_string(); 
 			 echo<<"&show_deleted=";
 if(obj["show_deleted"].to_int()==1){ 
 			 echo<<"0";
 }else{ 
 			 echo<<"1";
 } 
 			 echo<<"\">\n            ";
 if(obj["show_deleted"].to_int()==1){ 
 			 echo<<"hide deleted";
 }else{ 
 			 echo<<"show deleted";
 } 
 			 echo<<"          </a>\n        </div>\n        <div class=\"card-body\">\n          ";
 if(obj["repos"].as_array().empty()){ 
 			 echo<<"          <p class=\"text-muted\">Nothing stored yet. The sync task writes this list; the page never calls GitHub.</p>\n          ";
 } else { 
 			 echo<<"          <table class=\"table table-sm align-middle\">\n            <thead>\n              <tr>\n                <th>Repository</th>\n                <th>Language</th>\n                <th>Stars</th>\n                <th>Pushed</th>\n                <th>State</th>\n              </tr>\n            </thead>\n            <tbody>\n              ";
 for(auto &r:obj["repos"].as_array()){ 
 			 echo<<"              <tr>\n                <td>\n                  ";
 echo<<r["full_name"].to_string(); 
 			 echo<<"                  ";
 if(r["is_private"].to_int()==1){ 
 			 echo<<"<span class=\"badge bg-secondary\">private</span>";
 } 
 			 echo<<"                  <div class=\"form-text\">";
 echo<<r["description"].to_string(); 
 			 echo<<"</div>\n                </td>\n                <td>";
 echo<<r["primary_language"].to_string(); 
 			 echo<<"</td>\n                <td>";
 echo<<r["stars"].to_string(); 
 			 echo<<"</td>\n                <td>";
 echo<<r["pushed_text"].to_string(); 
 			 echo<<"</td>\n                <td>\n                  ";
 if(r["is_deleted"].to_int()==1){ 
 			 echo<<"<span class=\"badge bg-dark\">deleted upstream</span>\n                  ";
 }else{ 
 			 echo<<"<span class=\"badge bg-success\">present</span>";
 } 
 			 echo<<"                </td>\n              </tr>\n              ";
 } 
 			 echo<<"            </tbody>\n          </table>\n          ";
 } 
 			 echo<<"        </div>\n      </div>\n      ";
 } 
 			 echo<<"    </div>\n    <script>\n      function ghUnbind(id, btn) {\n        if (!confirm(\"Revoke this token at GitHub and delete the local binding?\")) return;\n        btn.disabled = true;\n        var body = new FormData();\n        body.append(\"id\", id);\n        fetch(\"/github/unbind\", { method: \"POST\", body: body })\n          .then(function (r) { return r.json(); })\n          .then(function (j) {\n            if (j.code === 1) { window.location.reload(); return; }\n            alert(j.msg);\n            btn.disabled = false;\n          })\n          .catch(function () { alert(\"unbind request failed\"); btn.disabled = false; });\n      }\n    </script>\n  </body>\n</html>\n";

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
    