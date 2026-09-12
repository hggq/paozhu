# Paozhu C++ Web Framework - AI Development Guide

## I. Project Overview

Paozhu (炮竹) is a high-performance C++20 web framework built on the Asio asynchronous networking library, supporting HTTP/HTTPS/HTTP2 protocols. The framework adopts an MVC architecture and integrates ORM, template engine, coroutine support, and a rich set of functional modules.

### Core Features
- **Asynchronous I/O**: Asio‑based non‑blocking network model
- **Coroutine Support**: Use C++20 `co_await` for asynchronous code
- **HTTP/HTTP2**: Native support for HTTP/1.1 and HTTP/2
- **ORM**: Built‑in ORM with MySQL PostgreSQL and SQLite support
- **OCSP Stapling**: Automatic OCSP stapling
- **ACME**: Automatic SSL certificate issuance and renewal
- **Modular Extensions**: Image processing, Excel, Word, PDF, chart generation, etc.

### Compilation Standards
- C++20 or later
- OpenSSL 3.0+ (for SSL/TLS)
- Cross‑platform: macOS, Linux, Windows

---

## II. Directory Structure

```
paozhu/
├── conf/                          # Configuration files
│   ├── server.conf                # Main server configuration
│   ├── orm.conf                   # Database connection settings
│   ├── acme.conf                  # ACME certificate settings
│   └── ...                        # Other configuration and SQL files
│
├── controller/                    # Controller layer
│   ├── include/                   # Controller headers
│   │   ├── admin/                 # Admin controllers
│   │   └── test*.h               # Functional test controllers
│   └── src/                       # Controller implementations
│       ├── admin/                 # Admin implementations
│       └── test*.cpp              # Functional test implementations
│
├── models/                        # Data model layer (one tree per DB tag)
│   ├── cms/                       # CMS-related models (MySQL)
│   ├── pg/                        # PostgreSQL models
│   ├── lite/                      # SQLite models
│   └── include/                   # Models of the default DB tag
│
├── orm/                           # ORM layer (auto-generated)
│   ├── cms/include/               # CMS ORM operation classes
│   │   ├── *_base.h               # Base field definitions (auto-generated)
│   │   └── *_opsql.h              # SQL operation intermediate layer (auto-generated)
│   ├── pg/include/                # PostgreSQL variants of the same layout
│   ├── lite/include/              # SQLite variants of the same layout
│   ├── include/                   # Generated files of the default DB tag
│   ├── {table}_opsql.cpp          # Out-of-line oneXXX/manyXXX bodies for FK models
│   └── orm.h                      # ORM unified entry (includes every dialect)
│
├── view/                          # View templates (HTML)
│   ├── admin/                     # Admin views
│   ├── home/                      # Home views
│   └── ...                        # Other views
│
├── viewsrc/                       # Compiled view artifacts (C++ sources)
│   ├── include/                   # View registration headers
│   └── view/                      # View implementations
│
├── vendor/                        # Third‑party / internal libraries
│   ├── httpserver/                # HTTP server core
│   │   ├── include/               # Server headers
│   │   │   ├── request.h          # HTTP request handling
│   │   │   ├── httppeer.h         # HTTP request/response handler
│   │   │   ├── router.h           # URL routing
│   │   │   └── ...               # Other headers
│   │   └── src/                   # Server implementations
│   ├── httpcli/                   # CLI tools (code generators)
│   ├── pzexcel/                   # Excel read/write
│   ├── pzword/                    # Word document generation
│   ├── pzzip/                     # ZIP compression/decompression
│   ├── pzimage/                   # Image processing (JPG/PNG)
│   ├── pzcharts/                  # SVG chart generation
│   ├── webpdf/                    # HTML to PDF conversion
│   ├── sms/                       # SMS sending
│   └── webpay/                    # WeChat / Alipay payments
│
├── libs/                          # Business utility libraries
│   ├── img/                       # Image upload handling
│   ├── markdown/                  # Markdown to HTML conversion
│   ├── pinyin/                    # Chinese to Pinyin conversion
│   ├── ipdata/                    # IP geolocation lookup
│   └── types/                     # Common type definitions
│
├── common/                        # Common definitions
│   ├── autorestfulpaths.hpp       # RESTful path registration
│   ├── reghttpmethod.hpp          # HTTP method registration
│   └── ...
│
├── sockets/                       # Socket extensions
├── websockets/                    # WebSocket support
├── startup/                       # Program entry points
│   ├── main_dev.cpp               # Development mode entry
│   ├── main_daemon.cpp            # Daemon mode entry
│   └── main_docker.cpp            # Docker entry
│
├── www/                           # Web static file root
├── docs/                          # Example resources (images, fonts, etc.)
├── CMakeLists.txt                 # CMake build configuration
└── AGENTS.md                      # This document
```

---

## III. Configuration File Reference

### 3.1 server.conf – Server Configuration

```ini
[default]
; Thread settings
threadmax = 1024          ; Maximum threads
threadmin = 5             ; Minimum threads
cothreadnum = 8           ; Number of coroutine‑running threads

; Port settings
httpport = 80             ; HTTP port
httpsport = 443           ; HTTPS port

; SSL settings
mainhost = www.hggq.com   ; Primary domain
certificate_chain_file = www.hggq.com.pem    ; Certificate chain file
private_key_file = www.hggq.com.key         ; Private key file
dh4096.pem               ; DH parameters file

; Path settings
modelspath = ./models     ; Models directory
viewpath = ./view         ; Views directory
controlpath = ./controller; Controllers directory
temppath = ./temp         ; Temporary files directory
logpath = ./log           ; Logs directory
wwwpath = ./www/default   ; Web root directory

; Feature toggles
http2_enable = 1          ; HTTP/2 switch
debug_enable = 1          ; Debug mode
deamon_enable = 0         ; Daemon mode

; ACME settings
acme_auto = 0             ; Automatic SSL certificate renewal
acme_every_day = 7        ; Hour of day to run ACME
ocsp_intv_time = 14400    ; OCSP update interval (seconds)

; Session settings
session_type = 1          ; 0=file 1=memory 2=redis
static_file_compress_cache = 1  ; Static file compression cache

; Upload settings
upload_max_size = 16777216  ; Maximum upload size (16 MB)
```

