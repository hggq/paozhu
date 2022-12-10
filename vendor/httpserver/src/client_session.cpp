#include "http_socket.h"
#include "http2_frame.h"
#include "client_session.h"
#include "http2_parse.h"
#include "terminal_color.h"
#include "http2_define.h"
#include "http2_huffman.h"
#include "https_brotli.h"
#include "unicode.h"
#include "directory_fun.h"
#include "http_header.h"

namespace http
{
  client_session::client_session(std::list<asio::ip::tcp::socket> sock)
      : _socket(std::move(sock)),
        timer_(_socket.front().get_executor())
  {

    isssl = false;
    timer_.expires_at(std::chrono::steady_clock::time_point::max());
  }

  client_session::client_session(std::list<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket)
      : _sslsocket(std::move(sslsocket)),
        timer_(_sslsocket.front().get_executor())
  {
    isssl = true;
    timer_.expires_at(std::chrono::steady_clock::time_point::max());
  }
  std::shared_ptr<client_session> client_session::get_ptr()
  {
    return shared_from_this();
  }

  void client_session::flush_data()
  {
    timer_.cancel_one();
  }
  void client_session::add_data(const std::string &msg)
  {
    write_msgs_.push_back(msg);
  }
  void client_session::add_data(const unsigned char *buffer, unsigned int buffersize)
  {
    std::string msg;
    msg.resize(buffersize);
    for (int i = 0; i < buffersize; i++)
    {
      msg[i] = buffer[i];
    }
    write_msgs_.push_back(std::move(msg));
  }
  void client_session::write_data(const unsigned char *buffer, unsigned int buffersize)
  {
    std::string msg;
    msg.resize(buffersize);
    for (int i = 0; i < buffersize; i++)
    {
      msg[i] = buffer[i];
    }
    write_msgs_.push_back(std::move(msg));
    timer_.cancel_one();
  }
  void client_session::write_data(const std::string &msg)
  {
    write_msgs_.push_back(msg);
    timer_.cancel_one();
  }

  bool client_session::send_data(const std::string &msg)
  {
    if (isclose)
    {
      return false;
    }

    try
    {
      if (isssl)
      {
        if (_sslsocket.front().lowest_layer().is_open())
        {
          std::unique_lock<std::mutex> lock(writemutex);
          asio::write(_sslsocket.front(), asio::buffer(msg));
        }
        else
        {
          return false;
        }
      }
      else
      {
        if (_socket.front().is_open())
        {
          std::unique_lock<std::mutex> lock(writemutex);
          asio::write(_socket.front(), asio::buffer(msg));
        }
        else
        {
          return false;
        }
      }
      return true;
    }
    catch (std::exception &)
    {
      return false;
    }
  }
  bool client_session::isopensocket()
  {

    try
    {
      if (isssl)
      {
        if (_sslsocket.front().lowest_layer().is_open())
        {
          return true;
        }
        else
        {
          return false;
        }
      }
      else
      {
        if (_socket.front().is_open())
        {
          return true;
        }
        else
        {
          return false;
        }
      }
    }
    catch (std::exception &)
    {
      return false;
    }
  }

