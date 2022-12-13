#ifndef __HTTP2_FRAME_H
#define __HTTP2_FRAME_H

#include <string>
#include <map>
#include <vector>
#include <memory>
#include "http_header.h"

namespace http
{
   typedef struct
   {
      std::string key;
      std::string value;
   } http2_header_static_table_t;

#define PER_DATA_BLOCK_SIZE 10240

#define HTTP2_SETTINGS_HEADER_TABLE_SIZE 0x01
#define HTTP2_SETTINGS_ENABLE_PUSH 0x02
#define HTTP2_SETTINGS_MAX_CONCURRENT_STREAM 0x03
#define HTTP2_SETTINGS_INITIAL_WINDOW_SIZE 0x04
#define HTTP2_SETTINGS_MAX_FRAME_SIZE 0x05
#define HTTP2_SETTINGS_MAX_HEADER_LIST_SIZE 0x06

#define HTTP2_HEADER_END_STREAM 0x01
#define HTTP2_HEADER_END_HEADERS 0x04
#define HTTP2_HEADER_PADDED 0x08
#define HTTP2_HEADER_PRIORITY 0x20

#define HTTP2_DATA_END_STREAM 0x01
#define HTTP2_DATA_PADDED 0x08

   //

   struct http2_setting_t
   {

      unsigned int header_table_size = 4096;     // 0x01
      unsigned int enable_push = 0;              // 0x02
      unsigned int max_concurrent_streams = 128; // 0x03
      unsigned int initial_window_size = 65535;  // 0x04
      unsigned int max_frame_size = 16384;       // 0x05
      unsigned int max_heaer_list_size = 0;      // 0x06
      unsigned int accept_cache_digest = 0;      // 0x07
      unsigned int enable_connect_protocol = 0;  // 0x08
   };

   struct http2_priority_t
   {

      unsigned int depend_stream_id;
      unsigned char weight;
   };

   struct http2_header_t
   {
      unsigned int block_length;
      unsigned int stream_id;
      unsigned int depend_stream_id;
      unsigned char flag;
      unsigned char stream_e;
      unsigned char stream_d;
      unsigned char pad_length;
      unsigned char weight;

      std::vector<unsigned char> data;
   };

   // struct http2_window_update_t{
   //      std::map<unsigned int,unsigned char> data;
   // };

   union
   {
      struct
      {
         unsigned char END_STREAM : 1;
         unsigned char e1 : 1;
         unsigned char END_HEADERS : 1;
         unsigned char PADDED : 1;
         unsigned char e2 : 1;
         unsigned char PRIORITY : 1;

      } data;
      char value;
   } header_flags;

   struct http2_goaway_t
   {

      unsigned int last_stream_id;
      unsigned int error_code;
      std::vector<unsigned char> data;
   };

   struct http2_data_t
   {
      http2_data_t():uprawfile(nullptr,&std::fclose){};
      unsigned int stream_id = 0;
      unsigned int match_offset = 0;
      bool isbegin = false;
      bool isend = false;
      bool endstream = false;
      bool endheader = false;
      bool padded = false;
      bool priority = false;
      unsigned char posttype = 0;
      unsigned char postfieldtype = 0;
      unsigned char changetype = 0;
      unsigned char pad_length = 0;
      unsigned char weight = 0;
      unsigned long long length = 0;         // bloack data length;
      unsigned long long curnum = 0;         // now block length
      unsigned long long content_length = 0; // post length

      //std::FILE *uprawfile = NULL;
      std::unique_ptr<std::FILE, decltype(&std::fclose)> uprawfile;

      std::string boundary;
      std::string fieldname;
      std::string buffer_key;
      std::string buffer_value;
      std::string field_value;

      struct uploadfile_t upfile;
   };

