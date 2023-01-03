#include <zlib.h>
#include "version.h"
#include "terminal_color.h"
#include "sendqueue.h"
#include "http_socket.h"
#include "client_session.h"
#include "http2_parse.h"
#include "http_mime.h"
#include "http2_define.h"
#include "http_domain.h"
#include "server.h"
#include "http2_huffman.h"
#include "http_parse.h"
#include "serverconfig.h"
#include "httppeer.h"
#include "directory_fun.h"
#include "https_brotli.h"
#include "gzip.h"
#include "httphook.h"
#include "func.h"
#include "mysqlconfig.h"
#include "mysqlpool.h"
#include "reghttpmethod.hpp"
#include "reghttpmethod_pre.hpp"
#include "regviewmethod.hpp"
#include "loadviewso.h"
#include "loadmodule.h"
#include "mysqlproxyfun.h"
#include "http_so_common_api.h"
#include "server_localvar.h"
#include "debug_log.h"
#include "websockets_callback.h"
#include "websockets_method_reg.hpp"

namespace fs = std::filesystem;
#if defined(ASIO_ENABLE_HANDLER_TRACKING)
#define use_awaitable \
  asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

namespace http
{

  bool httpserver::http1_send_file(unsigned int streamid, std::shared_ptr<httppeer> peer, std::shared_ptr<client_session> peer_session, const std::string &filename)
  {

    FILE_AUTO fp(std::fopen(peer->sendfilename.c_str(), "rb"), &std::fclose);
    if (fp.get())
    {
      fseek(fp.get(), 0, SEEK_END);
      long long file_size = ftell(fp.get());
      fseek(fp.get(), 0, SEEK_SET);
      std::string htmlcontent;
      std::string etag;

      etag = make_header_etag(file_size, peer->fileinfo.st_mtime + peer->url.size());
      std::string fileexttype;
      std::string mime_value = "text/html; charset=utf-8";

      unsigned int filebasesize = peer->sendfilename.size();
      unsigned int filenameoffset = 0;

      if (filebasesize > 0)
      {
        for (filenameoffset = filebasesize - 1; filenameoffset > 0; filenameoffset--)
        {
          if (peer->sendfilename[filenameoffset] == '.')
          {
            break;
          }
        }
        filenameoffset += 1;
        for (; filenameoffset < filebasesize; filenameoffset++)
        {
          fileexttype.push_back(peer->sendfilename[filenameoffset]);
        }
      }

      if (peer->etag == etag)
      {
        peer->status(304);
        peer->length(0);
        peer->setHeader("date", getGmtTime());
        peer->setHeader("last-modified", getGmtTime((unsigned long long)peer->fileinfo.st_mtime));
        peer->setHeader("etag", etag);
        peer->type(mime_value);
        etag = peer->make_http1_header();
        etag.append("\r\n");
        peer_session->send_data(etag);
        return true;
      }

      peer->compress = 0;
      if (file_size < 16877216 && fileexttype.size() > 0 && mime_compress.contains(fileexttype))
      {

        if (peer->state.gzip || peer->state.br)
        {
          htmlcontent.resize(file_size);
          file_size = fread(&htmlcontent[0], 1, file_size, fp.get());
          htmlcontent.resize(file_size);

          std::string tempcompress;
          if (peer->state.gzip)
          {

            if (compress(htmlcontent.data(), htmlcontent.size(), tempcompress, Z_DEFAULT_COMPRESSION) == Z_OK)
            {
              htmlcontent = tempcompress;
              peer->compress = 1;
            }
          }
          else if (peer->state.br)
          {
            brotli_encode(htmlcontent, tempcompress);
            peer->compress = 2;
            htmlcontent = tempcompress;
          }
          file_size = htmlcontent.size();
        }
      }

      if (fileexttype.size() > 0)
      {
        mime_value = mime_map[fileexttype];
        if (mime_value.empty())
        {
          mime_value = "text/html; charset=utf-8";
        }
      }
      else
      {
        if (file_size > 20480)
        {
          mime_value = "application/octet-stream";
        }
        else
        {
          mime_value = "text/plain";
        }
      }
      peer->status(200);
      peer->length(file_size);
      peer->type(mime_value);
      if (peer->compress == 0)
      {
        peer->setHeader("accept-ranges", "bytes");
      }

      peer->setHeader("date", getGmtTime());
      peer->setHeader("last-modified", getGmtTime((unsigned long long)peer->fileinfo.st_mtime));

      peer->setHeader("etag", etag);

      etag = peer->make_http1_header();
      etag.append("\r\n");

      if (peer->compress > 0 && htmlcontent.size() == 0)
      {
        peer_session->send_data(etag);
        return true;
      }
      if (peer->compress > 0 && htmlcontent.size() > 0)
      {
        etag.append(&htmlcontent[0], htmlcontent.size());
        peer_session->send_data(etag);
      }
      else
      {
        peer_session->send_data(etag);
        fseek(fp.get(), 0, SEEK_SET);
        try
        {
          long long readnum = 0;
          while (readnum < file_size)
          {
            htmlcontent.resize(4096);
            unsigned int nread = fread(&htmlcontent[0], 1, 4096, fp.get());
            htmlcontent.resize(nread);
            peer_session->send_data(htmlcontent);
            readnum += nread;
          }
        }
        catch (std::exception &e)
        {
        }
      }
    }
    else
    {
      http1_send_bad_server(500, peer, peer_session);
    }
    return true;
  }
  bool httpserver::http1_send_file_range(unsigned int streamid, std::shared_ptr<httppeer> peer, std::shared_ptr<client_session> peer_session, const std::string &filename)
  {
    FILE_AUTO fp(std::fopen(peer->sendfilename.c_str(), "rb"), &std::fclose);
    if (fp.get())
    {
      fseek(fp.get(), 0, SEEK_END);
      long long file_size = ftell(fp.get());
      fseek(fp.get(), 0, SEEK_SET);
      std::string htmlcontent;
      std::string etag;

      etag = make_header_etag(file_size, peer->fileinfo.st_mtime + peer->url.size());
      long long readnum = 0, mustnum = 0;

      std::string fileexttype;
      std::string mime_value = "text/html; charset=utf-8";

      unsigned int filebasesize = peer->sendfilename.size();
      unsigned int filenameoffset = 0;

      if (filebasesize > 0)
      {
        for (filenameoffset = filebasesize - 1; filenameoffset > 0; filenameoffset--)
        {
          if (peer->sendfilename[filenameoffset] == '.')
          {
            break;
          }
        }
        filenameoffset += 1;
        for (; filenameoffset < filebasesize; filenameoffset++)
        {
          fileexttype.push_back(peer->sendfilename[filenameoffset]);
        }
      }

      unsigned int statecode = 200;
      peer->compress = 0;

      if (fileexttype.size() > 0)
      {
        mime_value = mime_map[fileexttype];
        if (mime_value.empty())
        {
          mime_value = "text/html; charset=utf-8";
        }
      }
      else
      {
        if (file_size > 20480)
        {
          mime_value = "application/octet-stream";
        }
        else
        {
          mime_value = "text/plain";
        }
      }
      mustnum = file_size;
      if (peer->state.rangebegin > 0)
      {
        readnum = peer->state.rangebegin;
        statecode = 206;
      }
      if (peer->state.rangeend > 0)
      {
        mustnum = peer->state.rangeend + 1;
        statecode = 206;
      }

      peer->status(statecode);
      peer->length(mustnum - readnum);
      peer->type(mime_value);

      if (statecode == 206)
      {
        peer->setHeader("content-range", "bytes " + std::to_string(readnum) + "-" + std::to_string(mustnum - 1) + "/" + std::to_string(file_size));
      }
      else
      {
        peer->setHeader("accept-ranges", "bytes");
      }

      peer->setHeader("date", getGmtTime());
      peer->setHeader("last-modified", getGmtTime((unsigned long long)peer->fileinfo.st_mtime));
      peer->setHeader("etag", etag);
      htmlcontent = peer->make_http1_header();
      htmlcontent.append("\r\n");

      peer_session->send_data(htmlcontent);
      fseek(fp.get(), readnum, SEEK_SET);
      try
      {

        while (readnum < mustnum)
        {
          htmlcontent.resize(4096);
          unsigned int nread = fread(&htmlcontent[0], 1, 4096, fp.get());
          htmlcontent.resize(nread);
          peer_session->send_data(htmlcontent);

          readnum += nread;
        }
      }
      catch (std::exception &e)
      {
      }
    }
    else
    {
      http1_send_bad_server(500, peer, peer_session);
    }
    return true;
  }
  bool httpserver::http1_send_body(unsigned int streamid, std::shared_ptr<httppeer> peer, std::shared_ptr<client_session> peer_session, const unsigned char *buffer, unsigned int begin_end)
  {

    return true;
  }
  void httpserver::http1loop(unsigned int stream_id, std::shared_ptr<httppeer> peer, std::shared_ptr<client_session> peer_session)
  {
    serverconfig &sysconfigpath = getserversysconfig();
    peer->sitepath = sysconfigpath.getsitepath(peer->host);
    unsigned char sendtype = peer->getfileinfo();
    DEBUG_LOG("http1loop:%s %d", peer->sendfilename.c_str(), sendtype);
    if (sendtype == 1)
    {
      if (peer->state.rangebytes)
      {
        http1_send_file_range(stream_id, peer, peer_session, peer->sendfilename);
      }
      else
      {
        http1_send_file(stream_id, peer, peer_session, peer->sendfilename);
      }
    }
    else if (sendtype == 2 && peer->isshow_directory())
    {
      std::string htmlcontent = displaydirectory(peer->sendfilename, peer->urlpath, peer->get["sort"].as_string(), sysconfigpath.configpath);
      std::string str = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nConnection: Keep-Alive\r\nContent-Length: ";
      str.append(std::to_string(htmlcontent.size()));
      str.append("\r\n\r\n");
      str.append(htmlcontent);
      peer_session->send_data(str);
    }
    else
    {
      DEBUG_LOG("---  http1 pool pre --------");
      peer->linktype = 0;

      if (peer->cookie.check(COOKIE_SESSION_NAME))
      {
        peer->parse_session();
      }
      std::promise<int> p;
      std::future<int> f = p.get_future();
      peer->loopresults.emplace_back(std::move(f));
      peer->looprunpromise = std::move(p);
      peer->status(200);
      peer->content_type.clear();
      peer->etag.clear();
      try
      {
        clientrunpool.addclient(peer);
        
        peer->loopresults.front().get();
        peer->loopresults.pop_front();
      }
      catch (const std::exception &e)
      {
        peer->status(500);
      }

      if (peer->getStatus() < 100)
      {
        peer->status(200);
      }
      if (!peer->issettype())
      {
        peer->type("text/html; charset=utf-8");
      }
      if (peer->state.gzip)
      {
        if (strcasecmp(peer->content_type.c_str(), "text/html; charset=utf-8") == 0 || strcasecmp(peer->content_type.c_str(), "application/json") == 0 || strcasecmp(peer->content_type.c_str(), "text/html") == 0 || strcasecmp(peer->content_type.c_str(), "application/json; charset=utf-8") == 0)
        {
          if (peer->output.size() > 100)
          {
            std::string tempcompress;
            if (compress(peer->output.data(), peer->output.size(), tempcompress, Z_DEFAULT_COMPRESSION) == Z_OK)
            {
              peer->output = tempcompress;
              peer->compress = 1;
            }
          }
        }
      }
      peer->length(peer->output.size());
      std::string htmlcontent = peer->make_http1_header();
      htmlcontent.append("\r\n");
      htmlcontent.append(&peer->output[0], peer->output.size());
      peer_session->send_data(htmlcontent);
    }
  }
  void httpserver::http1_send_bad_server(unsigned int error_code, std::shared_ptr<httppeer> peer, std::shared_ptr<client_session> peer_session)
  {
    std::string str = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\nContent-Length: ";
    std::string stfilecom = "<h3>500 Internal Server Error</h3>";
    stfilecom.append("<hr /><p>File: " + peer->urlpath + " Access is denied!</p>");
    str.append(std::to_string(stfilecom.size()));
    str.append("\r\n\r\n");
    str.append(stfilecom);
    peer_session->send_data(str);
  }
  void httpserver::http1_send_bad_request(unsigned int error_code, std::shared_ptr<client_session> peer_session)
  {
    std::string stfilecom = "<h3>400 Bad Request</h3>";
    stfilecom.append("<hr /><p>Error Code: " + std::to_string(error_code) + "</p>");
    std::string str = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\nContent-Length: ";
    str.append(std::to_string(stfilecom.size()));
    str.append("\r\n\r\n");
    str.append(stfilecom);
    peer_session->send_data(str);
  }
  int httpserver::checkhttp2(std::shared_ptr<client_session> peer_session)
  {

    if (peer_session->_cache_data[0] == 0x50 && peer_session->_cache_data[1] == 0x52 && peer_session->_cache_data[2] == 0x49 && peer_session->_cache_data[3] == 0x20 && peer_session->_cache_data[4] == 0x2A && peer_session->_cache_data[5] == 0x20)
    {
      peer_session->httpv = 2;
      return 4;
    }
    else
    {
      peer_session->httpv = 1;
      return 1;
    }
  }
  asio::awaitable<void> httpserver::clientpeerfun(struct httpsocket_t sock_temp, bool isssl, bool httpversion)
  {
    try
    {
      std::shared_ptr<client_session> peer_session = isssl ? std::make_shared<client_session>(std::move(sock_temp._sslsocket)) : std::make_shared<client_session>(std::move(sock_temp._socket));
      std::string log_item;
      try
      {
        //serverconfig &sysconfigpath = getserversysconfig();
        std::shared_ptr<httppeer> peer = std::make_shared<httppeer>();

        peer->client_ip = peer_session->getremoteip();
        if (check_blockip(peer->client_ip))
        {
          total_count--;
          co_return;
        }

#ifdef DEBUG
        {
          DEBUG_LOG("new client in");
          std::ostringstream oss;
          oss << std::this_thread::get_id();
          std::string tempthread = oss.str();
          DEBUG_LOG("\033[31mclient thread:%s\033[0m", tempthread.c_str());
        }
#endif
        int readnum, error_state = 0;
        unsigned linktype = 0;
        //bool isbegin = false;

        std::unique_ptr<http2parse> http2pre;
        std::unique_ptr<httpparse> http1pre;

        peer->isssl = isssl ? true : false;
        peer->socket_session = peer_session;
        unsigned int offsetnum = 0;
        for (;;)
        {
          memset(peer_session->_cache_data, 0x00, 4096);
          try
          {
            if (isssl)
            {
              readnum = co_await peer_session->_sslsocket.front().async_read_some(asio::buffer(peer_session->_cache_data, 4096), asio::use_awaitable);
            }
            else
            {
              readnum = co_await peer_session->_socket.front().async_read_some(asio::buffer(peer_session->_cache_data, 4096), asio::use_awaitable);
            }
          }
          catch (const std::exception &e)
          {
            log_item.clear();
            log_item.append("co_await exception: ");
            log_item.append(e.what());
            log_item.push_back(0x20);
            log_item.append(peer->client_ip);
            log_item.push_back(0x20);
            log_item.append(peer->host);
            log_item.push_back(0x20);
            log_item.append(peer->url);
            std::unique_lock<std::mutex> lock(log_mutex);
            error_loglist.emplace_back(log_item);
            lock.unlock();
            break;
          }

          if (linktype == 0)
          {
            linktype = checkhttp2(peer_session);
            if (linktype == 1)
            {
              http1pre = std::make_unique<httpparse>();
              http1pre->setpeer(peer);
            }
            else if (linktype == 4)
            {
              http2pre = std::make_unique<http2parse>();
            }
          }

          // http1
          if (linktype == 1)
          {
            if (!http1pre)
            {
              break;
            }
            http1pre->process(peer_session->_cache_data, readnum);
            if (readnum == 0 || http1pre->getfinish())
            {

              //  LOG_OUT << "http1parse fininsh" << peer->url << LOG_END;
#ifdef DEBUG
              DEBUG_LOG("http1parse begin");
              DEBUG_LOG("urlpath:%s", peer->urlpath.c_str());
              DEBUG_LOG("host:%s", peer->host.c_str());
              DEBUG_LOG("user-agent:%s", peer->header["user-agent"].c_str());
              DEBUG_LOG("http1parse end");
#endif
              peer->isssl = isssl ? true : false;
              peer->socket_session = peer_session;
              peer->server_ip = peer_session->getlocalip();
              peer->client_ip = peer_session->getremoteip();
              peer->client_port = peer_session->getremoteport();
              peer->server_port = peer_session->getlocalport();

              {
                log_item.clear();
                log_item.append(peer->server_ip);
                log_item.push_back(0x20);
                log_item.append(date("%Y-%m-%d %X"));
                log_item.push_back(0x20);
                log_item.append(std::to_string(peer->server_port));
                log_item.push_back(0x20);
                log_item.append(peer->host);
                log_item.push_back(0x20);
                log_item.append(peer->url);
                std::unique_lock<std::mutex> lock(log_mutex);
                access_loglist.emplace_back(log_item);
                lock.unlock();
              }

              if (peer->state.h2c)
              {
                linktype = 4;

                peer_session->httpv = 2;
                peer->httpv = 2;
                peer->isfinish = true;
                peer->issend = false;

                peer_session->send_switch101();

                peer->stream_id = 1;

                peer->isssl = isssl ? true : false;
                http2pre = std::make_unique<http2parse>();
                http2pre->setsession(peer_session);
                http2pre->http_data[1] = peer;
                http2pre->stream_list.emplace(1);

                continue;
              }

              if (hook_host_http1(peer))
              {
                break;
              }

              if (peer->state.websocket)
              {
                linktype = 5;
                peer->linktype = 3;
                peer->ws = std::make_unique<websocketparse>();
                peer->ws->setWebsocketkey(peer->websocket.key);
                std::string resp = peer->ws->respondHandshake();
                peer_session->send_data(resp);
                WEBSOCKET_REG &wsreg = get_websocket_config();

                if (peer->pathinfos.size() == 0)
                {
                  break;
                }
                auto wsiter = wsreg.find(peer->pathinfos[0]);
                if (wsiter == wsreg.end())
                {
                  break;
                }
                auto myclientwsplugin = wsreg[peer->pathinfos[0]];
                peer->websockets = myclientwsplugin(peer);
                peer->ws->isopen = true;
                peer->websockets->onopen();
                if (peer->websockets->timeloop_num > 0)
                {
                  websockettasks.emplace_back(peer);
                }
                peer->ws->contentlength = 0;
                continue;
              }
              http1loop(1, peer, peer_session);

              // LOG_OUT << "http1loop end" << LOG_END;
              DEBUG_LOG("http1loop end");
              if (peer->state.keeplive == false)
              {
                DEBUG_LOG("--- keeplive false --------");
                break;
              }
              http1pre->clear();
            }
            if (http1pre->error > 0)
            {
              http1_send_bad_request(http1pre->error, peer_session);
              break;
            }
          }

          // websocket
          if (linktype == 5)
          {
            if (peer->ws->contentlength == 0)
            {
              DEBUG_LOG("websockets big data");
              peer->ws->getprocssdata(peer_session->_cache_data, readnum);
            }
            else
            {
              DEBUG_LOG("websockets small data");
              peer->ws->parsedata(peer_session->_cache_data, readnum);
            }

            if (peer->ws->isfinish)
            {
              DEBUG_LOG("onmessage isfinish");
              if (peer->ws->opcode < 0x08 && peer->ws->opcode > 0x00)
              {
                peer->ws->closefile();
                clientrunpool.addclient(peer);
              }
              else if (peer->ws->opcode == 0x08)
              {
                peer->ws->isopen = false;
                peer->websockets->onclose();
                DEBUG_LOG("websockets");
                break;
              }
              else if (peer->ws->opcode == 0x09)
              {
                std::string outhello;
                outhello = peer->ws->makePong();
                peer->send(outhello);
              }
            }
          }

          // h2c
          if (linktype == 4)
          {
            int j = 0;
            for (int i = 0; i < 24; i++, j++)
            {
              if (peer_session->_cache_data[j] != magicstr[i])
              {
                break;
              }
            }
            if (j != 24)
            {
              error_state = 1;
            }
            offsetnum = 24;
            if (error_state > 0)
            {
              break;
            }
            linktype = 3;
            peer_session->send_setting();
            if (!http2pre)
            {
              http2pre = std::make_unique<http2parse>();
            }
            http2pre->setsession(peer_session);
          }

          if (linktype == 3)
          {
            if (!http2pre)
            {
              break;
            }
            http2pre->process(&peer_session->_cache_data[offsetnum], readnum - offsetnum);
            offsetnum = 0;
            if (http2pre->stream_list.size() > 0)
            {
              for (; http2pre->stream_list.size() > 0;)
              {
                unsigned int block_steamid = http2pre->stream_list.front();
                http2pre->http_data[block_steamid]->socket_session = peer_session;
                if (hook_host_http2(http2pre->http_data[block_steamid]))
                {
                  http2pre->stream_list.pop();
                  error_state = 1;
                  break;
                }
                http2pre->http_data[block_steamid]->linktype = 1;
                http2pre->http_data[block_steamid]->server_ip = peer_session->getlocalip();
                http2pre->http_data[block_steamid]->client_ip = peer_session->getremoteip();
                http2pre->http_data[block_steamid]->client_port = peer_session->getremoteport();
                http2pre->http_data[block_steamid]->server_port = peer_session->getlocalport();

                {
                  log_item.clear();
                  log_item.append(http2pre->http_data[block_steamid]->client_ip);
                  log_item.push_back(0x20);
                  log_item.append(date("%Y-%m-%d %X"));
                  log_item.push_back(0x20);
                  log_item.append(std::to_string(http2pre->http_data[block_steamid]->server_port));
                  log_item.push_back(0x20);
                  log_item.append(http2pre->http_data[block_steamid]->host);
                  log_item.push_back(0x20);
                  log_item.append(http2pre->http_data[block_steamid]->url);
                  std::unique_lock<std::mutex> lock(log_mutex);
                  access_loglist.emplace_back(log_item);
                  lock.unlock();
                }
#ifdef DEBUG
                DEBUG_LOG("http2parse begin");
                DEBUG_LOG("urlpath:%s", http2pre->http_data[block_steamid]->urlpath.c_str());
                DEBUG_LOG("host:%s", http2pre->http_data[block_steamid]->host.c_str());
                DEBUG_LOG("user-agent:%s", http2pre->http_data[block_steamid]->header["user-agent"].c_str());
                DEBUG_LOG("http2parse end");
#endif

                clientrunpool.addclient(http2pre->http_data[block_steamid]);
                http2pre->stream_list.pop();
              }
            }
            if (error_state > 0)
            {
              peer_session->send_goway();
              peer_session->isclose = true;
              break;
            }
            if (peer_session->isgoway)
            {
              peer_session->isclose = true;
              break;
            }
          }
        }
      }
      catch (std::exception &e)
      {
        log_item.clear();
        log_item.append(" client thread exception  ");
        log_item.append(e.what());
        std::unique_lock<std::mutex> lock(log_mutex);
        error_loglist.emplace_back(log_item);
        lock.unlock();
      }
      peer_session->isclose = true;
      total_count--;
      DEBUG_LOG("client run exit:%d", total_count.load());
    }
    catch (const std::exception &e)
    {
      DEBUG_LOG("client exit exception");
    }

    co_return;
  }

