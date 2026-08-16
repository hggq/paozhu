# Paozhu C++ Web Framework - AI 开发指南

## 一、项目概述

Paozhu (炮竹) 是一个高性能的 C++20 Web 框架，基于 Asio 异步网络库构建，支持 HTTP/HTTPS/HTTP2 协议。框架采用 MVC 架构，内置 ORM、模板引擎、协程支持和丰富的功能模块。

### 核心特性
- **异步 IO**: 基于 Asio 的异步非阻塞网络模型
- **协程支持**: C++20 co_await 关键字编写异步代码
- **HTTP/HTTP2**: 原生支持 HTTP/1.1 和 HTTP/2 协议
- **ORM**: 内置 ORM，支持 MySQL 和 PostgreSQL
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
│   ├── gcc.conf                   # 编译器配置
│   ├── controlbuild.conf          # 控制器构建配置
│   └── ...                        # 其他配置和 SQL 文件
│
├── controller/                    # 控制器层
│   ├── include/                   # 控制器头文件
│   │   ├── admin/                 # 后台管理控制器
│   │   └── test*.h               # 功能测试控制器
│   └── src/                       # 控制器实现
│       ├── admin/                 # 后台管理实现
│       └── test*.cpp              # 功能测试实现
│
├── models/                        # 数据模型层
│   ├── cms/                       # CMS 相关模型
│   ├── ph/                        # PostgreSQL 测试模型
│   └── include/                   # 模型头文件
│
├── orm/                           # ORM 层（自动生成）
│   ├── cms/include/               # CMS ORM 操作类
│   │   ├── *_base.h               # 基础字段定义（自动生成）
│   │   └── *_opsql.h              # SQL 操作中间层（自动生成）
│   ├── include/                   # 通用 ORM 文件
│   └── orm.h                      # ORM 统一入口
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
└── AI_RULES.md                    # 本文档
```

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
[section_name]     ; 数据库标识（如 default, cms, ph）
type = main        ; main（主库）或 second（从库）
host = 127.0.0.1   ; 数据库主机
port = 3306        ; 端口（MySQL: 3306, PG: 5432）
dbname = database_name  ; 数据库名
user = username    ; 用户名
password = pass    ; 密码
pretable =         ; 表前缀
maxpool = 5        ; 最大连接池
dbtype = mysql     ; mysql 或 postgresql
charset = utf8mb4  ; 字符集
#ssl = ON          ; 启用 SSL
#sslverify = ON    ; 校验证书链
#sslhost =         ; 证书域名
```

**数据库标识约定**:
- `default` - 基准测试数据库
- `cms` - CMS 系统数据库
- `docs` - 文档数据库
- `ph` - PostgreSQL 测试库

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

| 同步方法 | 异步方法 | 说明 |
|----------|----------|------|
| `fetch()` | `async_fetch()` | 查询所有匹配记录 |
| `fetch_one()` | `async_fetch_one()` | 查询单条记录 |
| `fetch_append()` | `async_fetch_append()` | 查询并追加到现有结果集 |
| `count()` | `async_count()` | 统计匹配记录数 |
| `save()` | `async_save()` | 新增记录 |
| `update()` | `async_update()` | 更新记录 |
| `remove()` | `async_remove()` | 删除记录 |

**注意**: 同步控制器（返回 `std::string`）和协程控制器（返回 `asio::awaitable<std::string>`）可以在同一项目中混用。框架会根据函数签名自动选择正确的执行路径。

---

## 五、ORM 使用指南

### 5.1 生成 ORM 代码

使用 CLI 工具从数据库自动生成 ORM 代码：

```bash
# 生成指定数据库的 ORM
./bin/paozhu_cli orm cms      # 从 [cms] 数据库配置生成
./bin/paozhu_cli orm default  # 从 [default] 数据库配置生成
./bin/paozhu_cli orm ph       # 从 [ph] PostgreSQL 配置生成
```

### 5.2 模型文件组成

每个表对应三个文件：
- `models/{db}/ModelName.cpp` - 模型实现
- `orm/{db}/include/modelname_base.h` - 字段基础定义（自动生成，不手动修改）
- `orm/{db}/include/modelname_opsql.h` - SQL 操作层（自动生成，不手动修改）

### 5.3 ORM 基本查询

```cpp
#include "orm.h"

// 获取所有记录
auto model = orm::cms::Article();
model.fetch();

// 条件查询
model.where("userid", 123).fetch();
model.whereLike("title", "keyword");       // LIKE 查询
model.where("status", 1);                  // 等于
model.whereBT("price", 100);               // 大于 >
model.whereBE("price", 100);               // 大于等于 >=

// 多条件
model.where("userid", 123)
     .whereAnd("status", 1)
     .fetch();

// 排序
model.desc("aid").fetch();           // 降序
model.asc("addtime").fetch();        // 升序

// 分页
auto [minpage, maxpage, curpage, total] = model.page(1, 10);
model.fetch();

// 数量统计
unsigned int total = model.count();

// 聚合
unsigned int total = model.where("status", 1).count();
```

### 5.4 ORM 高级查询

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

// 子查询
model.whereIn("category_id", subQuery);

// 连续批量操作：锁定连接复用（事务内使用）
model.lock_conn();
model.where("id", 1).fetch_one();
// ... 更多操作
model.unlock_conn();
```

### 5.5 ORM 数据操作

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

### 5.6 原生 SQL 查询

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

### 5.7 PostgreSQL 支持

```cpp
// PostgreSQL 模型使用独立的命名空间
auto model = orm::ph::Forture();   // ph 命名空间对应 orm.conf 中的 [ph] 配置
model.fetch();