   // header code
   extern std::map<std::string, unsigned char> http2_header_codes_table;

#define HTTP2_CODE_authority 1
#define HTTP2_CODE_GET 2
#define HTTP2_CODE_POST 3
#define HTTP2_CODE_path 4
#define HTTP2_CODE_index 5
#define HTTP2_CODE_http 6
#define HTTP2_CODE_https 7
#define HTTP2_CODE_200 8
#define HTTP2_CODE_204 9
#define HTTP2_CODE_206 10
#define HTTP2_CODE_304 11
#define HTTP2_CODE_400 12
#define HTTP2_CODE_404 13
#define HTTP2_CODE_500 14
#define HTTP2_CODE_accept_charset 15
#define HTTP2_CODE_accept_encoding 16
#define HTTP2_CODE_accept_language 17
#define HTTP2_CODE_accept_ranges 18
#define HTTP2_CODE_accept 19
#define HTTP2_CODE_access_control_allow_origin 20
#define HTTP2_CODE_age 21
#define HTTP2_CODE_allow 22
#define HTTP2_CODE_authorization 23
#define HTTP2_CODE_cache_control 24
#define HTTP2_CODE_content_disposition 25
#define HTTP2_CODE_content_encoding 26
#define HTTP2_CODE_content_language 27
#define HTTP2_CODE_content_length 28
#define HTTP2_CODE_content_location 29
#define HTTP2_CODE_content_range 30
#define HTTP2_CODE_content_type 31
#define HTTP2_CODE_cookie 32
#define HTTP2_CODE_date 33
#define HTTP2_CODE_etag 34
#define HTTP2_CODE_expect 35
#define HTTP2_CODE_expires 36
#define HTTP2_CODE_from 37
#define HTTP2_CODE_host 38
#define HTTP2_CODE_if_match 39
#define HTTP2_CODE_if_modified_since 40
#define HTTP2_CODE_if_none_match 41
#define HTTP2_CODE_if_range 42
#define HTTP2_CODE_if_unmodified_since 43
#define HTTP2_CODE_last_modified 44
#define HTTP2_CODE_link 45
#define HTTP2_CODE_location 46
#define HTTP2_CODE_max_forwards 47
#define HTTP2_CODE_proxy_authenticate 48
#define HTTP2_CODE_proxy_authorization 49
#define HTTP2_CODE_range 50
#define HTTP2_CODE_referer 51
#define HTTP2_CODE_refresh 52
#define HTTP2_CODE_retry_after 53
#define HTTP2_CODE_server 54
#define HTTP2_CODE_set_cookie 55
#define HTTP2_CODE_strict_transport_security 56
#define HTTP2_CODE_transfer_encoding 57
#define HTTP2_CODE_user_agent 58
#define HTTP2_CODE_vary 59
#define HTTP2_CODE_via 60
#define HTTP2_CODE_www_authenticate 61

   static http2_header_static_table_t http2_header_static_table[] = {
       {":empty", ""},
       {":authority", ""},
       {":method", "GET"},
       {":method", "POST"},
       {":path", "/"},
       {":path", "/index.html"},
       {":scheme", "http"},
       {":scheme", "https"},
       {":status", "200"},
       {":status", "204"},
       {":status", "206"},
       {":status", "304"},
       {":status", "400"},
       {":status", "404"},
       {":status", "500"},
       {"accept-charset", ""},
       {"accept-encoding", "gzip, deflate"},
       {"accept-language", ""},
       {"accept-ranges", ""},
       {"accept", ""},
       {"access-control-allow-origin", ""},
       {"age", ""},
       {"allow", ""},
       {"authorization", ""},
       {"cache-control", ""},
       {"content-disposition", ""},
       {"content-encoding", ""},
       {"content-language", ""},
       {"content-length", ""},
       {"content-location", ""},
       {"content-range", ""},
       {"content-type", ""},
       {"cookie", ""},
       {"date", ""},
       {"etag", ""},
       {"expect", ""},
       {"expires", ""},
       {"from", ""},
       {"host", ""},
       {"if-match", ""},
       {"if-modified-since", ""},
       {"if-none-match", ""},
       {"if-range", ""},
       {"if-unmodified-since", ""},
       {"last-modified", ""},
       {"link", ""},
       {"location", ""},
       {"max-forwards", ""},
       {"proxy-authenticate", ""},
       {"proxy-authorization", ""},
       {"range", ""},
       {"referer", ""},
       {"refresh", ""},
       {"retry-after", ""},
       {"server", ""},
       {"set-cookie", ""},
       {"strict-transport-security", ""},
       {"transfer-encoding", ""},
       {"user-agent", ""},
       {"vary", ""},
       {"via", ""},
       {"www-authenticate", ""},
   };
   // http2 header make item
   bool make_http2_headers(std::string &hh_data);
   bool set_http2_headers_static(unsigned char *hh_data, unsigned char hh_code);
   bool make_http2_headers_static(std::string &hh_data, unsigned int hh_code);
   

   bool make_http2_headers_item(std::string &hh_data, unsigned int num);
   bool make_http2_headers_item(std::string &hh_data, const std::string &value);
   bool make_http2_headers_item(std::string &hh_data, unsigned char, const std::string &value);

   bool make_http2_headers_item2(std::string &hh_data, unsigned char, const std::string &value);
   bool make_http2_headers_item2(std::string &hh_data, const std::string &key, const std::string &value);
   bool make_http2_headers_item2(std::string &hh_data, unsigned char, unsigned long long num);

   bool make_http2_headers_item3(std::string &hh_data, const std::string &key, const std::string &value);
   bool make_http2_headers_item3(std::string &hh_data, unsigned char, const std::string &value);
   bool make_http2_headers_item3(std::string &hh_data, unsigned char, unsigned long long num);

   bool make_http2_headers_item4(std::string &hh_data, const std::string &key, const std::string &value);
   bool make_http2_headers_item4(std::string &hh_data, unsigned char, const std::string &value);
   bool make_http2_headers_item4(std::string &hh_data, unsigned char, unsigned long long num);

   bool set_http2_frame_streamid(std::string &hh_data, unsigned int streamid);
   bool set_http2_headers_size(std::string &hh_data, unsigned int sizenum);
   bool set_http2_headers_flag(std::string &hh_data, unsigned char flag);
}
#endif