  bool client_session::send_switch101()
  {
    std::unique_lock<std::mutex> lock(writemutex);
    try
    {
      std::string tempswitch = "HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nUpgrade: h2c\r\n\r\n";
      if (isssl)
      {
        asio::write(_sslsocket.front(), asio::buffer(tempswitch));
      }
      else
      {

        asio::write(_socket.front(), asio::buffer(tempswitch));
      }
      return true;
    }
    catch (std::exception &)
    {
      return false;
    }
  }
  bool client_session::send_setting()
  {
    std::unique_lock<std::mutex> lock(writemutex);
    try
    {
      unsigned char _setting[] = {0x00, 0x00, 0x0C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x64, 0x00, 0x04, 0x00, 0xFF, 0xFF, 0xFF};

      if (isssl)
      {

        asio::write(_sslsocket.front(), asio::buffer(_setting, 21));
      }
      else
      {

        asio::write(_socket.front(), asio::buffer(_setting, 21));
      }
      return true;
    }
    catch (std::exception &)
    {
      return false;
    }
  }
  bool client_session::send_recv_setting()
  {
    std::unique_lock<std::mutex> lock(writemutex);
    try
    {
      unsigned char _recvack[] = {0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00};
      if (isssl)
      {

        asio::write(_sslsocket.front(), asio::buffer(_recvack, 9));
      }
      else
      {

        asio::write(_socket.front(), asio::buffer(_recvack, 9));
      }
      return true;
    }
    catch (std::exception &)
    {
      return false;
    }
  }
  void client_session::send_window_update(unsigned int up_num, unsigned int stmid)
  {

    unsigned char _recvack[] = {0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    _recvack[12] = up_num & 0xFF;
    up_num = up_num >> 8;
    _recvack[11] = up_num & 0xFF;
    up_num = up_num >> 8;
    _recvack[10] = up_num & 0xFF;
    up_num = up_num >> 8;
    _recvack[9] = up_num & 0xFF;

    std::string msg;
    msg.resize(13);
    for (int i = 0; i < 13; i++)
    {
      msg[i] = _recvack[i];
    }
    write_msgs_.push_back(std::move(msg));

    _recvack[8] = stmid & 0xFF;
    stmid = stmid >> 8;
    _recvack[7] = stmid & 0xFF;
    stmid = stmid >> 8;
    _recvack[6] = stmid & 0xFF;
    stmid = stmid >> 8;
    _recvack[5] = stmid & 0xFF;

    msg.clear();
    for (int i = 0; i < 13; i++)
    {
      msg[i] = _recvack[i];
    }
    write_msgs_.push_back(std::move(msg));

    timer_.cancel_one();
  }
  void client_session::recv_window_update(unsigned int up_num, unsigned int stmid)
  {

    unsigned char _recvack[] = {0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    _recvack[12] = up_num & 0xFF;
    up_num = up_num >> 8;
    _recvack[11] = up_num & 0xFF;
    up_num = up_num >> 8;
    _recvack[10] = up_num & 0xFF;
    up_num = up_num >> 8;
    _recvack[9] = up_num & 0xFF;

    std::string msg;
    msg.resize(13);
    for (int i = 0; i < 13; i++)
    {
      msg[i] = _recvack[i];
    }

    _recvack[8] = stmid & 0xFF;
    stmid = stmid >> 8;
    _recvack[7] = stmid & 0xFF;
    stmid = stmid >> 8;
    _recvack[6] = stmid & 0xFF;
    stmid = stmid >> 8;
    _recvack[5] = stmid & 0xFF;

    // msg.resize(13);
    for (int i = 0; i < 13; i++)
    {
      msg.push_back(_recvack[i]);
    }

    send_data(msg);
  }

  bool client_session::send_data(const unsigned char *buffer, unsigned int buffersize)
  {
    if (isclose)
    {
      return false;
    }

    try
    {
      if (isssl)
      {
        if (_sslsocket.front().lowest_layer().is_open())
        {
          std::unique_lock<std::mutex> lock(writemutex);
          asio::write(_sslsocket.front(), asio::buffer(buffer, buffersize));
        }
        else
        {
          return false;
        }
      }
      else
      {
        if (_socket.front().is_open())
        {
          std::unique_lock<std::mutex> lock(writemutex);
          asio::write(_socket.front(), asio::buffer(buffer, buffersize));
        }
        else
        {
          return false;
        }
      }
      return true;
    }
    catch (std::exception &)
    {
      return false;
    }
  }
  awaitable<void> client_session::loopsendfile(filesend_promise finfo)
  {
    try
    {
      FILE_AUTO fp(std::fopen(finfo.filename.c_str(), "rb"), &std::fclose);
      if (fp.get())
      {
        fseek(fp.get(), 0, SEEK_END);
        long long file_size = ftell(fp.get());
        fseek(fp.get(), 0, SEEK_SET);

        fseek(fp.get(), finfo.begin_num, SEEK_SET);

        long long readnum = 0, send_size = 0, totalsend_num = 0;
        unsigned int data_send_id = 0;
        unsigned int nread = 0;
        send_size = finfo.end_num - finfo.begin_num;
        bool mustwait_window_update = true;
        while (readnum < send_size)
        {
          memset(_write_data, 0x00, 4105);
          nread = 4096;
          if (readnum + 4096 > send_size)
          {
            nread = send_size - readnum;
          }

          nread = fread(&_write_data[9], 1, nread, fp.get());

          data_send_id = nread;
          _write_data[2] = data_send_id & 0xFF;
          data_send_id = data_send_id >> 8;
          _write_data[1] = data_send_id & 0xFF;
          data_send_id = data_send_id >> 8;
          _write_data[0] = data_send_id & 0xFF;

          readnum += nread;
          totalsend_num += nread;
          if (readnum >= send_size)
          {
            _write_data[4] = 0x01;
          }
          data_send_id = finfo.stream_id;
          _write_data[8] = data_send_id & 0xFF;
          data_send_id = data_send_id >> 8;
          _write_data[7] = data_send_id & 0xFF;
          data_send_id = data_send_id >> 8;
          _write_data[6] = data_send_id & 0xFF;
          data_send_id = data_send_id >> 8;
          _write_data[5] = data_send_id & 0x7F;

          if (isssl)
          {
            std::unique_lock<std::mutex> lock(writemutex);
            // asio::write(_sslsocket.front(),asio::buffer(_write_data, nread+9));
            co_await asio::async_write(_sslsocket.front(), asio::buffer(_write_data, nread + 9), use_awaitable);
          }
          else
          {
            std::unique_lock<std::mutex> lock(writemutex);
            co_await asio::async_write(_socket.front(), asio::buffer(_write_data, nread + 9), use_awaitable);
            // asio::write(_socket.front(),asio::buffer(_write_data, nread+9));
          }
          window_update_num -= nread;

          if (window_update_num < nread * 2)
          {
            std::promise<int> p;
            std::future<int> f{p.get_future()};

            window_update_results.emplace_back(std::move(f));
            window_update_promise = std::move(p);
            atomic_bool = true;

            try
            {
              int result = window_update_results.front().get();
              window_update_results.pop_front();
              mustwait_window_update = false;
            }
            catch (const std::exception &e)
            {
              break;
            }
          }

          if (file_size > 10485760)
          {

            if (mustwait_window_update && totalsend_num < 20971520)
            {
              std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
          }
        }

        finfo.isfinish = true;
        if (!_cache_send_results.empty())
        {
          try
          {
            _cache_send_promise.set_value(1);
          }
          catch (const std::exception &e)
          {
            std::cerr << e.what() << '\n';
          }
        }
      }
    }
    catch (std::exception &)
    {
    }
    co_return;
  }
  awaitable<void> client_session::loopwriter()
  {
    try
    {

      while (true)
      {
        if (write_msgs_.empty() && send_queue_list.empty())
        {
          asio::error_code ec;
          co_await timer_.async_wait(redirect_error(use_awaitable, ec));
        }
        else
        {
          if (!send_queue_list.empty())
          {
            while (!send_queue_list.empty())
            {

              auto send_ptr = send_queue_list.front();
              send_queue_list.pop_front();
              if (isssl)
              {
                if (_sslsocket.front().lowest_layer().is_open())
                {
                  std::unique_lock<std::mutex> lock(writemutex);
                  co_await asio::async_write(_sslsocket.front(),
                                             asio::buffer(send_ptr->data, send_ptr->data_size), use_awaitable);
                }
                else
                {
                  break;
                }
              }
              else
              {
                if (_socket.front().is_open())
                {
                  std::unique_lock<std::mutex> lock(writemutex);
                  co_await asio::async_write(_socket.front(),
                                             asio::buffer(send_ptr->data, send_ptr->data_size), use_awaitable);
                }
                else
                {
                  break;
                }
              }

              if (send_ptr->timeid > 0)
              {
                try
                {
                  peer_promise_list[send_ptr->timeid].send_promise.set_value(1);
                }
                catch (const std::exception &e)
                {
                  std::cerr << e.what() << '\n';
                }
              }
            }
          }
          else
          {
            if (isssl)
            {
              if (_sslsocket.front().lowest_layer().is_open())
              {
                std::unique_lock<std::mutex> lock(writemutex);
                co_await asio::async_write(_sslsocket.front(),
                                           asio::buffer(write_msgs_.front()), use_awaitable);
              }
              else
              {
                break;
              }
            }
            else
            {
              if (_socket.front().is_open())
              {
                std::unique_lock<std::mutex> lock(writemutex);
                co_await asio::async_write(_socket.front(),
                                           asio::buffer(write_msgs_.front()), use_awaitable);
              }
              else
              {
                break;
              }
            }
            write_msgs_.pop_front();
          }
        }
      }
    }
    catch (std::exception &)
    {
      stop();
    }
  }

  awaitable<void> client_session::sslwriter()
  {
    try
    {

      while (_sslsocket.front().lowest_layer().is_open())
      {
        if (write_msgs_.empty())
        {
          asio::error_code ec;
          co_await timer_.async_wait(redirect_error(use_awaitable, ec));
        }
        else
        {
          co_await asio::async_write(_sslsocket.front(),
                                     asio::buffer(write_msgs_.front()), use_awaitable);
          write_msgs_.pop_front();
        }
      }
    }
    catch (std::exception &)
    {
      stop();
    }
  }
  awaitable<void> client_session::writer()
  {
    try
    {
      while (_socket.front().is_open())
      {
        if (write_msgs_.empty())
        {
          asio::error_code ec;
          co_await timer_.async_wait(redirect_error(use_awaitable, ec));
        }
        else
        {
          co_await asio::async_write(_socket.front(),
                                     asio::buffer(write_msgs_.front()), use_awaitable);
          write_msgs_.pop_front();
        }
      }
    }
    catch (std::exception &)
    {
      stop();
    }
  }

  void client_session::stop()
  {
    isclose = true;
    if (isssl)
    {
      _sslsocket.front().shutdown();
    }
    else
    {
      _socket.front().close();
    }

    timer_.cancel();
  }

  std::string client_session::getremoteip()
  {
    if (isssl)
    {
      client_ip = _sslsocket.front().lowest_layer().remote_endpoint().address().to_string();
    }
    else
    {
      client_ip = _socket.front().remote_endpoint().address().to_string();
    }
    return client_ip;
  }
  unsigned int client_session::getremoteport()
  {
    if (isssl)
    {
      client_port = _sslsocket.front().lowest_layer().remote_endpoint().port();
    }
    else
    {
      client_port = _socket.front().remote_endpoint().port();
    }
    return client_port;
  }
  std::string client_session::getlocalip()
  {
    if (isssl)
    {
      server_ip = _sslsocket.front().lowest_layer().local_endpoint().address().to_string();
    }
    else
    {
      server_ip = _socket.front().local_endpoint().address().to_string();
    }
    return server_ip;
  }
  unsigned int client_session::getlocalport()
  {
    if (isssl)
    {
      server_port = _sslsocket.front().lowest_layer().local_endpoint().port();
    }
    else
    {
      server_port = _socket.front().local_endpoint().port();
    }
    return server_port;
  }

}