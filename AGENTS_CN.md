# Paozhu C++ Web Framework - AI 开发指南

## 一、项目概述

Paozhu (炮竹) 是一个高性能的 C++20 Web 框架，基于 Asio 异步网络库构建，支持 HTTP/HTTPS/HTTP2 协议。框架采用 MVC 架构，内置 ORM、模板引擎、协程支持和丰富的功能模块。

### 核心特性
- **异步 IO**: 基于 Asio 的异步非阻塞网络模型
- **协程支持**: C++20 co_await 关键字编写异步代码
- **HTTP/HTTP2**: 原生支持 HTTP/1.1 和 HTTP/2 协议
- **ORM**: 内置 ORM，支持 MySQL PostgreSQL 和 SQLite
- **OCSP Stapling**: 自动 OCSP stapling 支持
- **ACME**: 自动 SSL 证书申请和管理
- **模块扩展**: 图片处理、Excel、Word、PDF、图表等

### 编译标准
- C++20 或更高
- OpenSSL 3.0+ (SSL/TLS 支持)
- 跨平台: macOS, Linux, Windows

---

## 二、目录结构

```
paozhu/
├── conf/                          # 配置文件目录
│   ├── server.conf                # 服务器主配置
│   ├── orm.conf                   # 数据库连接配置
│   ├── acme.conf                  # ACME 证书配置
│   └── ...                        # 其他配置和 SQL 文件
│
├── controller/                    # 控制器层
│   ├── include/                   # 控制器头文件（自动生成，编译时从 controller/src 抽取，无需手动修改）
│   │   ├── admin/                 # 后台管理控制器
│   │   └── test*.h               # 功能测试控制器
│   └── src/                       # 控制器实现
│       ├── admin/                 # 后台管理实现
│       └── test*.cpp              # 功能测试实现
│
├── models/                        # 数据模型层（每个数据库标签一棵树）
│   ├── cms/                       # CMS 相关模型（MySQL）
│   ├── pg/                        # PostgreSQL 模型
│   ├── lite/                      # SQLite 模型
│   └── include/                   # 默认数据库标签的模型
│
├── orm/                           # ORM 层（自动生成）
│   ├── cms/include/               # CMS ORM 操作类
│   │   ├── *_base.h               # 基础字段定义（自动生成）
│   │   └── *_opsql.h              # SQL 操作中间层（自动生成）
│   ├── pg/include/                # PostgreSQL 同构布局
│   ├── lite/include/              # SQLite 同构布局
│   ├── include/                   # 默认数据库标签的生成文件
│   ├── {table}_opsql.cpp          # 含外键的表，oneXXX/manyXXX 的类外实现
│   └── orm.h                      # ORM 统一入口（包含所有方言）
│
├── view/                          # 视图模板（HTML）
│   ├── admin/                     # 后台管理视图
│   ├── home/                      # 首页视图
│   └── ...                        # 其他视图
│
├── viewsrc/                       # 视图编译产物（C++ 源码）
│   ├── include/                   # 视图注册头文件
│   └── view/                      # 视图实现
│
├── vendor/                        # 第三方/内部库
│   ├── httpserver/                # HTTP 服务器核心
│   │   ├── include/               # 服务器头文件
│   │   │   ├── request.h          # HTTP 请求处理
│   │   │   ├── httppeer.h        # HTTP 请求/响应处理对象
│   │   │   ├── router.h           # URL 路由
│   │   │   └── ...               # 其他头文件
│   │   └── src/                   # 服务器实现
│   ├── httpcli/                   # CLI 工具（代码生成器）
│   ├── pzexcel/                   # Excel 读写
│   ├── pzword/                    # Word 文档生成
│   ├── pzzip/                     # ZIP 压缩/解压
│   ├── pzimage/                   # 图片处理（JPG/PNG）
│   ├── pzcharts/                  # SVG 图表生成
│   ├── webpdf/                    # HTML 转 PDF
│   ├── sms/                       # 短信发送
│   └── webpay/                    # 微信/支付宝支付
│
├── libs/                          # 业务工具库
│   ├── img/                       # 图片上传处理
│   ├── markdown/                  # Markdown 转 HTML
│   ├── pinyin/                    # 汉字转拼音
│   ├── ipdata/                    # IP 地理位置查询
│   └── types/                     # 通用类型定义
│
├── common/                        # 公共定义
│   ├── autorestfulpaths.hpp       # RESTful 路径注册
│   ├── reghttpmethod.hpp          # HTTP 方法注册
│   ├── autocontrolmethod.hpp      # 控制器方法注册（自动生成，编译时从 controller/src 抽取，无需手动修改）
│   └── ...
│
├── sockets/                       # Socket 扩展
├── websockets/                    # WebSocket 支持
├── startup/                       # 程序入口
│   ├── main_dev.cpp               # 开发模式入口
│   ├── main_daemon.cpp            # 守护进程入口
│   └── main_docker.cpp            # Docker 入口
│
├── www/                           # Web 静态文件根目录
├── docs/                          # 示例资源（图片、字体等）
├── CMakeLists.txt                 # CMake 构建配置
└── AGENTS.md                      # 本文档
```

> **自动生成文件说明（请勿手动修改）**：
> - `models/` 与 `orm/` 目录下的模型代码均由 `bin/paozhu_cli orm <dbtag>` 从数据库自动生成，最好不要手动修改；如需改动共享行为请修改 `vendor/httpserver/include/` 下的模板并重新生成（见 §5.1）。
> - `common/autocontrolmethod.hpp` 由构建系统在编译项目时，从 `controller/src/` 抽取控制器函数自动生成，无需手动修改；改动控制器后重新编译即可刷新。
> - `controller/include/` 目录同样由构建系统自动生成（为每个缺少头文件的 `controller/src/*.cpp` 补一个空头文件），无需手动修改。

---

## 三、配置文件说明

### 3.1 server.conf - 服务器配置

```ini
[default]
; 线程配置
threadmax = 1024          ; 最大线程数
threadmin = 5             ; 最小线程数
cothreadnum = 8           ; 协程运行线程数

; 端口配置
httpport = 80             ; HTTP 端口
httpsport = 443           ; HTTPS 端口

; SSL 配置
mainhost = www.hggq.com  ; 主域名
certificate_chain_file = www.hggq.com.pem    ; 证书链文件
private_key_file = www.hggq.com.key         ; 私钥文件
dh4096.pem               ; DH 参数文件

; 路径配置
modelspath = ./models     ; 模型目录
viewpath = ./view         ; 视图目录
controlpath = ./controller; 控制器目录
temppath = ./temp         ; 临时文件目录
logpath = ./log           ; 日志目录
wwwpath = ./www/default  ; Web 根目录

; 功能开关
http2_enable = 1          ; HTTP/2 开关
debug_enable = 1          ; 调试模式
deamon_enable = 0         ; 守护进程模式

; ACME 配置
acme_auto = 0             ; 自动 SSL 证书更新
acme_every_day = 7        ; 每天几点执行 ACME
ocsp_intv_time = 14400    ; OCSP 更新间隔（秒）

; 会话配置
session_type = 1          ; 0=file 1=memory 2=redis
static_file_compress_cache = 1  ; 静态文件压缩缓存

; 上传配置
upload_max_size = 16777216  ; 最大上传尺寸（16MB）
```

### 3.2 orm.conf - 数据库配置

```ini
[section_name]     ; 数据库标签，如 default、cms、pg、lite
type = main        ; main（主库）或 second（从库）
host = 127.0.0.1   ; 数据库主机；dbtype = sqlite 时这里是 sqlite 数据库文件路径
port = 3306        ; 端口（MySQL: 3306, PostgreSQL: 5432）；sqlite 时留空
dbname = database_name  ; 数据库名
user = username    ; 用户名
password = pass    ; 密码
pretable =         ; 表前缀
maxpool = 5        ; 最大连接池
dbtype = mysql     ; mysql | postgresql | sqlite
charset = utf8mb4  ; 字符集
#ssl = ON          ; 启用 SSL
#sslverify = ON    ; 校验证书链
#sslhost =         ; 证书域名
```

节名**就是**数据库标签：它是 `paozhu_cli orm <tag>` 的参数，是 `model.dbtag` 的值，
也是生成代码传给 `orm::db_conn("cms")` 的那个字符串。

**数据库标签约定**（本仓库）:
- `default` - 基准测试表（`world`、`fortune`、`fk_parent`、`fk_child`）
- `cms` - CMS 系统数据库
- `docs` - 文档数据库
- `pg` - PostgreSQL 测试库（`dbtype = postgresql`）
- `lite` - SQLite 测试库（`dbtype = sqlite`，`host = ./docs/sqlite_lite.db`，`port` 留空）

### 3.3 acme.conf - ACME 证书配置

```ini
[info]
acme_path = ./acme         ; ACME 工作目录
cert_path = ./conf         ; 证书存储目录
email = your@email.com    ; 邮箱（必须）
days_remain = 10           ; 提前多少天更新证书

[zerossl]
server_url = https://acme.zerossl.com/v2/DV90  ; ZeroSSL 接口
eab_kid =                  ; EAB Key ID
eab_hmac_key =             ; EAB HMAC Key
```

---

## 四、控制器开发规范

### 4.1 控制器文件结构

```cpp
// controller/src/example.cpp
#include "orm.h"              // ORM 入口
#include "httppeer.h"         // HTTP 请求/响应处理对象
#include "func.h"             // 工具函数
#include "example.h"          // 对应头文件

namespace http
{
// 控制器函数实现
}
```

### 4.2 URL 路由注册

使用 `//@urlpath` 注释标记 URL 路由：

```cpp
//@urlpath(null,hello)
std::string testhello(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "Hello World!";
    return "";
}
```

**路由格式**: `//@urlpath(前置过滤器,URL路径)`

- 第一个参数: 前置过滤器函数名（`null` 表示无过滤器）
- 第二个参数: URL 路径

### 4.3 前置过滤器

```cpp
//@urlpath(null,admin/islogin)
std::string admin_islogin(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    if (client.session["userid"].to_int() == 0)
    {
        client.val["code"] = 0;
        client.val["msg"] = "please login";
        client.out_json();
        return "exit";         // 返回 "exit" 阻止后续执行
    }
    return "ok";
}

//@urlpath(admin_islogin,admin/main)  // 使用过滤器
std::string admin_main(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.view("admin/main");
    return "";
}
```

### 4.4 httppeer 常用 API