### 3.2 orm.conf – Database Configuration

```ini
[section_name]     ; Database tag, e.g. default, cms, pg, lite
type = main        ; main (primary) or second (replica)
host = 127.0.0.1   ; Database host; for dbtype = sqlite this is the DB file path
port = 3306        ; Port (MySQL: 3306, PostgreSQL: 5432); empty for sqlite
dbname = database_name  ; Database name
user = username    ; Username
password = pass    ; Password
pretable =         ; Table prefix
maxpool = 5        ; Maximum connection pool size
dbtype = mysql     ; mysql | postgresql | sqlite
charset = utf8mb4  ; Character set
#ssl = ON          ; Enable SSL
#sslverify = ON    ; Verify certificate chain
#sslhost =         ; Certificate domain
```

A section name **is** the DB tag: it is what you pass to `paozhu_cli orm <tag>`, what
`model.dbtag` holds, and what generated code passes to `orm::db_conn("cms")`.

**Database tag conventions** (in this repository):
- `default` – benchmark tables (`world`, `fortune`, `fk_parent`, `fk_child`)
- `cms` – CMS system database
- `docs` – documentation database
- `pg` – PostgreSQL test database (`dbtype = postgresql`)
- `lite` – SQLite test database (`dbtype = sqlite`, `host = ./docs/sqlite_lite.db`, `port` empty)

### 3.3 acme.conf – ACME Certificate Configuration

```ini
[info]
acme_path = ./acme         ; ACME working directory
cert_path = ./conf         ; Certificate storage directory
email = your@email.com     ; Email address (mandatory)
days_remain = 10           ; Renew certificate this many days before expiry

[zerossl]
server_url = https://acme.zerossl.com/v2/DV90  ; ZeroSSL endpoint
eab_kid =                  ; EAB Key ID
eab_hmac_key =             ; EAB HMAC Key
```

---

## IV. Controller Development Guidelines

### 4.1 Controller File Structure

```cpp
// controller/src/example.cpp
#include "orm.h"              // ORM entry
#include "httppeer.h"         // HTTP request/response handler
#include "func.h"             // Utility functions
#include "example.h"          // Corresponding header

namespace http
{
// Controller function implementation
}
```

### 4.2 URL Route Registration

Use `//@urlpath` comments to register URL routes:

```cpp
//@urlpath(null,hello)
std::string testhello(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client << "Hello World!";
    return "";
}
```

**Route format**: `//@urlpath(前置过滤器,URL路径)`

- First argument: pre‑filter function name (`null` means no filter)
- Second argument: URL path

### 4.3 Pre‑filters

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
        return "exit";         // Returning "exit" stops further execution
    }
    return "ok";
}

//@urlpath(admin_islogin,admin/main)  // Use the filter
std::string admin_main(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    client.view("admin/main");
    return "";
}
```

### 4.4 Common httppeer APIs

```cpp
httppeer &client = peer->get_peer();

// Request data
client.post["field"].to_string();   // POST data
client.post["field"].to_int();      // POST data as int
client.get["param"];                // GET parameter
client.client_ip;                   // Client IP
client.is_ssl();                    // Whether HTTPS is used

// Response output
client << "content";                // Direct output
client.view("path");                // Render view (without theme prefix)
peer->theme_view("path");           // Render themed view (automatically adds theme prefix)
client.goto_url("/path");           // Redirect
client.goto_url("/path", 3, msg);   // Redirect with delay and message

// JSON response
client.val["key"] = value;          // Set JSON field
client.out_json();                  // Output JSON

// Session management
client.session["key"] = value;      // Set session
client.session["key"].to_int();     // Read session
client.save_session();              // Save session

// File upload
client.files["field"]["filename"];  // Uploaded filename
client.files["field"]["tempfile"];  // Temporary file path

// Paths
client.get_sitepath();              // Get site path
client.get_hosturl();               // Get host URL
```

### 4.5 Themed Views (`theme_view`)

`theme_view` is a view‑rendering method that supports multi‑theme / multi‑tenant setups. Unlike `view`, it automatically adds a path prefix based on the `themes` configuration of the current domain.

```cpp
// view: renders view/front/producthome.html directly
peer->view("front/producthome");

// theme_view: if the current domain has themes = "saas1",
// it actually renders view/saas1/front/producthome.html.
// If no themes is set, it is equivalent to peer->view("front/producthome").
peer->theme_view("front/producthome");
```

**Usage examples** (the declaration of `theme_view` is in `vendor/httpserver/include/httppeer.h`):
```cpp
peer->theme_view("front/producthome");
peer->theme_view("front/productcatalogue");
peer->theme_view("front/producttopic");
peer->theme_view("front/productlist");
peer->theme_view("front/productbrand");
peer->theme_view("front/productsearch");
```

**Configuration** (in domain configuration files):
```ini
[saas1.example.com]
themes = saas1              ; Theme directory name, prepended to view paths
themes_url = /saas1/        ; Static resource URL prefix for the theme
```

**Related APIs**:

```cpp
// Get the current domain's theme name
std::string theme = peer->get_theme();     // Returns "saas1" or ""

// Manually concatenate theme path and then use view (not recommended, only for compatibility)
std::string viewfile = peer->get_theme();
if (!viewfile.empty()) viewfile += "/";
viewfile += "front/productcatalogue";
peer->view(viewfile);  // Equivalent to peer->theme_view("front/productcatalogue")

// Get the current domain's theme URL prefix (for static assets in templates)
std::string themeUrl = peer->get_themeurl();  // Returns "/saas1/" or ""
```

**Use cases**: Serving the same system to multiple tenants (SaaS) with different domains and theme template directories (e.g., `view/saas1/`, `view/saas2/`). Controllers use `theme_view` uniformly without caring about the specific theme name.

### 4.6 Complete Controller Example

```cpp
#include "orm.h"
#include "httppeer.h"
#include "func.h"
#include "testcrud.h"