  void httpserver::websocket_loop(int myid)
  {

    using namespace std::chrono;
    using dsec = duration<double>;
    auto invFpsLimit = duration_cast<system_clock::duration>(dsec{1. / 0.5});
    auto m_BeginFrame = system_clock::now();
    auto m_EndFrame = m_BeginFrame + invFpsLimit;
    unsigned frame_count_per_second = 0;
    auto prev_time_in_seconds = time_point_cast<seconds>(m_BeginFrame);
    unsigned int fps = 0;
    for (;;)
    {
      auto time_in_seconds = time_point_cast<seconds>(system_clock::now());
      ++frame_count_per_second;
      if (time_in_seconds > prev_time_in_seconds)
      {

        frame_count_per_second = 0;
        prev_time_in_seconds = time_in_seconds;

        ++fps;

        for (auto iter = websockettasks.begin(); iter != websockettasks.end();)
        {
          std::shared_ptr<httppeer> peer = iter->lock();
          try
          {

            if (peer)
            {
              if (peer->websockets->timeloop_num > 0 && fps % peer->websockets->timeloop_num == 0)
              {
                peer->websockets->pushloop();
              }

              if (peer->websockets->timeloop_num == 0)
              {
                peer.reset();
                websockettasks.erase(iter++);
              }
              else
              {
                ++iter;
              }
            }
            else
            {
              websockettasks.erase(iter++);
            }
          }
          catch (std::exception &e)
          {
            websockettasks.erase(iter++);
          }
        }
        if (fps > 31536000)
        {
          fps = 1;
        }
      }
      std::this_thread::sleep_until(m_EndFrame);
      m_BeginFrame = m_EndFrame;
      m_EndFrame = m_BeginFrame + invFpsLimit;
    }
  }
  asio::awaitable<void> httpserver::sslhandshake(asio::ip::tcp::socket socket,asio::ssl::context& context_, unsigned long long temp_domain )
  {
    try
    {

      serverconfig &sysconfigpath = getserversysconfig();
      const unsigned char *for_next_proto = nullptr;
      unsigned int next_proto_len = 0;
      bool httpversion = false;
      asio::error_code ec_error;
       DEBUG_LOG(" accept ok!");
        // if all http2
        if (sysconfigpath.isallnothttp2)
        {
          SSL_CTX_set_alpn_select_cb(context_.native_handle(), alpn_cb, (void *)temp_domain);
        }
        asio::ssl::stream<asio::ip::tcp::socket> sslsocket(std::move(socket), context_);
        sslsocket.lowest_layer().set_option(asio::ip::tcp::no_delay(true));
        sslsocket.handshake(asio::ssl::stream_base::server, ec_error);
        if (ec_error)
        {
          std::unique_lock<std::mutex> lock(log_mutex);
          error_loglist.emplace_back(" handshake ec_error ");
          lock.unlock();
          DEBUG_LOG(" handshake ec_error !");
          co_return;
        }
        // client select proto 看看客户端是否指定 协议，如果没有指定为null
        SSL_get0_next_proto_negotiated(sslsocket.native_handle(), &for_next_proto, &next_proto_len);
        if (for_next_proto == nullptr)
        {
          // openssl >2.0,5 必须大于2.0.5 alpn选择的协议
          SSL_get0_alpn_selected(sslsocket.native_handle(), &for_next_proto, &next_proto_len);
        }
        if (next_proto_len > 0)
        {
          if (for_next_proto[0] == 'h' && for_next_proto[1] == '2')
          {
            DEBUG_LOG(" h2 ");
            httpversion = true;
          }
        }
        DEBUG_LOG(" https ok!");
        total_count++;

        struct httpsocket_t sock_temp(std::move(sslsocket));
        co_spawn(this->io_context, clientpeerfun(std::move(sock_temp), true, httpversion), asio::detached);
    }
    catch (std::exception &e)
    {

    }
  }
  void httpserver::listeners()
  {
    serverconfig &sysconfigpath = getserversysconfig();
    unsigned short portnum = sysconfigpath.get_ssl_port();

    asio::error_code ec_error;
    asio::ip::tcp::acceptor acceptor(this->io_context);
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), portnum);
    acceptor.open(endpoint.protocol());

    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));