```cpp
httppeer &client = peer->get_peer();

// 请求数据
client.post["field"].to_string();   // POST 数据
client.post["field"].to_int();      // POST 数据转 int
client.get["param"];                // GET 参数
client.client_ip;                   // 客户端 IP
client.is_ssl();                    // 是否 HTTPS

// 响应输出
client << "content";                // 直接输出
client.view("path");                // 渲染视图（不带主题前缀）
peer->theme_view("path");           // 渲染主题视图（自动添加 themes 前缀）
client.goto_url("/path");           // 重定向
client.goto_url("/path", 3, msg);   // 带延迟和消息的重定向

// JSON 响应
client.val["key"] = value;          // 设置 JSON 字段
client.out_json();                  // 输出 JSON

// 会话管理
client.session["key"] = value;      // 设置会话
client.session["key"].to_int();    // 读取会话
client.save_session();              // 保存会话

// 文件上传
client.files["field"]["filename"];  // 上传文件名
client.files["field"]["tempfile"];  // 临时文件路径

// 路径
client.get_sitepath();              // 获取站点路径
client.get_hosturl();               // 获取主机 URL
```

### 4.5 主题视图 (theme_view)

`theme_view` 是支持多主题/多租户的视图渲染方法。与 `view` 的区别是会自动根据当前域名配置的 `themes` 值添加路径前缀：

```cpp
// view: 直接渲染 view/front/producthome.html
peer->view("front/producthome");

// theme_view: 如果当前域名配置了 themes = "saas1"
// 则实际渲染 view/saas1/front/producthome.html
// 如果没有配置 themes，则等价于 peer->view("front/producthome")
peer->theme_view("front/producthome");
```

**用法示例**（`theme_view` 的声明位于 `vendor/httpserver/include/httppeer.h`）：
```cpp
peer->theme_view("front/producthome");
peer->theme_view("front/productcatalogue");
peer->theme_view("front/producttopic");
peer->theme_view("front/productlist");
peer->theme_view("front/productbrand");
peer->theme_view("front/productsearch");
```

**配置方式**（在域名配置文件中）：
```ini
[saas1.example.com]
themes = saas1              ; 主题目录名，会被添加到视图路径前
themes_url = /saas1/        ; 主题的静态资源 URL 前缀
```

**配套 API**:

```cpp
// 获取当前域名的主题名
std::string theme = peer->get_theme();     // 返回 "saas1" 或 ""

// 手动拼接主题路径后使用 view（不推荐，仅作兼容用途）
std::string viewfile = peer->get_theme();
if (!viewfile.empty()) viewfile += "/";
viewfile += "front/productcatalogue";
peer->view(viewfile);  // 等价于 peer->theme_view("front/productcatalogue")

// 获取当前域名的主题 URL 前缀（用于模板中的静态资源）
std::string themeUrl = peer->get_themeurl();  // 返回 "/saas1/" 或 ""
```

**使用场景**：同一个系统服务多个租户（SaaS），每个租户有独立的域名和主题模板目录（如 `view/saas1/`、`view/saas2/`），控制器代码统一使用 `theme_view` 即可，无需关心具体主题名。

### 4.6 完整控制器示例

```cpp
#include "orm.h"
#include "httppeer.h"
#include "func.h"
#include "testcrud.h"

namespace http
{
// 列表页
//@urlpath(null,crud/list)
std::string crud_list(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    
    auto model = orm::cms::Article();
    unsigned int page = client.get["page"].to_int();
    if (page == 0) page = 1;
    
    auto [minpage, maxpage, curpage, totalpage] = model.page(page, 10, 5);   // 第3参数 list_num：分页条显示的页码个数
    model.where("userid", client.session["userid"].to_int())
         .desc("aid")
         .fetch();
    
    client.val["list"].set_array();
    for (auto &item : model.record)
    {
        obj_val row;
        row["id"] = item.aid;
        row["title"] = item.title;
        client.val["list"].push(row);
    }
    
    client.val["pagination"].set_object();
    client.val["pagination"]["total"] = totalpage;
    client.val["pagination"]["current"] = curpage;
    
    peer->view("crud/list");
    return "";
}

// 新增
//@urlpath(null,crud/addpost)
std::string crud_addpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    
    auto model = orm::cms::Article();
    model.data.userid = client.session["userid"].to_int();
    model.data.title = client.post["title"].to_string();
    model.data.content = client.post["content"].to_string();
    auto [effect, newId] = model.save();   // 返回 [生效行数, 自增ID]，save 已自动 setPK
    
    client.goto_url("/crud/list", 3, "添加成功！");
    return "";
}

// 编辑
//@urlpath(null,crud/editpost)
std::string crud_editpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int id = client.post["id"].to_int();
    
    auto model = orm::cms::Article();
    model.data.title = client.post["title"].to_string();
    model.where("aid", id);
    model.update("title");
    
    client.goto_url("/crud/list", 3, "修改成功！");
    return "";
}

// 删除
//@urlpath(null,crud/delete)
std::string crud_delete(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int id = client.get["id"].to_int();
    
    auto model = orm::cms::Article();
    model.where("aid", id).remove();
    
    client.val["code"] = 1;
    client.val["msg"] = "删除成功";
    client.out_json();
    return "";
}
}
```

### 4.7 协程控制器模式

控制器可以使用 C++20 协程编写，实现非阻塞异步 IO。框架会根据返回类型自动检测并选择对应的执行方式。

**核心规则**:
- 返回类型: `asio::awaitable<std::string>`（替代 `std::string`）
- 所有异步 ORM 操作使用 `co_await` 等待
- 使用 `co_return ""` 替代 `return ""`
- 批量操作使用 `lock_conn()` / `unlock_conn()` 复用单条连接（避免重复连接开销）

**示例 1 - 基础协程控制器**（无数据库，纯文本响应）:

```cpp
//@urlpath(null,plaintext)
asio::awaitable<std::string> techempowerplaintext(std::shared_ptr<httppeer> peer)
{
    peer->type("text/plain; charset=UTF-8");
    peer->set_header("Date", get_gmttime());
    peer->output = "Hello, World!";
    co_return "";
}
```

**示例 2 - 协程 + 异步 ORM 查询**:

```cpp
//@urlpath(null,db)
asio::awaitable<std::string> techempowerdb(std::shared_ptr<httppeer> peer)
{
    peer->type("application/json; charset=UTF-8");
    peer->set_header("Date", get_gmttime());

    auto myworld = orm::World();
    unsigned int rd_num = rand_range(1, 10000);
    myworld.where("id", rd_num);
    myworld.limit(1);
    co_await myworld.async_fetch_one();   // 异步单条查询

    peer->output = myworld.data_tojson();
    co_return "";
}
```

**示例 3 - 协程 + 异步批量查询**（使用 `lock_conn`/`unlock_conn`）:

```cpp
//@urlpath(null,queries)
asio::awaitable<std::string> techempowerqueries(std::shared_ptr<httppeer> peer)
{
    peer->type("application/json; charset=UTF-8");
    peer->set_header("Date", get_gmttime());

    unsigned int get_num = peer->get["queries"].to_int();
    if (get_num == 0) get_num = 1;
    else if (get_num > 500) get_num = 500;

    auto myworld = orm::World();
    myworld.record.reserve(get_num);
    myworld.lock_conn();                 // 锁定连接以便循环内复用
    for (unsigned int i = 0; i < get_num; i++)
    {
        myworld.wheresql.clear();
        unsigned int rd_num = rand_range(1, 10000);
        myworld.where("id", rd_num);
        co_await myworld.async_fetch_append();  // 异步追加查询结果
    }
    myworld.unlock_conn();               // 释放连接
    peer->output = myworld.to_json();
    co_return "";
}
```

**示例 4 - 协程 + 异步更新**（循环内查询 + 更新）:

```cpp
//@urlpath(null,updates)
asio::awaitable<std::string> techempowerupdates(std::shared_ptr<httppeer> peer)
{
    peer->type("application/json; charset=UTF-8");
    peer->set_header("Date", get_gmttime());
    unsigned int get_num = peer->get["queries"].to_int();
    if (get_num == 0) get_num = 1;
    else if (get_num > 500) get_num = 500;

    auto myworld = orm::World();
    myworld.record.clear();
    myworld.record.reserve(get_num);
    myworld.lock_conn();
    for (unsigned int i = 0; i < get_num; i++)
    {
        myworld.wheresql.clear();
        myworld.where("id", rand_range(1, 10000));
        co_await myworld.async_fetch_append();
        if (myworld.effect() > 0)
        {
            unsigned int j = myworld.record.size() - 1;
            myworld.data.randomnumber = rand_range(1, 10000);
            myworld.record[j].randomnumber = myworld.data.randomnumber;
            co_await myworld.async_update("randomnumber");  // 异步更新
        }
    }
    myworld.unlock_conn();
    peer->output = myworld.to_json();
    co_return "";
}
```

**可用的异步 ORM 方法**:

文本路径（值被转义进语句）:

| 同步方法 | 异步方法 | 说明 |
|----------|----------|------|
| `fetch()` | `async_fetch()` | 查询所有匹配记录 |
| `fetch_one()` | `async_fetch_one(isappend = false)` | 查询单条记录 |
| `fetch_append()` | `async_fetch_append()` | 查询并追加到现有结果集 |
| `fetch_to(vec)` | `async_fetch_to(vec)` | 填充自定义 `orm::Base` 结构体数组 |
| `fetch_one_to(obj)` | `async_fetch_one_to(obj)` | 填充一个自定义 `orm::Base` 结构体 |
| `count()` | `async_count()` | 统计匹配记录数 |
| `page(p, pp, ln)` | `async_page(p, pp = 10, ln = 5)` | 同样返回 `[bar_min, bar_max, current, total]` 四元组 |
| `save(isrealnew)` | `async_save(isrealnew = false)` | 新增记录 |
| `insert()` / `insert(row)` / `insert(rows)` | `async_insert()` / `async_insert(row)` / `async_insert(rows)` | 同样返回 `[effect, id]` 二元组；`row` 是一个 `meta` 结构体，`rows` 是 `std::vector<meta>` |
| `update()` / `update(fields)` | `async_update()` / `async_update(fields)` | 更新记录 |
| `update_dirty()` | `async_update_dirty()` | 只更新经生成的逐字段 setter 写过的列（§5.6） |
| `update_col(col, n, sign)` | `async_update_col(col, n, sign = '+')` | 自增或自减 |
| `replace_col(col, old, new)` | `async_replace_col(col, old, new)` | 在 UPDATE 中使用 `REPLACE()` |
| `remove()` / `remove(id)` | `async_remove()` / `async_remove(id)` | 删除记录 |