namespace http
{
// List page
//@urlpath(null,crud/list)
std::string crud_list(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    
    auto model = orm::cms::Article();
    unsigned int page = client.get["page"].to_int();
    if (page == 0) page = 1;
    
    auto [minpage, maxpage, curpage, totalpage] = model.page(page, 10, 5);   // 3rd parameter list_num: page link count
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

// Add
//@urlpath(null,crud/addpost)
std::string crud_addpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    
    auto model = orm::cms::Article();
    model.data.userid = client.session["userid"].to_int();
    model.data.title = client.post["title"].to_string();
    model.data.content = client.post["content"].to_string();
    auto [effect, newId] = model.save();   // returns [affected rows, auto-increment ID]; save() already sets PK
    
    client.goto_url("/crud/list", 3, "Added successfully!");
    return "";
}

// Edit
//@urlpath(null,crud/editpost)
std::string crud_editpost(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int id = client.post["id"].to_int();
    
    auto model = orm::cms::Article();
    model.data.title = client.post["title"].to_string();
    model.where("aid", id);
    model.update("title");
    
    client.goto_url("/crud/list", 3, "Modified successfully!");
    return "";
}

// Delete
//@urlpath(null,crud/delete)
std::string crud_delete(std::shared_ptr<httppeer> peer)
{
    httppeer &client = peer->get_peer();
    unsigned int id = client.get["id"].to_int();
    
    auto model = orm::cms::Article();
    model.where("aid", id).remove();
    
    client.val["code"] = 1;
    client.val["msg"] = "Deleted successfully";
    client.out_json();
    return "";
}
}
```

### 4.7 Coroutine Controller Pattern

Controllers can be written using C++20 coroutines for non‑blocking asynchronous I/O. The framework automatically detects the return type and dispatches accordingly.

**Key rules**:
- Return type: `asio::awaitable<std::string>` (instead of `std::string`)
- Use `co_await` for all async ORM operations
- Use `co_return ""` instead of `return ""`
- Use `lock_conn()` / `unlock_conn()` to reuse a single connection in batch operations (avoid repeated connect/disconnect overhead)

**Example 1 – Basic coroutine controller** (no DB, plain response):

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

**Example 2 – Coroutine with async ORM query**:

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
    co_await myworld.async_fetch_one();   // Async single‑row fetch

    peer->output = myworld.data_tojson();
    co_return "";
}
```

**Example 3 – Coroutine with async batch queries** (using `lock_conn`/`unlock_conn`):

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
    myworld.lock_conn();                 // Lock connection for reuse in loop
    for (unsigned int i = 0; i < get_num; i++)
    {
        myworld.wheresql.clear();
        unsigned int rd_num = rand_range(1, 10000);
        myworld.where("id", rd_num);
        co_await myworld.async_fetch_append();  // Async append to record set
    }
    myworld.unlock_conn();               // Release connection
    peer->output = myworld.to_json();
    co_return "";
}
```

**Example 4 – Coroutine with async update** (fetch + update in batch):

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
            co_await myworld.async_update("randomnumber");  // Async update
        }
    }
    myworld.unlock_conn();
    peer->output = myworld.to_json();
    co_return "";
}
```

**Available async ORM methods**:

Text path (values escaped into the statement):

| Sync method | Async method | Description |
|-------------|-------------|-------------|
| `fetch()` | `async_fetch()` | Fetch all matching records |
| `fetch_one()` | `async_fetch_one()` | Fetch single record |
| `fetch_append()` | `async_fetch_append()` | Fetch and append to existing record set |
| `count()` | `async_count()` | Count matching records |
| `save()` | `async_save()` | Insert new record |
| `update()` | `async_update()` | Update existing record |
| `remove()` | `async_remove()` | Delete record |

Prepared path (values bound as parameters, see §5.8) — every `exec_` method gains an
`async_exec_` twin with the same signature and return type:

| Sync method | Async method |
|-------------|-------------|
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

`exec_update_fields()` has no async twin; use `async_exec_update("col1,col2")` instead.

**Note**: Synchronous controllers (returning `std::string`) and coroutine controllers (returning `asio::awaitable<std::string>`) can coexist in the same project. The framework automatically selects the correct execution path based on the function signature.

---

## V. ORM Usage Guide

### 5.1 Generating ORM Code

Use the CLI tool to automatically generate ORM code from a live database:

```bash
# One command per DB tag (the tag is the section name in conf/orm.conf)
./bin/paozhu_cli orm default   # MySQL tables of [default]   -> orm/include/,   models/include/
./bin/paozhu_cli orm cms       # MySQL tables of [cms]       -> orm/cms/include/, models/cms/
./bin/paozhu_cli orm pg        # PostgreSQL tables of [pg]   -> orm/pg/include/,  models/pg/
./bin/paozhu_cli orm lite      # SQLite tables of [lite]     -> orm/lite/include/, models/lite/
```

**The templates are not compiled.** `vendor/httpserver/include/mysqlorm.hpp`,
`sqliteorm.hpp` and `postgresqlorm.hpp` are code-generation templates: `paozhu_cli orm`
reads them as text and copies the whole class body into every generated `_opsql.h`.
Editing a template therefore changes **nothing** at build time — the generated files keep
the body they were generated from. After any change to a template, or to `orm_common.h`,
regenerate every tag you use and rebuild; otherwise queries built through the new API
silently keep the old behaviour (typically returning 0 rows).

### 5.2 Model File Composition

Each table corresponds to these files (`{tag}` is the DB tag; omitted for `default`):

- `orm/{tag}/include/{table}_base.h` – field metadata, `cols` enum, row `meta` struct
  (auto-generated, do not modify manually)
- `orm/{tag}/include/{table}_opsql.h` – SQL operation layer: a full copy of the template
  body plus the per-table column switch and the generated per-field DSL
  (auto-generated, do not modify manually)
- `orm/{tag}/{table}_opsql.cpp` – only for tables with foreign keys: out-of-line
  specializations of the `oneXXX` / `manyXXX` accessors
- `models/{tag}/ModelName.cpp` + `models/{tag}/include/ModelName.h` – the concrete model
  class that binds the two templates above, e.g.
  `class Fortune : public fortune_opsql<Fortune, fortune_base>`

`orm/orm.h` includes the models of every tag, so one `#include "orm.h"` gives access to
`orm::Fortune`, `orm::cms::Article`, `orm::pg::Fortune` and `orm::lite::Fortune`.

### 5.3 Basic ORM Queries