#if (defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)) && !defined(__CYGWIN__)

    typedef asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT> reuse_port;
    acceptor.set_option(reuse_port(true));

#endif

    acceptor.bind(endpoint, ec_error);
    acceptor.listen(asio::socket_base::max_connections, ec_error);
    if (ec_error)
    {

      std::unique_lock<std::mutex> lock(log_mutex);
      error_loglist.emplace_back(" acceptor listen https error ");
      lock.unlock();
      exit(1);
    }

    asio::ssl::context context_(asio::ssl::context::sslv23);
    context_.set_options(
        asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 | asio::ssl::context::single_dh_use);
    context_.set_password_callback(std::bind(get_password));
    context_.use_certificate_chain_file(sysconfigpath.ssl_chain_file());

    SSL_CTX_set_mode(context_.native_handle(), SSL_MODE_AUTO_RETRY);
    context_.use_private_key_file(sysconfigpath.ssl_key_file(), asio::ssl::context::pem);
    context_.use_tmp_dh_file(sysconfigpath.ssl_dh_file());
    SSL_CTX_set_tlsext_servername_callback(context_.native_handle(), serverNameCallback);

    auto ssl_opts = (SSL_OP_ALL & ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS) |
                    SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION |
                    SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION |
                    SSL_OP_SINGLE_ECDH_USE | SSL_OP_NO_TICKET |
                    SSL_OP_CIPHER_SERVER_PREFERENCE;
    SSL_CTX_set_options(context_.native_handle(), ssl_opts);
    SSL_CTX_set_mode(context_.native_handle(), SSL_MODE_AUTO_RETRY);
    SSL_CTX_set_mode(context_.native_handle(), SSL_MODE_RELEASE_BUFFERS);

    constexpr char DEFAULT_CIPHER_LIST[] =
        "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-"
        "AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-"
        "POLY1305:ECDHE-RSA-CHACHA20-POLY1305:DHE-RSA-AES128-GCM-SHA256:DHE-RSA-"
        "AES256-GCM-SHA384";

    if (SSL_CTX_set_cipher_list(context_.native_handle(), DEFAULT_CIPHER_LIST) == 0)
    {
      std::cerr << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
    }

    std::vector<unsigned char> next_proto;
    next_proto = get_default_alpn();

    unsigned int protos_len;
    const char *protos;

    protos = HTTP2_H2_ALPN HTTP1_NPN;
    protos_len = sizeof(HTTP2_H2_ALPN HTTP1_NPN) - 1;

    if (SSL_CTX_set_alpn_protos(context_.native_handle(), (const unsigned char *)protos, protos_len) < 0)
    {
    }

    SSL_CTX_set_next_protos_advertised_cb(context_.native_handle(), next_proto_cb, &next_proto);
    unsigned long long temp_domain = 0;
    sysconfigpath.domain_http2[temp_domain] = false;
    server_loaclvar &localvar = get_server_global_var();
    if (sysconfigpath.isallnothttp2)
    {
      temp_domain = std::hash<const char *>{}((const char *)&sysconfigpath.map_value["default"]["mainhost"]);
      // SSL_CTX_set_alpn_select_cb(context_.native_handle(), alpn_cb, &alpn_ctx);
      if (localvar.http2_enable)
      {
        sysconfigpath.domain_http2[temp_domain] = true;
      }
      else
      {
        sysconfigpath.domain_http2[temp_domain] = false;
        if (sysconfigpath.map_value.find(sysconfigpath.map_value["default"]["mainhost"]) != sysconfigpath.map_value.end())
        {
          if (sysconfigpath.map_value[sysconfigpath.map_value["default"]["mainhost"]].find("http2_enable") != sysconfigpath.map_value[sysconfigpath.map_value["default"]["mainhost"]].end())
          {

            if (sysconfigpath.map_value[sysconfigpath.map_value["default"]["mainhost"]]["http2_enable"][0] == '1')
            {
              sysconfigpath.domain_http2[temp_domain] = true;
            }
          }
        }
      }
    }
    SSL_CTX_set_alpn_select_cb(context_.native_handle(), alpn_cb, (void *)temp_domain);

    //const unsigned char *for_next_proto = nullptr;
    //unsigned int next_proto_len = 0;
    //bool httpversion = false;
    for (;;)
    {
      try
      {
        //httpversion = false;
        asio::ip::tcp::socket socket(this->io_context);
        DEBUG_LOG("https accept");

        acceptor.accept(socket, ec_error);
        if (ec_error)
        {

          std::unique_lock<std::mutex> lock(log_mutex);
          error_loglist.emplace_back(" accept ec_error ");
          lock.unlock();
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
          continue;
        }
        DEBUG_LOG(" accept ok!");

        co_spawn(this->io_context, sslhandshake(std::move(socket), std::ref(context_), temp_domain), asio::detached);

      }
      catch (std::exception &e)
      {
      }
    }
  }
  void httpserver::listener()
  {
    serverconfig &sysconfigpath = getserversysconfig();
    asio::error_code ec;

    unsigned short portnum = sysconfigpath.get_port();

    asio::ip::tcp::acceptor acceptor(this->io_context);

    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), portnum);
    acceptor.open(endpoint.protocol());

    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));