`async_fetch_to` 与 `async_fetch_one_to` 各还有一个带回调参数的重载
`(vec, callback)` / `(obj, callback)`。它在读取每一行的过程中，对每一个非 `NULL` 列触发一次，
形参为 `(row&, col_name, const unsigned char *data, std::size_t len, unsigned char field_type, unsigned char flag)`，
回调返回后该行仍会被追加进结果集 —— 它是一个按列钩子，不是流式模式。

预编译路径（值作为参数绑定，见 §5.8）—— 每个 `exec_` 方法都有一个签名与返回值完全
一致的 `async_exec_` 版本:

| 同步方法 | 异步方法 |
|----------|----------|
| `exec_count()` | `async_exec_count()` |
| `exec_page()` | `async_exec_page()` |
| `exec_fetch()` | `async_exec_fetch()` |
| `exec_fetch_to(rows)` | `async_exec_fetch_to(rows)` |
| `exec_fetch_append()` | `async_exec_fetch_append()` |
| `exec_one()` | `async_exec_one()` |
| `exec_one_to(rec)` | `async_exec_one_to(rec)` |
| `exec_one_append(vec)` | `async_exec_one_append(vec)` |
| `exec_insert()` | `async_exec_insert()` |
| `exec_insert_batch()` | `async_exec_insert_batch()` |
| `exec_update()` | `async_exec_update()` |
| `exec_update_dirty()` | `async_exec_update_dirty()` |
| `exec_update_col()` | `async_exec_update_col()` |
| `exec_replace_col()` | `async_exec_replace_col()` |
| `exec_remove()` | `async_exec_remove()` |

`exec_update_fields()` 没有异步版本；请改用 `async_exec_update("col1,col2")`。

**注意**: 同步控制器（返回 `std::string`）和协程控制器（返回 `asio::awaitable<std::string>`）可以在同一项目中混用。框架会根据函数签名自动选择正确的执行路径。

---

## 五、ORM 使用指南

### 5.1 生成 ORM 代码

使用 CLI 工具从真实数据库自动生成 ORM 代码：

```bash
# 每个数据库标签一条命令（标签就是 conf/orm.conf 里的节名）
./bin/paozhu_cli orm default   # [default] 的 MySQL 表 -> orm/include/,    models/include/
./bin/paozhu_cli orm cms       # [cms] 的 MySQL 表      -> orm/cms/include/, models/cms/
./bin/paozhu_cli orm pg        # [pg] 的 PostgreSQL 表  -> orm/pg/include/,  models/pg/
./bin/paozhu_cli orm lite      # [lite] 的 SQLite 表    -> orm/lite/include/, models/lite/
```

**模板不参与编译。** `vendor/httpserver/include/mysqlorm.hpp`、`sqliteorm.hpp`、
`postgresqlorm.hpp` 是代码生成模板：`paozhu_cli orm` 把它们当文本读取，将完整的类体复制
进每一个生成的 `_opsql.h`。因此修改模板在编译期**不会产生任何变化** —— 已生成的文件仍
保留生成时所用模板的内容。改动模板或 `orm_common.h` 之后，必须重新生成你使用的每一个
标签并重新编译；否则通过新 API 构造的查询会静默沿用旧行为（通常表现为返回 0 行）。

### 5.2 模型文件组成

每个表对应下列文件（`{tag}` 为数据库标签；默认标签不带该层目录）：

- `orm/{tag}/include/{table}_base.h` - 字段元数据、`cols` 枚举、行 `meta` 结构体
  （自动生成，不手动修改）
- `orm/{tag}/include/{table}_opsql.h` - SQL 操作层：模板类体的完整副本，外加按列展开的
  switch 与生成的逐字段 DSL（自动生成，不手动修改）
- `orm/{tag}/{table}_opsql.cpp` - 仅含外键的表才有：`oneXXX` / `manyXXX` 访问器的类外
  模板特化实现
- `models/{tag}/ModelName.cpp` + `models/{tag}/include/ModelName.h` - 具体的模型类，把上
  面两个模板绑定起来，例如
  `class Fortune : public fortune_opsql<Fortune, fortune_base>`

`orm/orm.h` 会包含所有标签的模型，因此一句 `#include "orm.h"` 即可访问
`orm::Fortune`、`orm::cms::Article`、`orm::pg::Fortune` 和 `orm::lite::Fortune`。

### 5.3 ORM 基本查询

同一个条件有三种等价写法。一条链里只用一种即可，三者最终都落入同一个 `wheresql` 容器。

```cpp
#include "orm.h"

auto model = orm::cms::Article();

// 1. 列名字符串
model.where("userid", 123).fetch();

// 2. 类型安全的列枚举（orm::{tag}::{table}_info::cols）
model.where(orm::cms::article_info::cols::userid, 123).fetch();

// 3. 生成的逐字段 DSL —— 每个列一个方法，形如 {op}{Field}
//    op 有 eq nq bt be lt le like null notnull，每个都另有 or{op}{Field} 兄弟方法
model.eqUserid(123).fetch();
```

第一个参数永远是**列名**，不是 SQL 片段。不存在 `where("isopen=1")`：这个单参数重载已从
三个模板删除，因为它把 `op_type` 写成 `orm::wq::qb`，而没有任何构建器处理该值，条件会被
静默丢弃——一次 `update()` 或 `remove()` 因此可能扩大成全表语句。把操作符写进列名字符串
同样不是受支持写法：`where(" aid=", aid)` 通不过 `findcolpos` 检查，会置
`field is not table column`。列名与值请分开传（`where("isopen", 1).where("aid", aid)`）；
确实需要非常规的 SQL 形状时，走 §5.7 的 `orm::db_conn` 执行原文语句。

```cpp
// 比较助手（第一个参数可以是列名字符串，也可以是 cols 枚举）
model.whereBE("price", 100);      // >=
model.whereBT("price", 100);      // >
model.whereLT("price", 100);      // <
model.whereLE("price", 100);      // <=
model.whereNQ("price", 100);      // !=
model.whereNull("deleted_at");
model.whereNotNull("deleted_at");

// 多个条件默认按 AND 组合；whereAnd/whereOr 用于显式声明
model.where("userid", 123)
     .whereAnd("status", 1)
     .fetch();

// LIKE：% 通配符由 ORM 自动附加，调用方不要自己写
model.whereLike("title", "kw");        // LIKE '%kw%'
model.whereLikeLeft("title", "kw");    // LIKE '%kw'  （后缀匹配）
model.whereLikeRight("title", "kw");   // LIKE 'kw%'  （前缀匹配）
model.whereNotLike("title", "kw");     // NOT LIKE '%kw%'
// 以上四个都有 Or 前缀版本：whereOrLike / whereOrLikeLeft /
// whereOrLikeRight / whereOrNotLike。自己传 "kw%" 会得到 "kw%%"。

// 排序
model.desc("aid").fetch();             // ORDER BY aid DESC
model.asc("addtime").fetch();

// 分页
auto [minpage, maxpage, curpage, total] = model.page(1, 10);
model.fetch();

// 数量统计
unsigned int total = model.count();
```

### 5.4 括号分组（布尔逻辑）

`andsub()` 与 `orsub()` 都用来开启一个括号，区别只在于该括号如何与前面的条件相连
（`andsub` = `AND (`，`orsub` = `OR (`）；`endsub()` 用 `)` 收尾。它们自身不携带条件，
也不做任何自动配平 —— 括号未闭合就会生成非法 SQL。

```cpp
// WHERE isopen >= 1 AND aid > 0 AND (title LIKE '%kw%' OR content LIKE '%kw%')
art.beIsopen(1)
   .btAid(0)
   .andsub()
   .likeTitle("kw")
   .orlikeContent("kw")
   .endsub()
   .fetch();
```

同样三个方法也服务于预编译路径（§5.8），那里还可以使用通用的 `AND()` / `OR()` 写法：

```cpp
// WHERE id >= 1 AND id <= 200 AND (randomnumber > 5000 OR randomnumber < 200)
world.AND("id", orm::wq::be, 1)
     .AND("id", orm::wq::le, 200)
     .orsub()
     .AND("randomnumber", orm::wq::bt, 5000)
     .OR("randomnumber", orm::wq::lt, 200)
     .endsub();
world.exec_count();
```

`clearWhere()` 会清空累加的全部条件（含括号标记），使同一个模型对象可以复用于下一次查询。

### 5.5 ORM 高级查询

```cpp
// 限制结果
model.limit(10).fetch();
model.limit(5, 10).fetch();  // offset, count

// 选择字段
model.select("aid,title,addtime").fetch();

// 分组
model.group("category_id").fetch();

// 聚合函数（无 sum/avg/max/min 链式方法，用 select 别名 + 自定义结构体读取）
namespace orm::cust
{
    struct SumStruct : orm::Base<SumStruct>
    {
        long long total;
        ORM_NAMES(total);
    };
}
std::vector<orm::cust::SumStruct> result;
model.select("sum(price) as total");
model.fetch_to(result);    // result[0].total 为聚合结果

// IN 列表：值容器，或逗号分隔字符串
std::vector<long long> ids = {1, 2, 3};
model.whereIn("category_id", ids).fetch();
model.whereIn("category_id", "1,2,3").fetch();
model.whereNotIn("category_id", ids).fetch();
// 没有任何重载接受查询对象，所以子查询只能二选一：先查后绑
// （执行内层查询，把 id 收进 vector 再传给上面的 whereIn），
// 或者用 orm::db_conn 走原生 SQL（§5.7）。

// 连续批量操作：锁定连接复用（事务内使用）
model.lock_conn();
model.where("id", 1).fetch_one();
// ... 更多操作
model.unlock_conn();
```

### 5.6 ORM 数据操作

```cpp
auto model = orm::cms::Article();

// 新增
model.data.title = "标题";
model.data.content = "内容";
model.data.userid = 1;
auto [effect, newId] = model.save();   // 返回 [生效行数, 自增ID]，save 已自动 setPK

// 修改
model.data.title = "新标题";
model.where("aid", id).update("title");

// 批量修改
model.data.status = 1;
model.where("userid", 1).update("status");

// 删除
model.where("aid", id).remove();

// 字段自增/自减
model.where("aid", id).update_col("readnum", 1);     // readnum + 1
model.where("aid", id).update_col("stock", -1);      // stock - 1

// 字段替换
model.where("aid", id).replace_col("content", "old", "new");

// 按主键查询（setPK 用于记录 save() 后的自增ID，不能代替 where 查询）
model.where("aid", id).fetch_one();
```