A condition can be written three equivalent ways. Pick one per call chain; all three end
up in the same `wheresql` vector.

```cpp
#include "orm.h"

auto model = orm::cms::Article();

// 1. column name as a string
model.where("userid", 123).fetch();

// 2. type-safe column enum  (orm::{tag}::{table}_info::cols)
model.where(orm::cms::article_info::cols::userid, 123).fetch();

// 3. generated per-field DSL — one method per column, {op}{Field}
//    ops: eq nq bt be lt le like null notnull, each with an or{op}{Field} sibling
model.eqUserid(123).fetch();
```

The first argument is always a **column name**, never a SQL fragment. There is no
`where("isopen=1")`: that single-argument overload was removed from all three templates
because it stamped `op_type` with `orm::wq::qb`, no builder had a case for it, and the
condition was silently dropped — which could widen an `update()` or `remove()` to the
whole table. Writing the operator inside the column string is not a supported spelling
either; `where(" aid=", aid)` fails the `findcolpos` check and sets
`field is not table column`. Pass the column and the value separately
(`where("isopen", 1).where("aid", aid)`), and when the query shape really is exotic, run
the statement through `orm::db_conn` (§5.7).

```cpp
// Comparison helpers (string or cols enum as the first argument)
model.whereBE("price", 100);      // >=
model.whereBT("price", 100);      // >
model.whereLT("price", 100);      // <
model.whereLE("price", 100);      // <=
model.whereNQ("price", 100);      // !=
model.whereNull("deleted_at");
model.whereNotNull("deleted_at");

// Several conditions AND together by default; whereAnd/whereOr state it explicitly
model.where("userid", 123)
     .whereAnd("status", 1)
     .fetch();

// LIKE: the ORM attaches the % wildcards, callers must NOT write them
model.whereLike("title", "kw");        // LIKE '%kw%'
model.whereLikeLeft("title", "kw");    // LIKE '%kw'  (suffix match)
model.whereLikeRight("title", "kw");   // LIKE 'kw%'  (prefix match)
model.whereNotLike("title", "kw");     // NOT LIKE '%kw%'
// the same four exist with an Or prefix: whereOrLike / whereOrLikeLeft /
// whereOrLikeRight / whereOrNotLike. Passing "kw%" yourself yields "kw%%".

// Ordering
model.desc("aid").fetch();             // ORDER BY aid DESC
model.asc("addtime").fetch();

// Pagination
auto [minpage, maxpage, curpage, total] = model.page(1, 10);
model.fetch();

// Count
unsigned int total = model.count();
```

### 5.4 Parenthesis Grouping (Boolean Logic)

`andsub()` and `orsub()` open a parenthesis and choose how it joins to what precedes it
(`andsub` = `AND (`, `orsub` = `OR (`); `endsub()` closes it with `)`. They take no
conditions themselves, and nothing is auto-balanced — an unclosed group yields invalid SQL.

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

The same three methods feed the prepared path (§5.8), where the generic `AND()` / `OR()`
spelling is available too:

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

`clearWhere()` drops every accumulated condition (including group markers) so one model
object can be reused for the next query.

### 5.5 Advanced ORM Queries

```cpp
// Limit results
model.limit(10).fetch();
model.limit(5, 10).fetch();  // offset, count

// Select fields
model.select("aid,title,addtime").fetch();

// Grouping
model.group("category_id").fetch();

// Aggregate functions (no sum/avg/max/min chain methods;
// use a SELECT alias plus a custom struct with fetch_to)
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
model.fetch_to(result);    // result[0].total holds the aggregate

// IN lists: a container of values, or a comma-separated string
std::vector<long long> ids = {1, 2, 3};
model.whereIn("category_id", ids).fetch();
model.whereIn("category_id", "1,2,3").fetch();
model.whereNotIn("category_id", ids).fetch();
// No overload takes a query object, so a subquery is either a fetch-then-bind
// (run the inner query, collect its ids into a vector, pass it above) or a raw
// statement through orm::db_conn (§5.7).

// Batch operations: keep the same connection (within transactions)
model.lock_conn();
model.where("id", 1).fetch_one();
// ... more operations
model.unlock_conn();
```

### 5.6 ORM Data Manipulation

```cpp
auto model = orm::cms::Article();

// Insert
model.data.title = "Title";
model.data.content = "Content";
model.data.userid = 1;
auto [effect, newId] = model.save();   // returns [affected rows, auto-increment ID]; save() already sets PK

// Update
model.data.title = "New Title";
model.where("aid", id).update("title");

// Batch update
model.data.status = 1;
model.where("userid", 1).update("status");

// Delete
model.where("aid", id).remove();

// Increment / decrement
model.where("aid", id).update_col("readnum", 1);     // readnum + 1
model.where("aid", id).update_col("stock", -1);      // stock - 1

// Replace in column
model.where("aid", id).replace_col("content", "old", "new");

// Query by primary key (setPK records the auto-increment ID after save();
// it cannot replace a where() query)
model.where("aid", id).fetch_one();
```

### 5.7 Raw SQL Queries

When complex SQL is needed or ORM does not support the query, use `orm::db_conn` as an independent database connection:

```cpp
#include "orm.h"
#include "orm_query.h"

// 1. Define a result structure inheriting from orm::Base, using ORM_NAMES macro
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

// 2. Use an independent database connection to execute SQL
std::vector<orm::cust::LocalusersqlStruct> loaduser;
auto ulink = std::make_unique<orm::db_conn>("cms");  // "cms" is the database label in orm.conf

std::string sqlstring = "SELECT adminid,name,nickname FROM sysuser where 1 limit 1";

// Synchronous mode
ulink->query(sqlstring, loaduser);

// Coroutine mode (inside an asio::awaitable function)
co_await ulink->async_query(sqlstring, loaduser);

// 3. Read results
for (auto &row : loaduser)
{
    client << "adminid:" << row.adminid
           << ", name:" << row.name
           << "<br />";
}
```

**Note**: The fields selected in the SQL must correspond one‑to‑one with the fields in the returned structure.

### 5.8 Prepared Statement Queries

