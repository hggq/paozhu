#ifndef _ORM_COMMON_H
#define _ORM_COMMON_H

#include <asio.hpp>
#include <asio/ssl.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "session_buf.h"

namespace orm {

enum class DB_TYPE { MYSQL, POSTGRESQL, SQLITE };

enum class wq {
  qb = 0,
  bt = 1,
  be,
  eq,
  lt,
  le,
  in,
  like,
  notin,
  between,
  isnull,
  notnull
};
struct orm_conn_t {
  // std::string type;
  std::string host;
  std::string port;
  std::string dbname;
  std::string user;
  std::string password;
  std::string pretable;
  std::string dbtype;

  std::string tag;
  std::string charset;

  bool isssl = false;
  bool sslverify =
      false; // 是否校验服务器证书链（配合系统 CA 或 SSL_CERT_FILE）
  std::string
      sslhost; // 证书主机名/SNI（域名校验），为空则只校验证书链不校验域名
  bool issock = false;
  bool isdebug = false;
  bool islocal = false;
  unsigned char link_type = 0; // 0 edit 1 select 2 backup
  unsigned char max_pool = 0;
  unsigned char min_pool = 0;
  unsigned char charset_val = 0;
  DB_TYPE db_type = DB_TYPE::MYSQL;

  bool operator==(const orm_conn_t &o) const {
    return std::tie(host, port, dbname, user, password, pretable, dbtype,
                    tag, charset, sslhost,
                    isssl, sslverify, issock, isdebug, islocal,
                    link_type, max_pool, min_pool, charset_val, db_type) ==
           std::tie(o.host, o.port, o.dbname, o.user, o.password, o.pretable, o.dbtype,
                    o.tag, o.charset, o.sslhost,
                    o.isssl, o.sslverify, o.issock, o.isdebug, o.islocal,
                    o.link_type, o.max_pool, o.min_pool, o.charset_val, o.db_type);
  }
  bool operator!=(const orm_conn_t &o) const { return !(*this == o); }
};

struct pack_info_t {
  unsigned int length = 0;
  unsigned int current_length = 0;
  unsigned char seq_id = 0;
  unsigned char error = 0;
  unsigned char padd_length = 0;
  unsigned char padd_str[5] = {0x00};
  std::string data;
};

struct field_info_t {
  std::string org_name;
  std::string name;
  std::string org_table;
  std::string table;
  std::string dbname;
  unsigned char field_type = 0;
  unsigned char decimals = 0;
  unsigned short charset = 0;
  unsigned short flags = 0;
  unsigned int column_length = 0;
};

// 结果集行数据 (PG / SQLite 共用)
struct pg_row_data_t {
  std::vector<std::string> values; // 每列的值（NULL 用空字符串+is_null标记）
  std::vector<bool> is_null;
};

// SQLite 结果集 (values 与 is_null 一一对应, BLOB 以原始字节存于 string)
struct sqlite_query_result {
  std::vector<std::string> column_names;
  std::vector<std::vector<std::string>> rows;
  std::vector<std::vector<bool>> is_null;
  int affected_rows = 0;
};

// SQLite 标量查询结果 (如 SELECT COUNT(*))
struct sqlite_scalar_result {
  std::string value;
  bool is_null = true;
};

// SQLite 绑定参数 (用于参数化执行, 二进制安全)
struct sqlite_bind_param {
  int type = 0; // SQLITE_TEXT = 3, SQLITE_INTEGER = 1, etc.
  std::string data;
};

struct table_columns_info_t {
  std::string col_name;
  std::string comment;
  std::string default_value;
  unsigned char col_type;
  unsigned int col_length;
  unsigned char decimals;
  unsigned char big_type;
  bool is_pk = false;
  bool is_auto_inc = false;
  bool is_unsigned = false;
  bool is_datetime = false;
  bool is_nullable = true;
};

struct orm_left_join_t {
  std::string selectsql;
  std::string join_table;
  std::string wheresql;
  std::string ordersql;
  std::string limitsql;
  std::string parbysql;
  std::string subsql;
};

struct orm_where_sql_t {
  bool begin_sub = false; // 只支持2级括号 Only level 2 brackets are supported
  bool end_sub = false;
  wq op_type;
  unsigned char pos_n = 0; // Which position is the preprocessing statement at
  std::string filed_name;
  std::string filed_value; // BETWEEN?
};

struct orm_conn_link_t {
  std::unique_ptr<asio::ip::tcp::socket> socket;
  std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> sslsocket;
  std::unique_ptr<asio::local::stream_protocol::socket> localsocket;
  std::shared_ptr<asio::ssl::context> ssl_context;
  asio::error_code ec;
  asio::io_context *io_ctx = nullptr;
  unsigned int begin_time = 0;
  DB_TYPE db_type = DB_TYPE::MYSQL;
  unsigned char sock_type = 0; // 0: none, 1: tcp, 2: ssl, 3: local socket
  // 工厂方法：根据配置创建 conn_link
  static std::unique_ptr<orm_conn_link_t> create(asio::io_context &ioc,
                                                 DB_TYPE type);

  // 关闭连接
  void close();

  // 检查是否已关闭
  bool is_closed() const;
};

std::string to_escape(std::string_view val, bool no_backslash_escapes = false);

// ---------------------------------------------------------------------------
// 全 ORM 共用累积缓冲容量池（设计与 tls13 项目 session_buf 一致，两项目合并后共用）：
// 协议解析需要跨读次累积缓冲（read_some 每次最多 CACHE_DATA_LENGTH 字节），
// 每次查询新建/销毁会产生重复堆分配；改为池借出/归还，热路径稳定后零堆分配。
// MySQL / PG / SQLite 连接层统一使用本池，不再各自建池。
// ---------------------------------------------------------------------------
inline session_buf g_orm_buf_pool;

// RAII：构造时从容量池借出累积缓冲（不足 4096 则补齐首次容量），
// 析构时归还——覆盖错误/中止等所有提前返回路径，避免漏归还。
// 用法：pooled_accum_buf holder; auto &accum_buf = holder.buf;
struct pooled_accum_buf {
  std::vector<uint8_t> buf;
  pooled_accum_buf() {
    buf = g_orm_buf_pool.acquire();
    if (buf.capacity() < 4096)
      buf.reserve(4096);
  }
  ~pooled_accum_buf() { g_orm_buf_pool.release(std::move(buf)); }
};

} // namespace orm

#endif