**只更新你动过的列** —— `update_dirty()`，协程版 `async_update_dirty()`，以及预编译的
`exec_update_dirty()` / `async_exec_update_dirty()`。只有经生成的逐字段 setter 写入的列
才算"脏"；直接给 `model.data.x` 赋值不留痕迹，主键的 `setAid`、`setPK` 也永不标脏。

```cpp
auto model = orm::cms::Article();
model.setTopicid(7);            // topicid 标脏
model.setTitle("新标题");        // title 标脏
model.data.content = "静音";     // 不标脏 —— 这一列不会出现在 UPDATE 里

model.where("aid", id).update_dirty();       // UPDATE ... SET topicid=7,title='新标题'
model.clear_dirty();                         // 需要时手动清零
model.where("aid", id).exec_update_dirty();  // 同一组列，值改为参数绑定
```

守卫行为在文本与预编译两种形式下一致：没有脏列时，文本路径返回 `0` 且不发送任何语句，
`exec_update_dirty()` 则置 `iserror` 并返回 `(unsigned int)-1`。`wheresql` 为空时语句回退
到主键（`getPK() > 0`）；两者都没有时，文本路径返回 `0`，预编译路径拒绝发出语句。更新
成功后脏位自动清零。

**`save()` 是一个开关，不是插入** —— 当模型已经带着主键（`getPK() > 0`）且 `isrealnew`
为 false 时，`save()` 发出的是 `UPDATE ... WHERE pk = ...`，而不是 INSERT。需要新行时请用
`insert()` 或 `save(true)`。

```cpp
auto articles = orm::cms::Article();
articles.data.title = "标题";
auto [effect, newId] = articles.insert();   // 永远 INSERT，返回 [影响行数, 新 id]

articles.data.title = "第二条";
auto [e2, id2] = articles.save(true);       // 已有主键也仍然 INSERT

articles.data.title = "补丁";
auto [e3, _] = articles.save();             // UPDATE pk == id2 的那一行
```

`insert()` 有三个重载 —— `insert()` 取 `data` 作为行，`insert(row)` 取一个 `meta`，
`insert(rows)` 取 `std::vector<meta>` 并只发一条多行语句
`INSERT INTO t (全部列) VALUES (...),(...)`，所以你没填的那一列写入的是 `0` / `''`，
而不是列默认值。三个重载都返回 `[影响行数, 插入 id]` 并用该 id 调用 `setPK()`。一条多行
语句整体只有 **一个** id（MySQL 返回的是首个自增值），因此它最多只能定位其中一行 ——
如果调用方需要每一行的主键，请重新查询。

```cpp
auto rows = std::vector<orm::cms::article_info::meta>();
// 把每一行的每个列都填好，然后：
auto [batch_effect, batch_id] = articles.insert(rows);
```

`set_data(row)` 把一整个 `meta` 赋给 `data` 并返回调用链；`get()` 只是返回模型自身的
引用，方便把一长串链式调用拆成多条语句。这两个方法在本仓库里都还没有调用点 ——
现有控制器用的是 `model.data = row;`。

```cpp
orm::cms::Article one_row;                 // 别处查出来的
articles.set_data(one_row.data).insert();  // data = one_row.data，然后 INSERT
```

**整批 upsert / 替换 `record`** —— `update_batch(fieldname)` 发送的是由 `record`（不是
`data`）构造的一条语句，`record` 为空时直接返回 `0`。具体是哪条语句取决于方言和参数：

| 方言 | `update_batch("a,b")` | `update_batch("")` |
|---|---|---|
| MySQL | `INSERT ... ON DUPLICATE KEY UPDATE` | `REPLACE INTO` |
| PostgreSQL | `INSERT ... ON CONFLICT (pk) DO UPDATE SET` | `INSERT ... ON CONFLICT DO NOTHING` |
| SQLite | `INSERT ... ON CONFLICT(pk) DO UPDATE SET` | `REPLACE INTO` |

upsert 形式需要冲突目标，所以 `record` 里每一行的主键都必须有值。它没有协程版，也没有
预编译版 —— 预编译的批量插入请用 `exec_insert_batch()`。

**软删除** —— `soft_remove()` / `soft_remove("额外的 set 片段")` 执行的是生成器依据表列名
写出的那条 `UPDATE`，也就是给行打标记而不是删行。`paozhu_cli orm` 认下面这一组固定命名，
并把对应的赋值写进 `{table}_base.h::soft_remove_sql()`：

| 列名 | 生成的赋值 |
|---|---|
| `isdelete`、`deleted`、`isdeleted`、`is_deleted`、`is_delete`、`order_deleted`、`order_delete` | `col=1` |
| `deletetime`、`delete_time`、`deletedtime`、`deleted_time`、`deleted_at`、`delete_at` | `col=<unix 时间戳>`（选中该列时，这个值也会写回 `data`） |

没有这类列的表会得到一个空的函数体，此时 `soft_remove()` 置
`error_msg = "soft delete field empty."` 并返回 `0`，不发送任何语句。本仓库目前的任何一张
表都没有符合条件的列，所以这个 API 只对新建表有意义。和 `remove()` 一样，`wheresql` 为空
时回退到主键，并且 `group()/order()/limit()` 会被拼到语句尾部。

```cpp
articles.where("aid", id).soft_remove();   // UPDATE article SET isdelete=1 WHERE aid = id
```

### 5.7 原生 SQL 查询

当需要执行复杂 SQL 或 ORM 不支持的查询时，使用 `orm::db_conn` 独立数据库连接：

```cpp
#include "orm.h"
#include "orm_query.h"

// 1. 定义返回结果结构体，继承 orm::Base，使用 ORM_NAMES 宏注册字段
namespace orm::cust
{
    struct LocalusersqlStruct : orm::Base<LocalusersqlStruct>
    {
        unsigned int adminid;
        std::string name;
        std::string nickname;
        ORM_NAMES(adminid, name, nickname);
    };
}

// 2. 使用独立数据库连接执行 SQL
std::vector<orm::cust::LocalusersqlStruct> loaduser;
auto ulink = std::make_unique<orm::db_conn>("cms");  // cms 为 orm.conf 中的数据库标签

std::string sqlstring = "SELECT adminid,name,nickname FROM sysuser where 1 limit 1";

// 同步模式
ulink->query(sqlstring, loaduser);

// 协程模式（在 asio::awaitable 函数中）
co_await ulink->async_query(sqlstring, loaduser);

// 3. 读取结果
for (auto &row : loaduser)
{
    client << "adminid:" << row.adminid
           << ", name:" << row.name
           << "<br />";
}
```

**注意**: SQL 中 SELECT 的字段必须与返回结构体的字段一一对应。

**模型对象自己也能跑原生 SQL。** `orm::db_conn` 是一条独立连接；每个模型还自带一组入口，
复用模型自己的连接池与标签。

```cpp
auto articles = orm::cms::Article();

std::vector<orm::cust::ArticleTopSql> top;
unsigned int rows = articles.query("SELECT aid,title FROM article WHERE isopen=1 LIMIT 3", top);
// rows == top.size()；结构体需满足 orm::Base / ResultHasSetVal

co_await articles.async_query(sqlstring, top);          // 协程版

int affected = articles.edit_query("UPDATE article SET readnum=readnum+1 WHERE topicid=3");
co_await articles.async_edit_query(sql);                // 协程版
```

| 方法 | 返回 | 行为 |
|------|------|------|
| `query(sql, rows)` / `async_query(sql, rows)` | 取回的行数 | SELECT 路径；非 SELECT 会转交 `edit_query`，结果集被丢弃 |
| `edit_query(sql)` / `async_edit_query(sql)` | 影响行数 | 经写连接发送任意语句；不改动 `sqlstring` |
| `get_query()` | `std::string` | 上一条生成的语句，用于日志；`edit_query` 不会设置它 |
| `effect()` | `unsigned int` | 即 `effect_num` —— 写操作是影响行数，读操作是行数 |
| `commit_insert()` / `commit_insert(row)` | `std::string` | 只拼出 INSERT 文本，不发送 |
| `commit_update(fields)` / `commit_remove()` | `std::string` | 只拼出 UPDATE / DELETE 文本，不发送 |

读取用 `query(sql, rows)`，写入用 `edit_query(sql)`；不存在只吃一条语句的 `query(sql)`，
所以遗留的这种调用是编译错误，而不是一次悄悄返回 0 行的读取。任何一路都不要把请求数据
拼进语句 —— 只拼你自己代码产生的值，就像上面 `orm::db_conn` 的例子那样。

`commit_*` 是查询 DSL 表达不出来的语句的出口：它们拼出的文本与对应的
`save()`/`update()`/`remove()` 将要发送的完全相同，你可以查看、记日志，或者交给
`edit_query()` / 批量导入器。`commit_update()` 与 `commit_remove()` 拒绝生成无边界语句 ——
`wheresql` 为空时回退到主键，两者都没有时返回 `""`（`commit_update()` 还会置
`error_msg = "warning empty where sql!"`）。`commit_insert()` 没有这道守卫。

### 5.8 预编译语句查询

读与写的每个操作都有对应的预编译版本。构造条件的 API 完全共用，所以 `where()`、逐字段
DSL 以及 `andsub()/orsub()/endsub()` 都无需改动 —— 区别只在 SQL 送达服务端的方式：值以
参数形式绑定（MySQL 与 SQLite 用 `?`，PostgreSQL 用 `$1…$n`），不再转义进语句文本，因此
任何值都无法改变查询结构。

```cpp
auto world = orm::World();
world.AND("id", orm::wq::be, 1)
     .AND("id", orm::wq::le, 200)
     .orsub()
     .AND("randomnumber", orm::wq::bt, 5000)
     .OR("randomnumber", orm::wq::lt, 200)
     .endsub();

unsigned int total = world.exec_count();          // 对应 count()
world.select("id,randomnumber");
world.exec_fetch();                               // 结果行进入 world.record
std::vector<orm::cust::WorldRowStruct> rows;
world.exec_fetch_to(rows);                         // 结果行写入自定义结构体
world.exec_one();                                 // 单行结果落在 world.data
```

写操作命名规律相同，返回值也与文本路径一一对应：

```cpp
auto model = orm::cms::Testa();
model.data.parentid = 7;
model.data.content  = "prepared insert";
auto [effect, newid] = model.exec_insert();       // 与 save() 相同的 tuple

model.clear();
model.data.content = "prepared update";
model.where(orm::cms::testa_info::cols::id, newid);
model.exec_update("content");                     // 不带参数则更新全部已设字段
model.exec_update_fields({{"content", "x"}});     // 显式给出 名/值 对
model.exec_update_col("parentid", 3, '+');        // += 3   （'-' 为自减）
model.exec_replace_col("content", "old", "new");
model.exec_remove();

auto batch = orm::cms::Testa();
batch.record.push_back(row1);                     // meta 结构体
batch.record.push_back(row2);
auto [batch_effect, first_id] = batch.exec_insert_batch();
```