Every read and write has a prepared twin. The condition-building API is shared, so
`where()`, the per-field DSL and `andsub()/orsub()/endsub()` all work unchanged — the
difference is only in how the SQL reaches the server: values are bound as parameters
(`?` on MySQL and SQLite, `$1…$n` on PostgreSQL) instead of being escaped into the
statement text, so no value can alter the query shape.

```cpp
auto world = orm::World();
world.AND("id", orm::wq::be, 1)
     .AND("id", orm::wq::le, 200)
     .orsub()
     .AND("randomnumber", orm::wq::bt, 5000)
     .OR("randomnumber", orm::wq::lt, 200)
     .endsub();

unsigned int total = world.exec_count();          // vs. count()
world.select("id,randomnumber");
world.exec_fetch();                               // rows into world.record
std::vector<orm::cust::WorldRowStruct> rows;
world.exec_fetch_to(rows);                         // rows into a custom struct
world.exec_one();                                   // single row lands in world.data
```

Writes follow the same naming, and mirror the text-path return values:

```cpp
auto model = orm::cms::Testa();
model.data.parentid = 7;
model.data.content  = "prepared insert";
auto [effect, newid] = model.exec_insert();       // same tuple as save()

model.clear();
model.data.content = "prepared update";
model.where(orm::cms::testa_info::cols::id, newid);
model.exec_update("content");                     // or exec_update() for all set fields
model.exec_update_fields({{"content", "x"}});     // explicit name/value pairs
model.exec_update_col("parentid", 3, '+');        // += 3   ('-' decrements)
model.exec_replace_col("content", "old", "new");
model.exec_remove();

auto batch = orm::cms::Testa();
batch.record.push_back(row1);                     // meta structs
batch.record.push_back(row2);
auto [batch_effect, first_id] = batch.exec_insert_batch();
```

`exec_page(page, per_page, list_num)` and `exec_count()` are the prepared equivalents of
`page()` and `count()`. Async versions prefix `async_exec_` (§5.12). Prefer the `exec_`
family for anything whose value comes from a request; the text family is kept for
compatibility and for SQL fragments (raw `whereIn("id", "1,2,3")`, expressions in
`select()`) that must be spliced into the statement.

### 5.9 LEFT JOIN / INNER JOIN

`leftJoin<T>()` / `innerJoin<T>()` take the joined **model type**, then a `join*` chain
configures the join. The joined columns appear in the outer SELECT, and the result rows
land in the main model's `record`.

```cpp
// SELECT world.id, world.randomnumber, fortune.message
// FROM world LEFT JOIN fortune ON fortune.id = world.id
auto world = orm::World();
world.btId(2).ltId(6);
world.select("id,randomnumber");
world.leftJoin<orm::Fortune>().joinOn("id", "id").joinSelect("message");
world.fetch();
```

`joinOn(join_col, main_col)` compares one column of the joined table with one column of
the main table; the right-hand name may be a `cols` enum.

`joinWhere()` / `joinWhereOr()` / `joinDesc()` / `joinAsc()` / `joinGroup()` only take
effect once `joinLimit()` is also set: without a limit the ORM emits a plain
`LEFT JOIN tbl ON …` and never builds the sub-query those methods feed. With a limit it
wraps the joined table in `( SELECT … WHERE … ORDER BY … LIMIT n )`, and adding
`joinGroup()` turns that into a `ROW_NUMBER() OVER (PARTITION BY …)` window, i.e. top-N
rows **per group**:

```cpp
// 2 newest articles per topic, for topics owned by user 1
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

`joinParAppend()` adds further `PARTITION BY` columns.

### 5.10 Foreign Keys: oneXXX / manyXXX

`paozhu_cli orm` reads the database's real single-column foreign keys and, for each
related table `{Camel}`, emits `one{Camel}` / `many{Camel}` on both sides of the
relationship. Their bodies live in the generated `orm/{tag}/{table}_opsql.cpp`, so that
file must be in the build for the accessors to link.

```cpp
// child side: fk_child.parent_id -> fk_parent.id
auto child = orm::FkChild();
child.where(orm::fk_child_info::cols::id, 1);
child.fetch_one();                       // oneXXX keys off data, so load the row first

auto parent = child.oneFkParent();       // FkParent scoped to this row's parent_id
parent.fetch_one();

auto parent2 = child.oneFkParent(2);     // explicit key instead of the current row

auto decorated = orm::FkParent();        // void overload: decorate, then keep chaining
child.oneFkParent(decorated);
decorated.select("id,name");

// parent side: all children of every row currently in record
auto parents = orm::FkParent();
parents.fetch();                         // manyXXX keys off record, so fetch first
auto children = parents.manyFkChild();   // WHERE parent_id IN (<ids in record>)
children.fetch();
```

`many{Camel}` builds its `IN (...)` list from
`get_cols_vec<{self}_info::cols::pk>()`, i.e. the primary keys of the rows already
loaded. Load children once and group them in memory instead of
calling the accessor per row — per-row calls are one query each. The `one{Camel}(obj&)`
and `many{Camel}(obj&)` overloads return `void` and add the scope to the model you pass
in, which is how you attach a `select()` or an extra `where()` to the relation.

### 5.11 Cross-Dialect Usage: MySQL / PostgreSQL / SQLite

The three dialects expose the same public API — `mysqlorm.hpp`, `sqliteorm.hpp` and
`postgresqlorm.hpp` differ only in private escaping and placeholder helpers. What differs
is which generated model you use, because a model is bound to the tag it was generated
from at codegen time; there is no runtime switch.

```cpp
auto a = orm::Fortune();          // [default]   MySQL      namespace orm
auto b = orm::cms::Article();     // [cms]       MySQL      namespace orm::cms
auto c = orm::pg::Fortune();      // [pg]        PostgreSQL namespace orm::pg
auto d = orm::lite::Fortune();    // [lite]      SQLite     namespace orm::lite
```

| | MySQL | PostgreSQL | SQLite |
|---|---|---|---|
| `orm.conf` `dbtype` | `mysql` | `postgresql` | `sqlite` |
| `host` / `port` | host / 3306 | host / 5432 | DB file path / empty |
| model namespace | `orm` or `orm::{tag}` | `orm::pg` | `orm::lite` |
| generated tree | `orm/{tag}/` | `orm/pg/` | `orm/lite/` |
| prepared placeholder | `?` | `$1…$n` | `?` |

PostgreSQL emits `DEFAULT` for a zero auto-increment primary key so the sequence assigns
it; SQLite binds `NULL` for the same case. Everything else — `exec_*`, joins, FK
accessors, `andsub()/orsub()/endsub()` — behaves identically across the three.

### 5.12 Asynchronous ORM Operations

```cpp
// Use asynchronous ORM inside a coroutine
asio::awaitable<void> handle_request()
{
    auto model = orm::cms::Article();

    // Asynchronous queries (text path)
    co_await model.async_fetch();
    co_await model.async_count();

    // Asynchronous operations (text path)
    co_await model.async_save();
    co_await model.async_update();
    co_await model.async_remove();

    // Asynchronous prepared path
    co_await model.async_exec_count();
    co_await model.async_exec_fetch();
    co_await model.async_exec_insert();
}
```

---

## VI. CLI Tool Usage

### 6.1 View Compilation

```bash
# Compile modified views
./bin/paozhu_cli view