#if (defined(unix) || defined(__unix) || defined(__unix__) || \
     defined(__APPLE__)) &&                                   \
    !defined(__CYGWIN__)

    typedef asio::detail::socket_option::boolean<SOL_SOCKET, SO_REUSEPORT>
        reuse_port;
    acceptor.set_option(reuse_port(true));

#endif

    acceptor.bind(endpoint, ec);
    acceptor.listen(asio::socket_base::max_connections, ec);
    if (ec)
    {
      std::unique_lock<std::mutex> lock(log_mutex);
      error_loglist.emplace_back("  acceptor listen http error  ");
      lock.unlock();
      exit(1);
    }

    for (;;)
    {
      try
      {
        asio::ip::tcp::socket socket(this->io_context);
        acceptor.accept(socket, ec);
        total_count++;
        struct httpsocket_t sock_temp(std::move(socket));
        co_spawn(this->io_context, clientpeerfun(std::move(sock_temp), false, false), asio::detached);
      }
      catch (std::exception &e)
      {
        std::unique_lock<std::mutex> lock(log_mutex);
        error_loglist.emplace_back(" http accept error  ");
        lock.unlock();
      }
    }
  }

  void httpserver::add_runsocketthread()
  {
    runthreads.emplace_back([self = this]()
                            {

            std::unique_lock<std::mutex> lock(self->log_mutex);
          self->error_loglist.emplace_back(" add socket thread ");
          lock.unlock();

            self->io_context.run(); });
  }
  void httpserver::httpwatch()
  {
    serverconfig &sysconfigpath = getserversysconfig();
    mysqlconfig_init_link();
    struct regmethold_t temp;
    temp.pre = nullptr;
    temp.regfun = [self = this](std::shared_ptr<httppeer> peer) -> std::string
    {
                    httppeer &client=peer->getpeer();
                    client<<"<h3 align=\"center\" title=\"Live ";
                    client<<self->total_count.load();
                    client<<"\"><span style=\"font-size:2em\">🧨 Paozhu</h3> <p align=\"center\">Version ";
                    client<<(PAOZHU_VERSION/100000);
                    client<<".";
                    client<<(PAOZHU_VERSION/100%1000);
                    client<<".";
                    client<<(PAOZHU_VERSION%100);
                    client<<"</p>"; 
       return ""; };
    _http_regmethod_table.emplace("paozhu_version", std::move(temp));

    clientapi *pn = clientapi::instance();

    pn->api_loadview = loadviewso;
    pn->api_loadcontrol = loadcontrol;

    pn->api_mysqlselect = get_mysqlselectexecute;
    pn->api_mysqledit = get_mysqlselectexecute;
    pn->api_mysqlcommit = get_mysqlselectexecute;
    pn->map_value = sysconfigpath.map_value;
    pn->server_global_var = get_server_global_var;
    pn->api_mysql_back_conn=back_mysql_connect;
    int catch_num = 0;
    unsigned int updatetimetemp = 0;
    std::string currentpath;
    std::string error_path;
    server_loaclvar &static_server_var = get_server_global_var();
    currentpath = static_server_var.log_path;

    if (currentpath.size() > 0 && currentpath.back() != '/')
    {
      currentpath.push_back('/');
    }
    error_path=currentpath;
    currentpath.append("access.log");
    error_path.append("error.log");
    struct flock lockstr = {};
    unsigned int mysqlpool_time=1;
    std::size_t n_write=0;
    for (;;)
    {
      if (catch_num > 10)
      {
        std::this_thread::sleep_for(std::chrono::seconds(20));
        catch_num = 0;
      }
      try
      {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        if (clientrunpool.getlivenum() > (clientrunpool.getmixthreads() - 3))
        {
          clientrunpool.addthread(3);
          updatetimetemp = 0;
        }
        else if (clientrunpool.getpoolthreadnum() > clientrunpool.getmixthreads())
        {
          updatetimetemp += 1;
          if (updatetimetemp == 5)
          {
            clientrunpool.fixthread();
            updatetimetemp = 0;
          }
        }
        DEBUG_LOG("pool thread:%d", clientrunpool.getpoolthreadnum());
#ifdef DEBUG
        clientrunpool.printthreads();
#endif

        // save access.log
        if (!access_loglist.empty())
        {
          int fd = open(currentpath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
          if (fd == -1)
          {
            continue;
          }

          lockstr.l_type = F_WRLCK;
          lockstr.l_whence = SEEK_END;
          lockstr.l_start = 0;
          lockstr.l_len = 0;

          lockstr.l_pid = 0;

          if (fcntl(fd, F_SETLK, &lockstr) == -1)
          {
            close(fd);
            continue;
          }
          std::unique_lock<std::mutex> loglock(log_mutex);
          while (!access_loglist.empty())
          {
            n_write=write(fd, access_loglist.front().data(), access_loglist.front().size());
            access_loglist.pop_front();
          }
          loglock.unlock();

          lockstr.l_type = F_UNLCK;
          if (fcntl(fd, F_SETLK, &lockstr) == -1)
          {
            close(fd);
            continue;
          }
          close(fd);
        }

         // save error.log
        if (!error_loglist.empty())
        {
          int fd = open(error_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
          if (fd == -1)
          {
            continue;
          }

          lockstr.l_type = F_WRLCK;
          lockstr.l_whence = SEEK_END;
          lockstr.l_start = 0;
          lockstr.l_len = 0;

          lockstr.l_pid = 0;

          if (fcntl(fd, F_SETLK, &lockstr) == -1)
          {
            close(fd);
            continue;
          }
          std::unique_lock<std::mutex> loglock(log_mutex);
          while (!error_loglist.empty())
          {
            n_write=write(fd, error_loglist.front().data(), error_loglist.front().size());
            error_loglist.pop_front();
          }
          loglock.unlock();

          lockstr.l_type = F_UNLCK;
          if (fcntl(fd, F_SETLK, &lockstr) == -1)
          {
            close(fd);
            continue;
          }
          close(fd);
        }
        if(n_write>0)
        {
          n_write=0;
        }

        if(mysqlpool_time%100==0)
        {
            if(total_count.load()==0)
            {
                std::map<size_t, mysqllinkpool> &mysqldbpoolglobal = get_mysqlpool();
                for(auto iter=mysqldbpoolglobal.begin();iter != mysqldbpoolglobal.end();iter++)
                {
                    iter->second.clearpool();

                    DEBUG_LOG("mysql pool clearpoool ");
                }
                mysqlpool_time=1;
                 
            }
        }

        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);

       if(now->tm_hour<3&&mysqlpool_time>82800)
       {
            std::map<size_t, mysqllinkpool> &mysqldbpoolglobal = get_mysqlpool();
            for(auto iter=mysqldbpoolglobal.begin();iter != mysqldbpoolglobal.end();iter++)
            {
                iter->second.clearpool();
            }
            mysqlpool_time=1;
       } 
        
        mysqlpool_time+=1;
        DEBUG_LOG("clear mysql poll time:%d,client live:%d",mysqlpool_time,total_count.load());
        
      }
      catch (std::exception &e)
      {
        DEBUG_LOG("frame thread:%s", e.what());
        catch_num += 1;
      }
    }
  }
  void httpserver::run(const std::string &sysconfpath)
  {
    try
    {
      serverconfig &sysconfigpath = getserversysconfig();
      if (sysconfpath.empty())
      {
        sysconfigpath.configfile.clear();
        sysconfigpath.configfile.append(sysconfpath);
      }

      sysconfigpath.init_path();
      if (sysconfigpath.configfile.empty())
      {
        LOG_ERROR << " specify server.conf fullpath,example: ./xxx_run /etc/paozhu/server.conf " << LOG_END;
        return;
      }
      server_loaclvar &static_server_var = get_server_global_var();

      debug_log::instance()->setDebug(!static_server_var.deamon_enable);
      debug_log::instance()->setLogfile(static_server_var.log_path);

      _inithttpmethodregto(_http_regmethod_table);
      _inithttpmethodregto_pre(_http_regmethod_table);
      sendqueue &send_cache = get_sendqueue();
      send_cache.inti_sendqueue(256);

      VIEW_REG &viewreg = get_viewmetholdreg();
      _initview_method_regto(viewreg);

      WEBSOCKET_REG &wsreg = get_websocket_config();
      _initwebsocketmethodregto(wsreg);

      total_count = sysconfigpath.get_co_thread_num();
      asio::io_context::work worker(io_context);
      for (std::size_t i = 0; i < total_count; ++i)
      {
        runthreads.emplace_back([this]()
                                {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        std::string tempthread=oss.str();
        DEBUG_LOG("frame thread:%s",tempthread.c_str());
        this->io_context.run(); });
      }
      total_count = 0;

      std::thread httpwatch(std::bind(&httpserver::httpwatch, this));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::thread https(std::bind(&httpserver::listeners, this));
      std::thread http(std::bind(&httpserver::listener, this));

      for (int i = 0; i < 1; ++i)
      {
        websocketthreads.emplace_back(std::bind(&httpserver::websocket_loop, this, i));
      }

      if (https.joinable())
      {
        https.join();
      }
      if (http.joinable())
      {
        http.join();
      }
      for (int i = 0; i < 1; ++i)
      {
        if (websocketthreads[i].joinable())
        {
          websocketthreads[i].join();
        }
      }
    }
    catch (std::exception &e)
    {
      LOG_ERROR << " httpserver Exception " << e.what() << LOG_END;
    }
  }
}