`exec_page(page, per_page, list_num)` 与 `exec_count()` 分别是 `page()` 和 `count()` 的
预编译版本。异步版本以 `async_exec_` 为前缀（§5.12）。凡是取值来自请求的参数，优先使用
`exec_` 系列；文本系列保留用于兼容，以及那些必须拼接进语句的 SQL 片段（`whereIn("id",
"1,2,3")` 这样的原始列表、`select()` 里的表达式）。

### 5.9 LEFT JOIN / INNER JOIN

`leftJoin<T>()` / `innerJoin<T>()` 接收被连接表的**模型类型**，随后用 `join*` 链配置连接
细节。被连接表的列会出现在外层 SELECT 中，结果行仍然落在主模型的 `record` 里。

```cpp
// SELECT world.id, world.randomnumber, fortune.message
// FROM world LEFT JOIN fortune ON fortune.id = world.id
auto world = orm::World();
world.btId(2).ltId(6);
world.select("id,randomnumber");
world.leftJoin<orm::Fortune>().joinOn("id", "id").joinSelect("message");
world.fetch();
```

`joinOn(join_col, main_col)` 用被连接表的一列与主表的一列比较；右边的列名也可以写 `cols`
枚举。

`joinWhere()` / `joinWhereOr()` / `joinDesc()` / `joinAsc()` / `joinGroup()` 只有在同时
设置了 `joinLimit()` 时才生效：没有 limit 时 ORM 生成的是普通的
`LEFT JOIN tbl ON …`，根本不会构造这些方法所填充的子查询。带 limit 时，被连接表会被包装成
`( SELECT … WHERE … ORDER BY … LIMIT n )`；再加上 `joinGroup()` 就变成
`ROW_NUMBER() OVER (PARTITION BY …)` 窗口，即每个分组取前 N 行：

```cpp
// 每个 topic 取最新 2 篇文章，只统计 user 1 拥有的 topic
auto topic = orm::cms::Topic();
topic.eqUserid(1);
topic.select("topicid,title AS topictitle");
topic.innerJoin<orm::cms::Article>()
     .joinOn("topicid", orm::cms::topic_info::cols::topicid)
     .joinSelect("aid, title AS arttitle")
     .joinWhere("userid", orm::wq::eq, 1)
     .joinGroup("topicid")
     .joinDesc("aid")
     .joinLimit(2);
topic.fetch();
```

`joinParAppend()` 用于追加更多的 `PARTITION BY` 列。

### 5.10 外键：oneXXX / manyXXX

`paozhu_cli orm` 会读取数据库中真实存在的单列外键，并为关系两端的每个相关表 `{Camel}`
生成 `one{Camel}` / `many{Camel}`。它们的函数体在生成的 `orm/{tag}/{table}_opsql.cpp`
里，所以该文件必须参与编译，否则访问器无法链接。

```cpp
// 子表侧：fk_child.parent_id -> fk_parent.id
auto child = orm::FkChild();
child.where(orm::fk_child_info::cols::id, 1);
child.fetch_one();                       // oneXXX 依赖 data 取键，所以先把行查出来

auto parent = child.oneFkParent();       // 以当前行的 parent_id 限定 FkParent
parent.fetch_one();

auto parent2 = child.oneFkParent(2);     // 用显式键替代当前行

auto decorated = orm::FkParent();        // void 重载：装饰传入对象，之后继续链式调用
child.oneFkParent(decorated);
decorated.select("id,name");

// 父表侧：record 中每一行对应的所有子行
auto parents = orm::FkParent();
parents.fetch();                         // manyXXX 依赖 record，所以先 fetch
auto children = parents.manyFkChild();   // WHERE parent_id IN (<record 中的 id>)
children.fetch();
```

`many{Camel}` 的 `IN (...)` 列表来自
`get_cols_vec<{self}_info::cols::pk>()`，也就是已加载行的主键集合。子表行应当一次查出后在
内存里分组，而不是逐行调用访问器 —— 逐行调用就是每行一次查询。`one{Camel}(obj&)` 与
`many{Camel}(obj&)` 重载返回 `void`，把限定条件加到你传入的模型上，这正是给关联追加
`select()` 或额外 `where()` 的做法。

### 5.11 跨方言使用：MySQL / PostgreSQL / SQLite

三种方言暴露的公共 API 完全一致 —— `mysqlorm.hpp`、`sqliteorm.hpp`、`postgresqlorm.hpp`
只在私有的转义与占位符助手上有差异。真正的差异在于你使用哪个生成的模型，因为模型在代码
生成时就绑定了它所属的标签，运行时无法切换。

```cpp
auto a = orm::Fortune();          // [default]   MySQL      命名空间 orm
auto b = orm::cms::Article();     // [cms]       MySQL      命名空间 orm::cms
auto c = orm::pg::Fortune();      // [pg]        PostgreSQL 命名空间 orm::pg
auto d = orm::lite::Fortune();    // [lite]      SQLite     命名空间 orm::lite
```

| | MySQL | PostgreSQL | SQLite |
|---|---|---|---|
| `orm.conf` 的 `dbtype` | `mysql` | `postgresql` | `sqlite` |
| `host` / `port` | 主机 / 3306 | 主机 / 5432 | 数据库文件路径 / 留空 |
| 模型命名空间 | `orm` 或 `orm::{tag}` | `orm::pg` | `orm::lite` |
| 生成目录 | `orm/{tag}/` | `orm/pg/` | `orm/lite/` |
| 预编译占位符 | `?` | `$1…$n` | `?` |

PostgreSQL 在自增主键为 0 时会写入 `DEFAULT`，交给序列赋值；SQLite 同样的情况绑定 `NULL`。
其余部分 —— `exec_*`、连接、外键访问器、`andsub()/orsub()/endsub()` —— 三者的行为完全
相同。

### 5.12 异步 ORM 操作

```cpp
// 在协程中使用异步 ORM
asio::awaitable<void> handle_request()
{
    auto model = orm::cms::Article();

    // 异步查询（文本路径）
    co_await model.async_fetch();
    co_await model.async_count();

    // 异步操作（文本路径）
    co_await model.async_save();
    co_await model.async_update();
    co_await model.async_remove();

    // 异步预编译路径
    co_await model.async_exec_count();
    co_await model.async_exec_fetch();
    co_await model.async_exec_insert();
}
```

### 5.13 结果缓存（`use_cache`，进程内）

`vendor/httpserver/include/orm_cache.hpp`，加上被复制进每个生成模型的 `use_cache` 代码块，
构成一个查询结果的读穿（read-through）缓存。它只是进程内存：没有 Redis、没有文件、没有序
列化，也没有任何 `orm.conf` / `server.conf` 配置项能开启或关闭它 —— 调用本身就是唯一的开
关。这段代码在三个方言模板中逐字节相同。

**存的是什么。** 缓存键是 `std::hash<std::string>{}(sqlstring)`，即完整拼好的语句，所以
select 列表、WHERE 文本、group/order 和 limit 全部参与构成键；DB 标签不参与，标签在模型构
造时就固定了。按值类型分成两个存储，彼此永不冲突：

| 存储 | 值 | 写入者 | 读取者 |
|------|-----|--------|--------|
| `meta` | `data`（单行） | `fetch_one()`、`get_one()`、`save_data_cache()` | `fetch_one()`、`get_one()`、`get_data_cache()` |
| `std::vector<meta>` | `record`（结果集） | `fetch()`、`fetch_append()`、`save_cache()` | `fetch()`、`fetch_append()`、`get_record_cache()` |

`fetch_row()` 把行存放在另外三个存储里 —— `std::vector<std::vector<std::string>>` 本身、
列名向量、以及 名 → 下标 的映射 —— 键的算法相同。这三个值类型都不携带模型信息，所以它们
是全进程共享的：与上面两个存储不同，它们不按 `meta` 类型分份。
`fetch_to()` / `fetch_one_to()` 永不走缓存，预编译路径上的任何方法也都不走。

**装填一次读穿。** `use_cache(int cache_time = 0)` 可链式调用，为下一次读取装填；
`cache_time` 是以秒为单位的 TTL，`0` 表示 *永不过期*。

```cpp
auto articles = orm::cms::Article();
articles.where("isopen", 1).where("topicid", 3).use_cache(60).fetch();  // record 存储，60 秒
```

语句发出之前，读取方法先查自己那个存储；命中就填充 `record` / `data`、解除装填并返回。
真正读完之后，只在 `if (iscache && exptime > 0)` 成立时回写，然后解除装填。严格按这段代码
理解，会得到三条结论：

- 使用默认 TTL 的 `use_cache()` 什么都不会存 —— 回写要求 `exptime > 0`。它的含义是“装填，
  然后什么都不做”，不是“永久缓存”。
- 命中时 `fetch()` / `async_fetch()` / `fetch_one()` / `get_one()` 返回 `0`，与结果集为空时
  的返回值完全一样，所以 `if (model.fetch_one() > 0)` 会把每一次缓存命中都当成失败。
- 没有别的东西会解除装填。`clear()` 和 `clearWhere()` 重置 `iscache` 但把 `exptime` 留在
  原处；`set_cache_state(bool)` 只设置这个标志。

**写操作从不失效。** `save()`、`update()`、`update_dirty()`、`update_col()`、
`replace_col()`、`remove()` 都不碰任何一个存储；`exec_*` / `async_exec_*` 路径既不查缓存也
不填缓存。凡是经这些方法、经 `orm::db_conn`、或被其他进程改动过的行，都会被一直返回，直到
TTL 到期。

**不存在就地刷新。** `model_meta_cache::save()` 做的是插入；若键已存在，它只重写存储项的
`exptime`，行数据原样保留（`cover_data` 默认 `false`，且没有任何包装函数传它）。对仍然有效
的键重新缓存，只延长寿命，不改变内容。删除 / 清理这一族 —— `clear_cache()`、两种形式的
`remove_cache()`、`remove_exptime_cache()` —— 现在同时覆盖 `meta` 和 `record` 两个存储，
两种结果集都能被丢掉；只要任一存储里还有这个键，`remove_cache()` 就返回 `true`。
`check_cache()` 与 `update_cache()` 仍是单存储的（分别对应 `meta` 与 `record`）。而
`fetch_row()` 用的三个存储以通用值类型为键，所有模型共用，模型上没有清理它们的入口，只能
靠 TTL 到期。