// 注意：MySQL 模型(orm::cms)和 PostgreSQL 模型(orm::ph) 是独立的
// 已生成的 MySQL 模型内部固化了 MySQL 连接，不能切换到 PostgreSQL
// 如需使用 PostgreSQL，必须使用 orm::ph 命名空间下的模型
```

### 5.8 异步 ORM 操作

```cpp
// 在协程中使用异步 ORM
asio::awaitable<void> handle_request()
{
    auto model = orm::cms::Article();
    
    // 异步查询
    co_await model.async_fetch();
    co_await model.async_count();
    
    // 异步操作
    co_await model.async_save();
    co_await model.async_update();
    co_await model.async_remove();
}
```

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
# 生成 ORM 代码
./bin/paozhu_cli orm cms
./bin/paozhu_cli orm default
./bin/paozhu_cli orm ph
```

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

| 方法 | 说明 | 示例 |
|------|------|------|
| `where(col, val)` | 添加 WHERE 条件 | `.where("status", 1)` |
| `whereAnd(col, val)` | AND 条件 | `.whereAnd("type", 2)` |
| `whereOr(col, val)` | OR 条件 | `.whereOr("tag", 3)` |
| `whereLike(col, val)` | LIKE 模糊查询 | `.whereLike("title","kw")` |
| `whereOrLike(col, val)` | OR LIKE 查询 | `.whereOrLike("title","kw")` |
| `whereBT(col, val)` | 大于 `>` | `.whereBT("price", 100)` |
| `whereBE(col, val)` | 大于等于 `>=` | `.whereBE("price", 100)` |
| `whereLT(col, val)` | 小于 `<` | `.whereLT("price", 100)` |
| `whereLE(col, val)` | 小于等于 `<=` | `.whereLE("price", 100)` |
| `whereIn(col, vals)` | IN 查询 | `.whereIn("id", "1,2,3")` |
| `whereNotIn(col, vals)` | NOT IN | `.whereNotIn("id", "4,5")` |
| `desc(col)` | 降序排序 | `.desc("id")` |
| `asc(col)` | 升序排序 | `.asc("time")` |
| `limit(n)` | 限制数量 | `.limit(10)` |
| `limit(offset, n)` | 分页限制 | `.limit(0, 20)` |
| `select(fields)` | 选择字段 | `.select("id,name")` |
| `group(col)` | 分组 | `.group("category")` |
| `fetch()` | 执行查询 | 获取结果集 |
| `fetch_one()` | 单条查询 | 获取单条记录 |
| `count()` | 计数 | 返回记录数 |
| `page(page, per)` | 分页计算 | 返回分页信息 |

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

1. 在 `controller/include/` 创建头文件（如 `newfeature.h`）
2. 在 `controller/src/` 创建实现文件（如 `newfeature.cpp`）
3. 使用 `//@urlpath` 注册路由
4. 在 CMakeLists.txt 中添加源文件
5. 重新编译项目

### 10.2 新增数据表模型

1. 在数据库创建数据表
2. 运行 `./bin/paozhu_cli orm <tag>` 生成 ORM 代码
3. 在 `models/<db>/` 中实现模型业务逻辑
4. 在控制器中使用 `orm::<db>::ModelName()` 访问数据

### 10.3 新增视图

1. 在 `view/` 下创建 HTML 模板
2. 使用 `<%c echo<<obj["var"].to_string(); %>` 输出变量
3. 运行 `./bin/paozhu_cli view` 编译视图
4. 在控制器中通过 `client.view()` 渲染

### 10.4 构建项目

```bash
# CMake 配置
mkdir build && cd build
cmake ..

# 编译
make -j$(nproc)

# 运行
./bin/paozhu_cli         # CLI 工具
./bin/paozhu             # Web 服务器
```

---

## 十一、常见问题

### Q: 如何添加新的数据库连接？
在 `conf/orm.conf` 中添加新的 section，然后在代码中使用对应的命名空间。

### Q: ORM 代码可以手动修改吗？
`_base.h` 和 `_opsql.h` 文件是自动生成的，不应手动修改。业务逻辑应在 `models/` 目录下的模型类中实现。

### Q: 如何在 PostgreSQL 和 MySQL 之间切换？
修改 `orm.conf` 中的 `dbtype` 参数，或使用 CLI 工具的 `dbconver` 功能迁移数据。

### Q: 视图模板支持哪些特殊标签？
- `<%c echo<<obj["variable"].to_string(); %>` - 变量输出
- `<%c echo<<obj["variable"].as_string(); %>` - 输出文本（默认转义，无 raw 语法）
- `<%c cpp_code %>` - 嵌入 C++ 代码
- `<%c include_sub("home/header",obj); %>` - 包含子模板

### Q: 如何调试 SQL 查询？
在 `server.conf` 中设置 `debug_enable = 1`，ORM 会自动记录生成的 SQL 语句。

---

## 十二、技术栈依赖

### 核心依赖
- **C++20** - 编程语言标准
- **Asio** - 异步网络库
- **OpenSSL 3.0+** - SSL/TLS 加密

### 内置数据库客户端
- 框架自行集成 MySQL 和 PostgreSQL 客户端，无需额外安装 MySQL Connector/C++ 或 libpq
- 支持 **MySQL 8.0.4 以上版本**（支持 `caching_sha2_password` 认证插件）
- 支持 **MariaDB 12.1+**（12.1 版本开始支持 `caching_sha2_password` 认证插件，与 MySQL 8.0 默认认证方式兼容）
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

*文档生成时间: 2026-08-15*
*版本: 基于 Paozhu Framework 代码库分析*