# Force recompile all views
./bin/paozhu_cli view force
```

### 6.2 ORM Generation

```bash
# Generate ORM code for one DB tag (section name in conf/orm.conf)
./bin/paozhu_cli orm default
./bin/paozhu_cli orm cms
./bin/paozhu_cli orm pg
./bin/paozhu_cli orm lite
```

Always re-run this for every tag after editing `vendor/httpserver/include/{mysql,sqlite,
postgresql}orm.hpp` or `orm_common.h`: the generated `_opsql.h` files embed a copy of the
template body, so an unregenerated tree keeps compiling the old code (§5.1).

### 6.3 JSON Reflection Generation

```bash
# Scan the libs directory and generate reflection code for structs annotated with [//@reflect json to_json from_json]
./bin/paozhu_cli json
```

### 6.4 Database Migration

```bash
# Migrate from cms to pg
./bin/paozhu_cli dbconver cms pg

# Force migration (overwrite existing tables)
./bin/paozhu_cli dbconver cms pg force
```

### 6.5 SQL Export

```bash
# Export table structures
./bin/paozhu_cli dbtable cms ./schema.sql
./bin/paozhu_cli dbtable cms ./pg.sql -target=pg      # Export as PG syntax

# Export structure + data
./bin/paozhu_cli dbexport cms ./dump.sql
./bin/paozhu_cli dbexport cms ./mysql.sql -target=mysql
```

---

## VII. View Template Development

### 7.1 View Template Location

View templates reside in the `view/` directory, organised into subdirectories by function:
- `view/admin/` – Admin views
- `view/home/` – Home views
- `view/login/` – Login views

### 7.2 View Rendering

```cpp
// Render views in controllers
client.view("admin/main");       // Renders view/admin/main.html
client.view("home/header");      // Renders view/home/header.html
```

### 7.3 Passing Variables to Views

```cpp
// Set view variables in controllers
client.val["title"] = "Page Title";
client.val["user"]["name"] = "John Doe";
client.val["list"].set_array();
client.val["list"].push(item);