**未命中有副作用。** `get_data_cache()` 与 `get_record_cache()` 在键不存在时会写 `error_msg`
并调用 `unlock_conn()`。`iserror` 仍为 false，但在 `lock_conn()` 批次里做一次冷读取，会把连
接放回连接池。

**具名键。** 手动 API 接受调用方自选的 `std::string` 并自行哈希，这正是给“SQL 文本由 DSL
拼得不稳定”的查询做缓存的办法。返回存储值的两个访问器 —— `get_cache(name)` 与
`get_vector_cache(name)` —— 用抛异常表达未命中：先写 `error_msg`、调用 `unlock_conn()`，再
抛 `std::runtime_error("Not in cache")`，所以必须放在 `try` 里。而 `check_cache()`、
`get_record_cache()`、`update_cache()` 要的是 `std::size_t`，同一个名字请用
`std::hash<std::string>{}(name)` 自己哈希。

```cpp
const std::string key = "cms:article:top3:" + std::to_string(topicid);
try
{
    articles.record = articles.get_vector_cache(key);   // 未命中会抛异常
}
catch (const std::exception &)
{
    articles.clearWhere();
    articles.where("isopen", 1).desc("aid").limit(3).fetch();
    articles.save_vector_cache(key, articles.record, 60);
}
```

`save_vector_cache(name, rows, ttl)` 与 `save_cache(name, rows, ttl)` 是同一个调用。无参的
`remove_cache()` 以 `sqlstring` 为键，所以在该对象上先拼出语句才有意义。

**`isuse_cache()` 不是命中标志。** 无参时它表示装填是否还在。`isuse_cache(true)` 返回
`exptime == 0 && iscache == false`，而这个条件的含义随 TTL 模式翻转：在 `use_cache(60)` 下
真正读完之后，回写恰好把两者都清零，于是它返回 `true`；而命中时 `exptime` 仍是 `60`，于是
返回 `false`。在 `use_cache()`（TTL `0`）下又反过来。请把它理解成“装填已被消耗”，而不是
“这些行来自缓存”。

**冷缓存下的 `fetch_row()` 不再抛异常。** 它的读取要在同一个键上查三个存储，而
`model_meta_cache::get()` 在未命中或条目已过期时会抛，所以这三个 `get()` 现在写在 `try`
里：未命中、或者三个存储过期得不一致，都会落到真正的查询上。只有三者全部成功才算命中，因
此返回的元组不会带着行数据却配一个空的列名列表。`fetch_obj()` 两个方向都不碰缓存：每次都
从数据库读，也不回写，所以它不可能把 `fetch()` 读穿的那个 `record` 存储掏空。
`fetch_row()` 命中时消耗掉 `use_cache(ttl)` 的装填，未命中时保留它，于是真正读完之后的回写
照常执行；`fetch_obj()` 根本不看它，留在对象上的装填会活到下一次读取。上面那组具名键访问
器仍然按设计抛异常。

---

## 六、CLI 工具使用

### 6.1 视图编译

```bash
# 编译修改过的视图
./bin/paozhu_cli view

# 强制重新编译所有视图
./bin/paozhu_cli view force
```

### 6.2 ORM 生成

```bash
# 为一个数据库标签生成 ORM 代码（标签是 conf/orm.conf 中的节名）
./bin/paozhu_cli orm default
./bin/paozhu_cli orm cms
./bin/paozhu_cli orm pg
./bin/paozhu_cli orm lite
```

编辑 `vendor/httpserver/include/{mysql,sqlite,postgresql}orm.hpp` 或 `orm_common.h`
之后，必须为每一个标签重新执行该命令：生成的 `_opsql.h` 内嵌了模板类体的副本，未重新生成
的目录树会继续编译旧代码（§5.1）。

### 6.3 JSON 反射生成

```bash
# 扫描 libs 目录，为带 [//@reflect json to_json from_json] 注解的结构体生成反射代码
./bin/paozhu_cli json
```

### 6.4 数据库迁移

```bash
# 从 cms 迁移到 pg
./bin/paozhu_cli dbconver cms pg

# 强制迁移（覆盖已有表）
./bin/paozhu_cli dbconver cms pg force
```

### 6.5 SQL 导出

```bash
# 导出表结构
./bin/paozhu_cli dbtable cms ./schema.sql
./bin/paozhu_cli dbtable cms ./pg.sql -target=pg      # 导出为 PG 语法

# 导出结构+数据
./bin/paozhu_cli dbexport cms ./dump.sql
./bin/paozhu_cli dbexport cms ./mysql.sql -target=mysql
```

---

## 七、视图模板开发

### 7.1 视图模板位置

视图模板存放在 `view/` 目录下，按功能分子目录：
- `view/admin/` - 后台管理视图
- `view/home/` - 首页视图
- `view/login/` - 登录页视图

### 7.2 视图渲染

```cpp
// 控制器中渲染视图
client.view("admin/main");       // 渲染 view/admin/main.html
client.view("home/header");      // 渲染 view/home/header.html
```

### 7.3 视图变量传递

```cpp
// 控制器中设置视图变量
client.val["title"] = "页面标题";
client.val["user"]["name"] = "张三";
client.val["list"].set_array();
client.val["list"].push(item);

// 视图模板中使用
// <%c echo<<obj["title"].as_string(); %>      - 输出变量
// <%c echo<<obj["user"]["name"].as_string(); %> - 访问嵌套对象
// <%c ... %> - 嵌入 C++ 代码
```

### 7.4 视图编译为 C++

视图模板通过 CLI 工具编译为 `viewsrc/` 下的 C++ 源文件：

```bash
./bin/paozhu_cli view          # 增量编译
./bin/paozhu_cli view force    # 全量编译
```

---

## 八、核心 API 参考

### 8.1 请求对象 (httppeer)

| 功能 | 方法 | 说明 |
|------|------|------|
| GET 参数 | `client.get["key"]` | 获取 URL 查询参数 |
| POST 数据 | `client.post["key"]` | 获取 POST 数据 |
| 文件上传 | `client.files["field"]` | 获取上传文件信息 |
| Cookie | `client.cookie["name"]` | 获取 Cookie |
| 会话 | `client.session["key"]` | 会话读写 |
| 输出文本 | `client << "text"` | 直接输出字符串 |
| 渲染视图 | `client.view("path")` | 渲染视图模板 |
| 主题视图 | `client.theme_view("path")` | 渲染带主题前缀的视图 |
| 获取主题 | `client.get_theme()` | 获取当前域名的主题名 |
| 获取主题URL | `client.get_themeurl()` | 获取主题的静态资源URL前缀 |
| 重定向 | `client.goto_url("/path")` | 页面重定向 |
| JSON 输出 | `client.out_json()` | 输出 JSON 响应 |
| 客户端 IP | `client.client_ip` | 获取客户端 IP |
| SSL 检测 | `client.is_ssl()` | 是否 HTTPS |

### 8.2 ORM 链式查询方法

下表中的方法名定义在 `vendor/httpserver/include/{mysql,sqlite,postgresql}orm.hpp` 中；
三种方言提供同一组方法。

**条件** —— 第一个参数可以是列名（`"status"`），也可以是
`{tag}::{table}_info::cols` 枚举值。它不会是 SQL 片段；不存在
`where("isopen=1")` 这种重载（见 §5.3）。下面每个 `whereXxx` 都有一个 `whereOrXxx`
同参兄弟；它唯一的区别是与前一个条件用 `OR` 相连，这正是 `andsub()` / `orsub()`
括号内部需要的写法。

| 方法 | SQL | 示例 |
|------|-----|------|
| `where(col, val)` | `AND col = ?` | `.where("status", 1)` |
| `whereEQ` / `whereNE` | `=` / `!=` | `.whereNE("status", 0)` |
| `whereBT` / `whereBE` | `>` / `>=` | `.whereBE("price", 100)` |
| `whereLT` / `whereLE` | `<` / `<=` | `.whereLT("price", 100)` |
| `whereGT` / `whereGE` | `whereBT` / `whereBE` 的别名 | `.whereGE("price", 100)` |
| `whereNQ(col, val)` | `!=` | `.whereNQ("status", 0)` |
| `whereIn(col, vals)` | `IN (...)` —— 数组、`std::vector` 或原始 csv 串 | `.whereIn("id", std::vector<int>{1,2,3})` |
| `whereNotIn(col, vals)` | `NOT IN (...)` | `.whereNotIn("id", "4,5")` |
| `whereNull(col)` / `whereNotNull(col)` | `IS NULL` / `IS NOT NULL` | `.whereNull("deleted_at")` |
| `whereAnd(col, val)` | 显式 AND | `.whereAnd("type", 2)` |
| `whereOr(col, val)` | 显式 OR | `.whereOr("tag", 3)` |
| `whereOrBT` / `whereOrBE` / `whereOrLT` / `whereOrLE` / `whereOrNQ` | 用 `OR` 相连的 `>` / `>=` / `<` / `<=` / `!=` | `.whereOrLT("price", 100)` |
| `whereOrIn(col, vals)` / `whereOrNotIn(col, vals)` | `OR IN (...)` / `OR NOT IN (...)` | `.whereOrIn("id", ids)` |
| `whereOrNull(col)` / `whereOrNotNull(col)` | `OR IS NULL` / `OR IS NOT NULL` | `.whereOrNull("deleted_at")` |
| `where(col, "op", val)` | 运算符以字符串给出（`"="`、`"!="`、`">"`、`"<"`、`"LIKE"`、`"NOT LIKE"` …） | `.where("title", "NOT LIKE", "kw")` |

不存在 `whereOrGT` / `whereOrGE`，也不存在 `whereOrEQ` / `whereOrNE`：`whereOr(col, val)`
本身就是 `OR col = val`，而 `>` / `>=` 要写成 `whereOrBT` / `whereOrBE`。

**LIKE** —— `%` 通配符由 ORM 附加，调用方绝不要自己传。

| 方法 | SQL | 示例 |
|------|-----|------|
| `whereLike(col, v)` | `LIKE '%v%'` | `.whereLike("title", "kw")` |
| `whereLikeLeft(col, v)` | `LIKE '%v'`（后缀匹配） | `.whereLikeLeft("title", "kw")` |
| `whereLikeRight(col, v)` | `LIKE 'v%'`（前缀匹配） | `.whereLikeRight("title", "kw")` |
| `whereNotLike(col, v)` | `NOT LIKE '%v%'` | `.whereNotLike("title", "kw")` |
| `whereOrLike` / `whereOrLikeLeft` / `whereOrLikeRight` / `whereOrNotLike` | 同上四个，以 `OR` 连接 | `.whereOrLike("content", "kw")` |

**分组与修饰**

| 方法 | 说明 | 示例 |
|------|------|------|
| `andsub()` | 开启 `AND (` | 见 §5.4 |
| `orsub()` | 开启 `OR (` | 见 §5.4 |
| `endsub()` | 用 `)` 闭合括号 | 见 §5.4 |
| `clearWhere()` | 清空全部条件与括号标记 | |
| `clear()` | 清空条件、数据与已生成的 SQL | |
| `desc(col)` / `asc(col)` | `ORDER BY` | `.desc("aid")` |
| `order(col, "ASC")` / `order(col, "DESC")` | 由 `cols` 枚举加方向字符串拼出 `ORDER BY` —— 列必须是枚举，不存在 `order("aid", "DESC")` 这个重载 | `.order(orm::cms::article_info::cols::aid, "DESC")` |
| `order("expr")` / `asc("expr")` / `desc("expr")` | 参数原样拼进 `ORDER BY` | `.order("aid DESC, title ASC")` |
| `asc()` / `desc()` | 按主键 `ORDER BY` | `.desc()` |
| `limit(n)` / `limit(offset, n)` | `LIMIT` | `.limit(0, 20)` |
| `select(fields)` | 字段列表，允许表达式与别名 | `.select("id,title")` |
| `group(col)` | `GROUP BY` | `.group("topicid")` |
| `having(cond)` | `HAVING` | `.having("count(*) > 1")` |
| `lock_conn()` / `unlock_conn()` | 批量操作复用同一个连接 | §5.5 |

**执行 —— 文本路径**（值被转义进语句）

| 方法 | 返回值 |
|------|--------|
| `fetch()` / `fetch_one()` | 生效行数；结果在 `record`，首行在 `data` |
| `fetch_append()` | 把结果追加进 `record` |
| `fetch_to(vec)` / `fetch_one_to(obj)` | 写入自定义 `orm::Base` 结构体 |
| `count()` | 匹配行数 |
| `page(page, per_page, list_num)` | `[bar_min, bar_max, current, total]` |
| `save()` / `update(fields)` / `remove()` | `[effect, id]` / 生效行数 / 生效行数 |
| `insert()` / `insert(row)` / `insert(rows)` | `[effect, id]`；永远执行 INSERT，`rows` 是一条多行语句（§5.6） |
| `update_batch(fields)` | 生效行数；由 `record` 构造的一条 upsert（参数为空时是 replace）（§5.6） |
| `soft_remove()` / `soft_remove(extra)` | 生效行数；执行生成的“标记列”UPDATE，表没有该列时返回 `0`（§5.6） |
| `update_col(col, n, sign)` | 自增（`'+'`）或自减（`'-'`） |
| `replace_col(col, old, new)` | 在 UPDATE 中使用 `REPLACE()` |
| `update_dirty()` | 生效行数；没有脏列时返回 `0` 且不发送语句（§5.6） |

**执行 —— 预编译路径**（§5.8）：上述每个读取和单行写方法都有 `exec_` 版本，协程形式再加
`async_` 前缀。没有预编译对应版本的是：`insert(rows)`（批量场景由 `exec_insert_batch()`
覆盖）、`update_batch()`、`soft_remove()`，以及 `query()`/`edit_query()` 这一族。

| 文本 | 预编译 | 异步预编译 |
|------|--------|------------|
| `count()` | `exec_count()` | `async_exec_count()` |
| `page(...)` | `exec_page(...)` | `async_exec_page(...)` |
| `fetch()` | `exec_fetch()` / `exec_fetch_to(v)` | `async_exec_fetch()` / `async_exec_fetch_to(v)` |
| `fetch_one()` | `exec_one()` / `exec_one_to(v)` | `async_exec_one()` / `async_exec_one_to(v)` |
| `fetch_append()` | `exec_fetch_append()` | `async_exec_fetch_append()` |
| `save()` | `exec_insert()` | `async_exec_insert()` |
| 批量新增 | `exec_insert_batch()` | `async_exec_insert_batch()` |
| `update(...)` | `exec_update()` / `exec_update(fields)` / `exec_update_fields(pairs)` / `exec_update_dirty()` | `async_exec_update*()`（无 `async_exec_update_fields`） |
| `update_col(...)` | `exec_update_col(...)` | `async_exec_update_col(...)` |
| `replace_col(...)` | `exec_replace_col(...)` | `async_exec_replace_col(...)` |
| `remove()` | `exec_remove()` | `async_exec_remove()` |

**连接与外键**（§5.9、§5.10）

| 方法 | 说明 |
|------|------|
| `leftJoin<T>()` / `innerJoin<T>()` | 以模型类型 `T` 开始一个连接 |
| `joinOn(join_col, main_col)` | `ON` 条件，列与列比较 |
| `joinSelect(fields)` | 需要返回的被连接表字段 |
| `joinWhere(col, op, val)` / `joinWhereOr(...)` | 被连接子查询内的条件（需要 `joinLimit`） |
| `joinAsc(col)` / `joinDesc(col)` | 被连接子查询内的排序（需要 `joinLimit`） |
| `joinGroup(col)` / `joinParAppend(col)` | `PARTITION BY` 列（需要 `joinLimit`） |
| `joinLimit(n)` | 切换到子查询形式；配合 `joinGroup` 即每组取前 N 行 |
| `one{Camel}()` / `one{Camel}(id)` / `one{Camel}(obj&)` | 生成的外键：单条关联行 |
| `many{Camel}()` / `many{Camel}(obj&)` | 生成的外键：当前结果集的全部关联行 |
| `get_cols_vec<{table}_info::cols::col>()` | 取 `record` 中该列的值集合，即 `many{Camel}` 用来限定的集合。按表生成于 `_base.h`；另一个重载接受 `bool(const value&)` 过滤回调 |

**结果缓存**（§5.13）—— “存储”一列指明该方法落在两个 map 的哪一个上：`meta` 存单行
（`data`），`vector` 存结果集（`record`）。

| 方法 | 存储 | 说明 |
|------|------|------|
| `use_cache(ttl = 0)` | — | 可链式调用；为下一次文本路径读取装填 |
| `isuse_cache(bydate = false)` | — | 装填标志，不是命中指示器 |
| `set_cache_state(bool)` | — | 强制设置 `iscache`，不动 `exptime` |
| `save_data_cache(ttl = 0)` | `meta` | 以 `hash(sqlstring)` 为键存 `data` |
| `save_data_cache(name, row, ttl = 0)` | `meta` | 调用方指定的键 |
| `save_cache(ttl = 0)` | `vector` | 以 `hash(sqlstring)` 为键存 `record` |
| `save_cache(key, rows, ttl = 0)` / `save_cache(name, rows, ttl = 0)` / `save_vector_cache(name, rows, ttl = 0)` | `vector` | 调用方指定的键 |
| `save_cache(name, row, ttl = 0)` | `meta` | 用名字存单行 |
| `get_data_cache(key)` / `get_record_cache(key)` | `meta` / `vector` | 填充 `data` / `record`，返回 `bool`；未命中会写 `error_msg` 并调用 `unlock_conn()` |
| `get_cache(name)` / `get_vector_cache(name)` | `meta` / `vector` | 返回存储的值；未命中**抛** `std::runtime_error` |
| `check_cache(key)` | `meta` | `-1` 不存在，`0` 永不过期，其余为剩余秒数 |
| `update_cache(ttl = 0)` / `update_cache(key, ttl)` | `vector` | 只改 TTL；键不存在返回 `-1` |
| `remove_cache()` / `remove_cache(key)` | `meta` + `record` | 无参版本以 `sqlstring` 为键；任一存储里有这个键就返回 `true` |
| `clear_cache()` / `remove_exptime_cache()` | `meta` + `record` | 整个存储 / TTL 已到期的条目 |

读穿逻辑内置在 `fetch()`、`fetch_append()`、`fetch_one()`、`get_one()` 及其 `async_` 兄弟
里，另有 `fetch_row()`；`fetch_obj()` 既不读也不写，任何 `exec_*` 方法都不读写缓存。

**无类型结果形态** —— 用于类型化的 `meta` 表达不出的列组合。它们读取与 `fetch()` 相同的
`select()/where()/limit()` 状态。

| 方法 | 返回 | 说明 |
|------|------|------|
| `fetch_obj()` | `std::vector<std::map<std::string, std::string>>` | 一行一个 map；所有值都是字符串，`NULL` 变成 `""`，因此 `0` 与空列无法区分 |
| `fetch_row()` | `tuple<vector<string>, map<string, unsigned int>, vector<vector<string>>>` | 列名、名 → 下标、以及行数据；按下标访问可以保留重复列名 |
| `fetch_json()` / `async_fetch_json()` | `http::obj_val` | 一个由字符串值对象组成的数组，可直接输出 |
| `get_one(id)` / `async_get_one(id)` | `long long` | 按主键取一行进 `data`；完全忽略 `wheresql`，所以链式 `where()` 会被静默丢弃 |

两种 `get_one()` 都返回读到的行数，所以 `if (m.get_one(id) > 0)` 就意味着这一行已经落进
`data`。唯一的例外是结果缓存：命中时每一个读穿方法（包括 `get_one()`）都返回 `0`，所以可
能用到缓存时，判断依据请用 `data` 而不是返回值。

**表名与连接切换**

| 方法 | 说明 |
|------|------|
| `set_table(name)` | 覆盖后续语句使用的表名；空参数被忽略 |
| `reset_table()` | 恢复为 `org_tablename` |
| `switchDB(tag)` | 把模型重新绑定到另一个 `orm.conf` 标签，且必须**同引擎** —— MySQL 模型会拒绝 `postgresql`/`sqlite` 标签。成功时 `dbtag` 跟着切换，后续语句与报错文本指的就是真正在用的那个池 |
| `resetDB()` | 反向操作：重新绑定到 `B_BASE::_rmstag`，也就是这张表**生成时**所用的标签，从而丢掉 `switchDB()` 指过去的目标。可链式，引擎校验与 `switchDB()` 相同 |
| `get_db_type()` | 模型编译期的引擎：`DB_TYPE::MYSQL` / `POSTGRESQL` / `SQLITE` —— 属于生成出来的类，而不是连接 |
| `data` | 当前行（`{table}_info::meta`） |
| `record` | 已加载的行集（`std::vector<meta>`） |
| `error_msg` | 最近一次错误文本 |
| `iserror` | 一旦置位就锁住 —— 它为真时，大多数读写方法不接触数据库直接返回 `0`。只有 `clear()` / `clearWhere()` 会复位（两者同时清空 `error_msg`） |