// Use in view templates
// <%c echo<<obj["title"].as_string(); %>      - output variable
// <%c echo<<obj["user"]["name"].as_string(); %> - access nested object
// <%c ... %> - embed C++ code
```

### 7.4 Compiling Views to C++

View templates are compiled into C++ source files in `viewsrc/` via the CLI tool:

```bash
./bin/paozhu_cli view          # Incremental compilation
./bin/paozhu_cli view force    # Full compilation
```

---

## VIII. Core API Reference

### 8.1 Request Object (httppeer)

| Feature | Method | Description |
|---------|--------|-------------|
| GET parameters | `client.get["key"]` | Get URL query parameters |
| POST data | `client.post["key"]` | Get POST data |
| File uploads | `client.files["field"]` | Get uploaded file info |
| Cookies | `client.cookie["name"]` | Get cookie |
| Session | `client.session["key"]` | Read/write session |
| Output text | `client << "text"` | Output string directly |
| Render view | `client.view("path")` | Render a view template |
| Themed view | `client.theme_view("path")` | Render view with theme prefix |
| Get theme | `client.get_theme()` | Get current domain’s theme name |
| Get theme URL | `client.get_themeurl()` | Get static resource URL prefix for theme |
| Redirect | `client.goto_url("/path")` | Redirect page |
| JSON output | `client.out_json()` | Output JSON response |
| Client IP | `client.client_ip` | Get client IP |
| SSL detection | `client.is_ssl()` | Whether HTTPS is used |

### 8.2 ORM Chainable Query Methods

Method names below are the ones defined in
`vendor/httpserver/include/{mysql,sqlite,postgresql}orm.hpp`; the three dialects expose the
same set.

**Conditions** — the first argument is either a column name (`"status"`) or a
`{tag}::{table}_info::cols` value. It is never a SQL fragment; there is no
`where("isopen=1")` overload (§5.3).

| Method | SQL | Example |
|--------|-----|---------|
| `where(col, val)` | `AND col = ?` | `.where("status", 1)` |
| `whereEQ` / `whereNE` | `=` / `!=` | `.whereNE("status", 0)` |
| `whereBT` / `whereBE` | `>` / `>=` | `.whereBE("price", 100)` |
| `whereLT` / `whereLE` | `<` / `<=` | `.whereLT("price", 100)` |
| `whereGT` / `whereGE` | aliases of `whereBT` / `whereBE` | `.whereGE("price", 100)` |
| `whereNQ(col, val)` | `!=` | `.whereNQ("status", 0)` |
| `whereIn(col, vals)` | `IN (...)` — array, `std::vector`, or raw csv | `.whereIn("id", std::vector<int>{1,2,3})` |
| `whereNotIn(col, vals)` | `NOT IN (...)` | `.whereNotIn("id", "4,5")` |
| `whereNull(col)` / `whereNotNull(col)` | `IS NULL` / `IS NOT NULL` | `.whereNull("deleted_at")` |
| `whereAnd(col, val)` | explicit AND | `.whereAnd("type", 2)` |
| `whereOr(col, val)` | explicit OR | `.whereOr("tag", 3)` |
| `where(col, "op", val)` | operator as a string (`"="`, `"!="`, `">"`, `"<"`, `"LIKE"`, `"NOT LIKE"`, …) | `.where("title", "NOT LIKE", "kw")` |

**LIKE** — the ORM attaches the `%` wildcards; never pass them yourself.

| Method | SQL | Example |
|--------|-----|---------|
| `whereLike(col, v)` | `LIKE '%v%'` | `.whereLike("title", "kw")` |
| `whereLikeLeft(col, v)` | `LIKE '%v'` (suffix match) | `.whereLikeLeft("title", "kw")` |
| `whereLikeRight(col, v)` | `LIKE 'v%'` (prefix match) | `.whereLikeRight("title", "kw")` |
| `whereNotLike(col, v)` | `NOT LIKE '%v%'` | `.whereNotLike("title", "kw")` |
| `whereOrLike` / `whereOrLikeLeft` / `whereOrLikeRight` / `whereOrNotLike` | same four, joined with `OR` | `.whereOrLike("content", "kw")` |

**Grouping and modifiers**

| Method | Description | Example |
|--------|-------------|---------|
| `andsub()` | opens `AND (` | see §5.4 |
| `orsub()` | opens `OR (` | see §5.4 |
| `endsub()` | closes the parenthesis with `)` | see §5.4 |
| `clearWhere()` | drop all conditions and group markers | |
| `clear()` | drop conditions, data and generated SQL | |
| `desc(col)` / `asc(col)` | `ORDER BY` | `.desc("aid")` |
| `limit(n)` / `limit(offset, n)` | `LIMIT` | `.limit(0, 20)` |
| `select(fields)` | column list, expressions and aliases allowed | `.select("id,title")` |
| `group(col)` | `GROUP BY` | `.group("topicid")` |
| `having(cond)` | `HAVING` | `.having("count(*) > 1")` |
| `lock_conn()` / `unlock_conn()` | reuse one connection across a batch | §5.5 |

**Execution — text path** (values escaped into the statement)

| Method | Returns |
|--------|---------|
| `fetch()` / `fetch_one()` | effect count; rows in `record`, first row in `data` |
| `fetch_append()` | appends rows to `record` |
| `fetch_to(vec)` / `fetch_one_to(obj)` | fill a custom `orm::Base` struct |
| `count()` | number of matching rows |
| `page(page, per_page, list_num)` | `[bar_min, bar_max, current, total]` |
| `save()` / `update(fields)` / `remove()` | `[effect, id]` / effect / effect |
| `update_col(col, n, sign)` | increment (`'+'`) or decrement (`'-'`) |
| `replace_col(col, old, new)` | `REPLACE()` inside an UPDATE |

**Execution — prepared path** (§5.8): every method above has an `exec_` twin, and the
coroutine form prefixes `async_`.

| Text | Prepared | Async prepared |
|------|----------|----------------|
| `count()` | `exec_count()` | `async_exec_count()` |
| `page(...)` | `exec_page(...)` | `async_exec_page(...)` |
| `fetch()` | `exec_fetch()` / `exec_fetch_to(v)` | `async_exec_fetch()` / `async_exec_fetch_to(v)` |
| `fetch_one()` | `exec_one()` / `exec_one_to(v)` | `async_exec_one()` / `async_exec_one_to(v)` |
| `fetch_append()` | `exec_fetch_append()` | `async_exec_fetch_append()` |
| `save()` | `exec_insert()` | `async_exec_insert()` |
| batch insert | `exec_insert_batch()` | `async_exec_insert_batch()` |
| `update(...)` | `exec_update()` / `exec_update(fields)` / `exec_update_fields(pairs)` / `exec_update_dirty()` | `async_exec_update*()` (no `async_exec_update_fields`) |
| `update_col(...)` | `exec_update_col(...)` | `async_exec_update_col(...)` |
| `replace_col(...)` | `exec_replace_col(...)` | `async_exec_replace_col(...)` |
| `remove()` | `exec_remove()` | `async_exec_remove()` |

**Joins and foreign keys** (§5.9, §5.10)

| Method | Description |
|--------|-------------|
| `leftJoin<T>()` / `innerJoin<T>()` | start a join with model type `T` |
| `joinOn(join_col, main_col)` | `ON` condition, column to column |
| `joinSelect(fields)` | columns of the joined table to return |
| `joinWhere(col, op, val)` / `joinWhereOr(...)` | condition inside the joined sub-query (needs `joinLimit`) |
| `joinAsc(col)` / `joinDesc(col)` | order inside the joined sub-query (needs `joinLimit`) |
| `joinGroup(col)` / `joinParAppend(col)` | `PARTITION BY` column (needs `joinLimit`) |
| `joinLimit(n)` | switches to the sub-query form; with `joinGroup` = top-N per group |
| `one{Camel}()` / `one{Camel}(id)` / `one{Camel}(obj&)` | generated FK: the single related row |
| `many{Camel}()` / `many{Camel}(obj&)` | generated FK: all related rows for the loaded set |
| `get_cols_vec<{table}_info::cols::col>()` | the column values of `record`, the set `many{Camel}` scopes with. Generated per table in `_base.h`; an overload takes a `bool(const value&)` filter |

---

## IX. Project Security Guidelines

### 9.1 Path Security
- Path validation must prevent path traversal attacks.
- Use `std::filesystem::canonical` to validate paths.
- Temporary files must not have group/others permissions.

### 9.2 Database Security
- Use ORM parameterised queries to prevent SQL injection.
- Sensitive data should be stored encrypted in configuration files.
- Database connections should follow the principle of least privilege.

### 9.3 Session Security
- Session IDs must be generated using a secure random source.
- Sensitive operations require session validation.
- Support for session expiration time configuration.

### 9.4 SSL/TLS
- OCSP Stapling must be enabled.
- Use OpenSSL 3.0+.
- Automatic certificate renewal (ACME).

---

## X. Development Workflow

### 10.1 Adding a New Controller

1. Create an implementation file in `controller/src/` (e.g., `newfeature.cpp`) and
   register its routes with `//@urlpath` comments.
2. The matching `controller/include/newfeature.h` is optional: CMake touches an empty
   header for every `controller/src/*.cpp` that has none.
3. There is no CMakeLists.txt edit — `controller/src/*.cpp` is collected with
   `file(GLOB_RECURSE ... CONFIGURE_DEPENDS)`, so a plain rebuild picks the new file up.

### 10.2 Adding a New Database Model

1. Create the table in the database.
2. Run `./bin/paozhu_cli orm <tag>` to generate ORM code.
3. Add business logic to `models/<tag>/ModelName.cpp`.
4. Access data in controllers via `orm::<tag>::ModelName()` (`orm::ModelName()` for the
   default tag).

### 10.3 Adding a New View

1. Create an HTML template under `view/`.
2. Use `<%c echo<<obj["var"].to_string(); %>` to output variables.
3. Run `./bin/paozhu_cli view` to compile the view.
4. Render it in controllers via `client.view()`.

### 10.4 Building the Project

```bash
# CMake configuration
mkdir build && cd build
cmake ..                 # add -DENABLE_SQLITE=ON to build the [lite] tag (§XII)

# Compile
make -j$(nproc)

# Run
./bin/paozhu_cli         # CLI tool
./bin/paozhu             # Web server
```

---

## XI. Frequently Asked Questions

**Q: How can I add a new database connection?**  
Add a new section in `conf/orm.conf`, run `./bin/paozhu_cli orm <tag>` to generate its
models, then use the matching namespace (`orm::<tag>::ModelName`) in your code.

**Q: Can ORM code be modified manually?**  
The `_base.h` and `_opsql.h` files are auto‑generated and should not be manually
modified. Business logic belongs in the model classes under `models/`. Changing the
shared behaviour instead means editing the templates under `vendor/httpserver/include/`,
and that requires regenerating every tag afterwards — see §5.1.

**Q: How do I switch between PostgreSQL and MySQL?**  
You don't switch at runtime. Each generated model is bound to its `orm.conf` tag at
code-generation time, so `orm::Fortune` can only ever talk to the MySQL `[default]`
section, while `orm::pg::Fortune` and `orm::lite::Fortune` are separate generated classes
tied to PostgreSQL and SQLite. To run on another engine, generate that tag with
`./bin/paozhu_cli orm <tag>` and use its namespace; `dbtype` in `orm.conf` decides which
client a *new* tag gets, and `./bin/paozhu_cli dbconver <from> <to>` moves the data
across.

**Q: A predicate the query DSL does not cover — how do I express it in `where()`?**  
You cannot hand a SQL fragment to `where()`. Its first argument is a column name and the
operator is a separate argument (`where("price", orm::wq::be, 100)`, or
`where("price", ">=", 100)`). For anything the DSL does not cover — full‑text matches,
`EXISTS`, expressions over several columns — run the statement verbatim through
`orm::db_conn` (§5.7) with a custom `orm::Base` struct, and interpolate only values your
code produced, not request data.

**Q: What special tags does the view template support?**  
- `<%c echo<<obj["variable"].to_string(); %>` – variable output
- `<%c echo<<obj["variable"].as_string(); %>` – output text (escaped by default, no `raw` syntax)
- `<%c cpp_code %>` – embed C++ code
- `<%c include_sub("home/header",obj); %>` – include sub‑template

**Q: How can I debug SQL queries?**  
Set `debug_enable = 1` in `server.conf`; ORM will log the generated SQL statements.

---

## XII. Technology Stack Dependencies

### Core Dependencies
- **C++20** – Programming language standard
- **Asio** – Asynchronous networking library
- **OpenSSL 3.0+** – SSL/TLS encryption

### Built‑in Database Clients
- The framework integrates the MySQL, PostgreSQL and SQLite clients directly – no need to
  install MySQL Connector/C++, libpq or link SQLite by hand.
- Supports **MySQL 8.0.4 and above** (including `caching_sha2_password` authentication plugin).
- Supports **MariaDB 12.1+** (starting from 12.1, it supports the `caching_sha2_password` authentication plugin, compatible with MySQL 8.0’s default authentication).
- **SQLite** is behind the `ENABLE_SQLITE` CMake option (default `OFF`). Turn it on to
  build the `[lite]` tag: CMake then adds `sqlite_conn.cpp` and locates the library —
  `find_package(SQLite3 REQUIRED)` in the vcpkg branch, `find_path`/`find_library` in the
  system branch. `vcpkg.json` / `xmake.lua` already list `sqlite3` as a dependency.
- With `ENABLE_SQLITE=OFF`, `vendor/httpserver/src/sqlite_conn_stub.cpp` still compiles
  (it is always in the source list) but its `#ifndef ENABLE_SQLITE` stubs win, so a
  `dbtype = sqlite` section fails at connect time with
  `SQLite support not compiled. Rebuild with -DENABLE_SQLITE=ON` instead of crashing.
- Supports both **C++20 coroutines** (asynchronous) and **synchronous** operation.

### Optional Dependencies
- **Redis** – Session storage
- **FastCGI** – PHP support
- **zlib** – Compression support

### Internal Modules
- `vendor/httpserver/` – HTTP server core
- `vendor/httpcli/` – CLI code generator
- `vendor/pzexcel/` – Excel processing
- `vendor/pzword/` – Word processing
- `vendor/pzzip/` – ZIP compression
- `vendor/pzimage/` – Image processing
- `vendor/pzcharts/` – SVG charts
- `vendor/webpdf/` – PDF generation
- `vendor/sms/` – SMS service
- `vendor/webpay/` – Payment integration

---

*Last updated: 2026-09-12*  
*ORM sections were written against the current `vendor/httpserver/include/*orm.hpp`
templates, not a snapshot: every documented method name is grep-checked, the call chains in
§5.4–§5.10 are the ones exercised by `controller/src/test_ormprepared.cpp` and
`controller/src/test_ormfk.cpp` (both compile clean), and all three dialect templates are
compile-probed with these examples by `build/ormprobe/gen.py`, which since 2026-09-12 also
runs a negative probe: the single-argument raw-SQL `where()`/`whereOr()` overloads were
deleted from all three templates, and `build/ormprobe/neg_control.py` shows that probe keys
on the deletion (re-adding the overloads makes it compile clean again). String literals in
the docs are placeholders and may differ from the sample files. None of it changes runtime
behaviour until `paozhu_cli orm` regenerates the existing `_opsql.h` copies (§5.1).*