`switchDB()` 只改变模型借用连接的池，这正是同引擎分库分表的入口。它是返回 `M_MODEL &` 的
链式方法，`model.switchDB("cms").where("aid", id).fetch()` 就是预期的写法。被拒绝的切换会
清空连接并置 `iserror`，而链式调用仍然落在这个对象上 —— 后面的读取不接触数据库直接返回
`0`，所以要在链式调用结束后检查 `error_msg`，而不是在 `switchDB()` 之后。两种拒绝报错点名
的都是**被请求的那个标签**：标签不存在是 `conn_pool not found <标签>`，标签在另一个引擎上
是 `conn_pool db type error <标签>`。

`resetDB()` 就是配对的回程 —— `model.switchDB("cms").fetch()` 之后再
`model.resetDB().fetch()`，同一个对象先读分片、再回到本表自己的库。它只是重新绑定，不是
修复：连接池接回来了，但和其它方法一样，**不会**复位已经锁住的 `iserror`。所以在一次
**失败的** `switchDB()` 之后，`resetDB()` 会恢复 `conn_obj` 与 `dbtag`，模型却仍然拒绝读取
—— 要调用 `clear()`（或 `clearWhere()`）才能解开锁，且 `clear()` 连 `error_msg` 一起清空，
所以先读它再清。

---

## 九、项目安全规范

### 9.1 路径安全
- 路径验证必须防止路径穿越攻击
- 使用 `std::filesystem::canonical` 验证路径
- 临时文件不得有 group/others 权限

### 9.2 数据库安全
- 使用 ORM 参数化查询防止 SQL 注入
- 敏感数据在配置文件中加密存储
- 数据库连接使用最小权限原则

### 9.3 会话安全
- 会话 ID 使用安全随机生成
- 敏感操作需验证会话状态
- 支持 Session 过期时间配置

### 9.4 SSL/TLS
- 必须启用 OCSP Stapling
- 使用 OpenSSL 3.0+
- 证书自动续期（ACME）

---

## 十、开发工作流

### 10.1 新增控制器

1. 在 `controller/src/` 创建实现文件（如 `newfeature.cpp`），并用 `//@urlpath` 注释注册路由。
2. 对应的 `controller/include/newfeature.h` 可以不写：CMake 会为每个缺少头文件的
   `controller/src/*.cpp` 自动 touch 一个空头文件。
3. 不需要改 CMakeLists.txt —— `controller/src/*.cpp` 由
   `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)` 收集，直接重新编译即可纳入新文件。

### 10.2 新增数据表模型

1. 在数据库创建数据表
2. 运行 `./bin/paozhu_cli orm <tag>` 生成 ORM 代码
3. 在 `models/<tag>/ModelName.cpp` 中实现模型业务逻辑
4. 在控制器中使用 `orm::<tag>::ModelName()` 访问数据（默认标签为 `orm::ModelName()`）

### 10.3 新增视图

1. 在 `view/` 下创建 HTML 模板
2. 使用 `<%c echo<<obj["var"].to_string(); %>` 输出变量
3. 运行 `./bin/paozhu_cli view` 编译视图
4. 在控制器中通过 `client.view()` 渲染

### 10.4 构建项目

```bash
# CMake 配置
mkdir build && cd build
cmake ..                 # 需要构建 [lite] 标签时加 -DENABLE_SQLITE=ON（见十二）

# 编译
make -j$(nproc)

# 运行
./bin/paozhu_cli         # CLI 工具
./bin/paozhu             # Web 服务器
```

---

## 十一、常见问题

**Q: 如何添加新的数据库连接？**  
在 `conf/orm.conf` 中添加新节，运行 `./bin/paozhu_cli orm <tag>` 生成其模型，然后在代码中
使用对应的命名空间（`orm::<tag>::ModelName`）。

**Q: ORM 代码可以手动修改吗？**  
`_base.h` 和 `_opsql.h` 是自动生成的，不应手动修改。业务逻辑应在 `models/` 下的模型类中
实现。若要改动的是共享行为，那就是修改 `vendor/httpserver/include/` 下的模板，而这要求
随后重新生成所有标签 —— 见 §5.1。

**Q: 如何在 PostgreSQL 和 MySQL 之间切换？**  
不能在运行时切换。每个生成的模型在代码生成阶段就绑定了它的 `orm.conf` 标签：`orm::Fortune`
永远只连 `[default]` 这个 MySQL 节，而 `orm::pg::Fortune` 与 `orm::lite::Fortune` 是另外
生成的类，分别绑定 PostgreSQL 与 SQLite。要换引擎，就用 `./bin/paozhu_cli orm <tag>` 生成
该标签并使用其命名空间；`orm.conf` 里的 `dbtype` 只决定*新*标签使用哪种客户端，
`./bin/paozhu_cli dbconver <from> <to>` 负责搬运数据。

**Q: DSL 覆盖不到的谓词，怎么写进 `where()`？**  
不能把 SQL 片段交给 `where()`。它的第一个参数是列名，操作符单独传（
`where("price", orm::wq::be, 100)`，或 `where("price", ">=", 100)`）。DSL 覆盖不到的东西
——全文检索、`EXISTS`、跨列表达式——请把语句原文交给 `orm::db_conn`（§5.7）配合自定义
`orm::Base` 结构体执行，并且只拼接自己代码产生的值，不要拼接请求数据。

**Q: 视图模板支持哪些特殊标签？**  
- `<%c echo<<obj["variable"].to_string(); %>` - 变量输出
- `<%c echo<<obj["variable"].as_string(); %>` - 输出文本（默认转义，无 raw 语法）
- `<%c cpp_code %>` - 嵌入 C++ 代码
- `<%c include_sub("home/header",obj); %>` - 包含子模板

**Q: 如何调试 SQL 查询？**  
在 `server.conf` 中设置 `debug_enable = 1`，ORM 会自动记录生成的 SQL 语句。

---

## 十二、技术栈依赖

### 核心依赖
- **C++20** - 编程语言标准
- **Asio** - 异步网络库
- **OpenSSL 3.0+** - SSL/TLS 加密

### 内置数据库客户端
- 框架自行集成 MySQL、PostgreSQL 与 SQLite 客户端，无需额外安装 MySQL Connector/C++、libpq，也不必手工链接 SQLite
- 支持 **MySQL 8.0.4 以上版本**（支持 `caching_sha2_password` 认证插件）
- 支持 **MariaDB 12.1+**（12.1 版本开始支持 `caching_sha2_password` 认证插件，与 MySQL 8.0 默认认证方式兼容）
- **SQLite** 由 CMake 选项 `ENABLE_SQLITE` 控制（默认 `OFF`）。打开后才能构建 `[lite]` 标签：
  CMake 会加入 `sqlite_conn.cpp` 并定位库 —— vcpkg 分支用 `find_package(SQLite3 REQUIRED)`，
  系统分支用 `find_path`/`find_library`。`vcpkg.json` / `xmake.lua` 已把 `sqlite3` 列为依赖。
- `ENABLE_SQLITE=OFF` 时，`vendor/httpserver/src/sqlite_conn_stub.cpp` 依然参与编译（它一直在
  源文件列表中），但生效的是其中 `#ifndef ENABLE_SQLITE` 的存根实现，因此 `dbtype = sqlite`
  的节会在连接时报
  `SQLite support not compiled. Rebuild with -DENABLE_SQLITE=ON`，而不是崩溃。
- 同时支持 **C++20 协程**（异步）和**同步**两种调用方式

### 可选依赖
- **Redis** - 会话存储
- **FastCGI** - PHP 支持
- **zlib** - 压缩支持

### 内部模块
- `vendor/httpserver/` - HTTP 服务器核心
- `vendor/httpcli/` - CLI 代码生成器
- `vendor/pzexcel/` - Excel 处理
- `vendor/pzword/` - Word 处理
- `vendor/pzzip/` - ZIP 压缩
- `vendor/pzimage/` - 图片处理
- `vendor/pzcharts/` - SVG 图表
- `vendor/webpdf/` - PDF 生成
- `vendor/sms/` - 短信服务
- `vendor/webpay/` - 支付集成

---

## 十三、项目干净初始化

从 GitHub clone 回 Paozhu 框架后，通常需要清理示例业务代码，保留框架核心和 hello 注解作为脚手架起点。以下是四步清理流程。

### 第一步：清空 common/ 注册文件

以下 6 个文件需要清理 —— 保留 `#include "httppeer.h"`，删除其他所有 `#include`，清空 `_initauto_control_httpmethodregto()` 等注册函数的函数体（只保留空函数签名）：

| 文件 | 作用 |
|------|------|
| `common/autocontrolmethod.hpp` | 控制器方法注册（由构建系统从 controller/src 抽取） |
| `common/autorestfulpaths.hpp` | RESTful 路径注册 |
| `common/reghttpmethod.hpp` | HTTP 方法注册 |
| `common/reghttpmethod_pre.hpp` | HTTP 前置方法注册 |
| `common/sockets_method_reg.hpp` | Socket 方法注册 |
| `common/websockets_method_reg.hpp` | WebSocket 方法注册 |

清理后每个文件形如：

```cpp
#include "httppeer.h"

void _initauto_control_httpmethodregto(std::map<std::string, regmethold_t> &methodcallback)
{
    // 空实现
}
// 其他注册函数同样清空
```

### 第二步：删除 ORM 层

```bash
rm -rf models/* schema/* orm/*
```

这三个目录存放数据库模型和 ORM 生成代码，从模板项目不需要。将来启用数据库时用 `bin/paozhu_cli orm <tag>` 重新生成（见 §5.1）。

### 第三步：删除视图文件

```bash
rm -rf view/* viewsrc/*
```

`view/` 是 HTML 模板，`viewsrc/` 是模板编译产物（C++ 源码）。两者都是示例代码，可安全清空。

### 第四步：删除控制器（保留 testhello）

```bash
rm -rf controller/include/*
find controller/src -type f ! -name 'testhello.cpp' -delete
```

保留的 `controller/src/testhello.cpp` 包含 `//@urlpath(null,hello)` 注解，是 hello world 路由的唯一入口。编译后即可通过 `http://127.0.0.1/hello` 访问。

### 清理完成

以上四步完成后，重新编译项目即可得到一个干净的脚手架：

```bash
cd build && cmake .. && make -j$(sysctl -n hw.ncpu)
```

运行 `./bin/paozhu` 后访问 `http://127.0.0.1/hello`，应返回示例中的 "Hello world! 🧨 Paozhu c++ web framework"，说明框架核心正常。

---

*最近更新: 2026-09-18*
