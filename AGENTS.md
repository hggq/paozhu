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
│   ├── include/                   # Controller headers (auto-generated from controller/src at build time; do not edit)
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
│   ├── autocontrolmethod.hpp      # Controller method registration (auto-generated from controller/src at build time; do not edit)
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

> **Auto-generated files (do not edit manually)**:
> - The model code under `models/` and `orm/` is generated by `bin/paozhu_cli orm <dbtag>` from the database — it is best not to modify it by hand. To change shared behaviour, edit the templates under `vendor/httpserver/include/` and regenerate (see §5.1).
> - `common/autocontrolmethod.hpp` is auto-generated by the build system when compiling the project: it extracts controller functions from `controller/src/` and writes them into `common/autocontrolmethod.hpp`. No manual editing needed — recompile after changing controllers to refresh it.
> - The `controller/include/` directory is also auto-generated by the build system (it touches an empty header for every `controller/src/*.cpp` that has none). No manual editing needed.

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
| `fetch_one()` | `async_fetch_one(isappend = false)` | Fetch single record |
| `fetch_append()` | `async_fetch_append()` | Fetch and append to existing record set |
| `fetch_to(vec)` | `async_fetch_to(vec)` | Fill a vector of custom `orm::Base` structs |
| `fetch_one_to(obj)` | `async_fetch_one_to(obj)` | Fill one custom `orm::Base` struct |
| `count()` | `async_count()` | Count matching records |
| `page(p, pp, ln)` | `async_page(p, pp = 10, ln = 5)` | Same `[bar_min, bar_max, current, total]` tuple |
| `save(isrealnew)` | `async_save(isrealnew = false)` | Insert new record |
| `insert()` / `insert(row)` / `insert(rows)` | `async_insert()` / `async_insert(row)` / `async_insert(rows)` | Same `[effect, id]` tuple; `row` is a `meta` struct, `rows` a `std::vector<meta>` |
| `update()` / `update(fields)` | `async_update()` / `async_update(fields)` | Update existing record |
| `update_dirty()` | `async_update_dirty()` | Update only columns written through the generated setters (§5.6) |
| `update_col(col, n, sign)` | `async_update_col(col, n, sign = '+')` | Increment or decrement |
| `replace_col(col, old, new)` | `async_replace_col(col, old, new)` | `REPLACE()` inside an UPDATE |
| `remove()` / `remove(id)` | `async_remove()` / `async_remove(id)` | Delete record |

`async_fetch_to` and `async_fetch_one_to` each have a second overload that takes a
callback, `(vec, callback)` / `(obj, callback)`. It fires once per non-`NULL` column of
every row as that row is read, with
`(row&, col_name, const unsigned char *data, std::size_t len, unsigned char field_type, unsigned char flag)`,
and the row is still appended afterwards — it is a column hook, not a streaming mode.

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

**Update only the columns you touched** — `update_dirty()`, its coroutine twin
`async_update_dirty()`, and the prepared pair `exec_update_dirty()` /
`async_exec_update_dirty()`. A column counts as touched only when the generated
per-field setter writes it; assigning `model.data.x` directly leaves no trace, and the
primary-key setters (`setAid`, `setPK`) never mark dirty.

```cpp
auto model = orm::cms::Article();
model.setTopicid(7);            // marks topicid dirty
model.setTitle("New Title");    // marks title dirty
model.data.content = "quiet";   // NOT dirty — this column is not in the UPDATE

model.where("aid", id).update_dirty();       // UPDATE ... SET topicid=7,title='New Title'
model.clear_dirty();                         // reset by hand if needed
model.where("aid", id).exec_update_dirty();  // same set, values bound as parameters
```

Guard rails, in both the text and the prepared form: with no dirty column the text path
returns `0` and sends nothing, while `exec_update_dirty()` sets `iserror` and returns
`(unsigned int)-1`. When `wheresql` is empty the statement falls back to the primary key
(`getPK() > 0`); with neither, the text path returns `0` and the prepared path refuses to
emit the statement. A successful update clears the dirty bits.

**`save()` is a switch, not an insert** — when the model already carries a primary key
(`getPK() > 0`) and `isrealnew` is false, `save()` emits `UPDATE ... WHERE pk = ...`
instead of an INSERT. Use `insert()` or `save(true)` when a new row is what you want.

```cpp
auto articles = orm::cms::Article();
articles.data.title = "Title";
auto [effect, newId] = articles.insert();   // always INSERT, returns [affected rows, new id]

articles.data.title = "Second";
auto [e2, id2] = articles.save(true);       // always INSERT even though a PK is set

articles.data.title = "Patched";
auto [e3, _] = articles.save();             // UPDATE the row whose pk == id2
```

`insert()` has three overloads — `insert()` takes the row from `data`, `insert(row)` takes
one `meta`, `insert(rows)` takes a `std::vector<meta>` and emits a single multi-row
`INSERT INTO t (every column) VALUES (...),(...)`, so a row you did not fill writes `0` /
`''` rather than the column default. All three return `[affected rows, insert id]` and call
`setPK()` with that id. A multi-row statement yields **one** id for the whole batch (MySQL
reports the first generated key), so it identifies a single row at best — re-query if the
callers need every new primary key.

```cpp
auto rows = std::vector<orm::cms::article_info::meta>();
// fill every column of each row, then:
auto [batch_effect, batch_id] = articles.insert(rows);
```

`set_data(row)` assigns a whole `meta` into `data` and returns the chain; `get()` is a
no-op that returns the model reference, so a long chain can be split across statements.
Neither has a call site in this repository yet — `model.data = row;` is what the existing
controllers use.

```cpp
orm::cms::Article one_row;                 // fetched elsewhere
articles.set_data(one_row.data).insert();  // data = one_row.data, then INSERT
```

**Batch upsert / replace of the whole `record` set** — `update_batch(fieldname)` sends one
statement built from `record`, not from `data`, and returns `0` immediately when `record`
is empty. Which statement it is depends on the dialect and on the argument:

| Dialect | `update_batch("a,b")` | `update_batch("")` |
|---|---|---|
| MySQL | `INSERT ... ON DUPLICATE KEY UPDATE` | `REPLACE INTO` |
| PostgreSQL | `INSERT ... ON CONFLICT (pk) DO UPDATE SET` | `INSERT ... ON CONFLICT DO NOTHING` |
| SQLite | `INSERT ... ON CONFLICT(pk) DO UPDATE SET` | `REPLACE INTO` |

The upsert form needs the conflict target, so the primary key must be set in every row of
`record`. There is no async twin and no prepared twin — use `exec_insert_batch()` for a
prepared batch insert.

**Soft delete** — `soft_remove()` / `soft_remove("extra, set, clauses")` run the `UPDATE`
that the generator emits from the table's column names, so a row is flagged rather than
deleted. `paozhu_cli orm` recognises a fixed set of conventions and writes the matching
assignment into `{table}_base.h::soft_remove_sql()`:

| Column name | Emitted assignment |
|---|---|
| `isdelete`, `deleted`, `isdeleted`, `is_deleted`, `is_delete`, `order_deleted`, `order_delete` | `col=1` |
| `deletetime`, `delete_time`, `deletedtime`, `deleted_time`, `deleted_at`, `delete_at` | `col=<unix time>` (the value is also copied into `data` when that column is selected) |

A table with none of those columns gets an empty body, and then `soft_remove()` sets
`error_msg = "soft delete field empty."` and returns `0` without sending anything. No table
in this repository currently has a matching column, so treat the API as available for new
schemas only. Like `remove()`, an empty `wheresql` falls back to the primary key and
`group()/order()/limit()` are appended to the statement.

```cpp
articles.where("aid", id).soft_remove();   // UPDATE article SET isdelete=1 WHERE aid = id
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

**The model object also runs raw SQL.** `orm::db_conn` is a standalone connection;
every model has its own set of entry points that reuse its connection pool and tag.

```cpp
auto articles = orm::cms::Article();

std::vector<orm::cust::ArticleTopSql> top;
unsigned int rows = articles.query("SELECT aid,title FROM article WHERE isopen=1 LIMIT 3", top);
// rows == top.size(); the struct must satisfy orm::Base / ResultHasSetVal

co_await articles.async_query(sqlstring, top);          // coroutine form

int affected = articles.edit_query("UPDATE article SET readnum=readnum+1 WHERE topicid=3");
co_await articles.async_edit_query(sql);                // coroutine form
```

| Method | Returns | Behaviour |
|--------|---------|-----------|
| `query(sql, rows)` / `async_query(sql, rows)` | rows fetched | SELECT path; a non‑SELECT is handed to `edit_query` and its result set is discarded |
| `edit_query(sql)` / `async_edit_query(sql)` | affected rows | sends anything through the edit connection; `sqlstring` is left alone |
| `get_query()` | `std::string` | the last generated statement, for logging; not set by `edit_query` |
| `effect()` | `unsigned int` | `effect_num` — affected rows for a write, row count for a fetch |
| `commit_insert()` / `commit_insert(row)` | `std::string` | the INSERT text, without sending it |
| `commit_update(fields)` / `commit_remove()` | `std::string` | the UPDATE / DELETE text, without sending it |

Reading takes `query(sql, rows)`, writing takes `edit_query(sql)`; there is no
`query(sql)` that takes only a statement, so a leftover call of that shape is a compile
error rather than a read that quietly returns zero rows. Never splice request data into any
of them — interpolate only values your own code produced, as in the `orm::db_conn` example
above.

`commit_*` are the escape hatch for statements the query DSL cannot express: they build the
same text the corresponding `save()`/`update()`/`remove()` would have sent, so you can
inspect it, log it, or pass it to `edit_query()` / a bulk importer. `commit_update()` and
`commit_remove()` refuse to produce an unbounded statement — with an empty `wheresql` they
fall back to the primary key, and with neither they return `""` (`commit_update()` also
sets `error_msg = "warning empty where sql!"`). `commit_insert()` has no such guard.

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

### 5.13 Result Cache (`use_cache`, in-process)

`vendor/httpserver/include/orm_cache.hpp`, plus the `use_cache` block copied into every
generated model, is a read-through cache of query results. It is process-local memory only:
no Redis, no file, no serialization, and no `orm.conf` / `server.conf` key enables or
disables it — the call is the only switch. The block is byte-identical in all three dialect
templates.

**What is stored.** The key is `std::hash<std::string>{}(sqlstring)`, the fully built
statement, so the select list, the WHERE text, the group/order and the limit are all part
of it. The DB tag is not; a model's tag is fixed when it is constructed. Two stores exist,
selected by the value type, and they never collide:

| Store | Value | Written by | Read by |
|-------|-------|------------|---------|
| `meta` | `data` (one row) | `fetch_one()`, `get_one()`, `save_data_cache()` | `fetch_one()`, `get_one()`, `get_data_cache()` |
| `std::vector<meta>` | `record` (row set) | `fetch()`, `fetch_append()`, `save_cache()` | `fetch()`, `fetch_append()`, `get_record_cache()` |

`fetch_row()` keeps its rows in three further stores — the
`std::vector<std::vector<std::string>>` itself, the column-name vector and the name → index
map — keyed the same way. Their value types say nothing about the model, so all three are
shared process-wide: unlike the two stores above, they are not per-`meta`-type.
`fetch_to()` / `fetch_one_to()` are never cached, and neither is anything on the prepared
path.

**Arming a read-through.** `use_cache(int cache_time = 0)` is chainable and arms the next
read; `cache_time` is a TTL in seconds, `0` meaning *never expires*.

```cpp
auto articles = orm::cms::Article();
articles.where("isopen", 1).where("topicid", 3).use_cache(60).fetch();  // record store, 60s
```

Before the statement goes out, the read consults its store; on a hit it fills `record` /
`data`, disarms and returns. After a real read it writes back only
`if (iscache && exptime > 0)`, then disarms. Taken literally, that yields three rules:

- `use_cache()` with the default TTL never stores anything — the write-back needs
  `exptime > 0`. It means "arm, then do nothing", not "cache forever".
- A hit returns `0` from `fetch()` / `async_fetch()` / `fetch_one()` / `get_one()`, exactly
  what an empty result set returns, so `if (model.fetch_one() > 0)` reads every cache hit as
  a failure.
- Nothing else disarms the slot. `clear()` and `clearWhere()` reset `iscache` but leave
  `exptime` behind; `set_cache_state(bool)` sets the flag alone.

**Writes never invalidate.** No `save()`, `update()`, `update_dirty()`, `update_col()`,
`replace_col()` or `remove()` touches either store, and the `exec_*` / `async_exec_*` path
neither consults nor fills the cache. Rows changed through those methods, through
`orm::db_conn`, or by any other process keep being served until the TTL lapses.

**There is no refresh in place.** `model_meta_cache::save()` inserts; if the key is already
present it rewrites only the stored `exptime` and leaves the old rows untouched
(`cover_data` defaults to `false` and no wrapper passes it). Re-caching a live key extends
its life, never changes its content. The delete/clear family — `clear_cache()`,
`remove_cache()` in both forms, and `remove_exptime_cache()` — reaches the `meta` and the
`record` store, so either row set can be dropped, and `remove_cache()` reports `true` when
either store held the key. `check_cache()` and `update_cache()` stay single-store (`meta`
and `record` respectively). The three stores `fetch_row()` uses are keyed by generic value
types, so every model shares them; nothing on a model clears them and they age out by TTL
alone.

**A miss has side effects.** `get_data_cache()` and `get_record_cache()` set `error_msg`
and call `unlock_conn()` when the key is absent. `iserror` stays false, but a cold read
inside a `lock_conn()` batch hands the connection back to the pool.

**Named keys.** The manual API takes a caller-chosen `std::string` and hashes it itself,
which is how you cache anything whose SQL the DSL builds differently. The accessors that
return a stored value — `get_cache(name)` and `get_vector_cache(name)` — signal a miss by
throwing `std::runtime_error("Not in cache")` after setting `error_msg` and calling
`unlock_conn()`, so they need a `try`. `check_cache()`, `get_record_cache()` and
`update_cache()` take the `std::size_t` instead, so hash the same name with
`std::hash<std::string>{}(name)`.

```cpp
const std::string key = "cms:article:top3:" + std::to_string(topicid);
try
{
    articles.record = articles.get_vector_cache(key);   // throws on a miss
}
catch (const std::exception &)
{
    articles.clearWhere();
    articles.where("isopen", 1).desc("aid").limit(3).fetch();
    articles.save_vector_cache(key, articles.record, 60);
}
```

`save_vector_cache(name, rows, ttl)` and `save_cache(name, rows, ttl)` are the same call.
`remove_cache()` with no argument keys off `sqlstring`, so it does something only after a
statement has been built on that object.

**`isuse_cache()` is not a hit flag.** With no argument it reports whether the slot is still
armed. `isuse_cache(true)` returns `exptime == 0 && iscache == false`, whose meaning flips
with the TTL mode: after a real read under `use_cache(60)` the write-back has just zeroed
both, so it returns `true`, while a hit leaves `exptime` at `60` and returns `false`. Under
`use_cache()` (TTL `0`) it is the other way round. Treat it as "the arming was consumed",
not as "the rows came from cache".

**A cold `fetch_row()` does not throw.** Its read consults three stores under one key, and
`model_meta_cache::get()` throws on a miss or an expired entry, so the three gets sit in a
`try` and a miss — or a set that lapsed unevenly — falls through to a real query. A hit
counts only when all three succeed, which is why the returned tuple never carries rows with
an empty column-name list. `fetch_obj()` touches the cache in neither direction: it reads
from the database every time and writes nothing back, so it cannot empty out the `record`
store that `fetch()` reads through. `fetch_row()` consumes the `use_cache(ttl)` arming on a
hit and keeps it on a miss, so the write-back at the end of a real read still runs;
`fetch_obj()` never looks at it, so an arming left on the object survives to the next read.
The named-key accessors above still throw by design.

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
`where("isopen=1")` overload (§5.3). Every `whereXxx` below has a `whereOrXxx` sibling
taking the same arguments; it only differs in joining with `OR` to the condition that
precedes it, which is what you want inside a group opened by `andsub()` / `orsub()`.

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
| `whereOrBT` / `whereOrBE` / `whereOrLT` / `whereOrLE` / `whereOrNQ` | `OR` with `>` / `>=` / `<` / `<=` / `!=` | `.whereOrLT("price", 100)` |
| `whereOrIn(col, vals)` / `whereOrNotIn(col, vals)` | `OR IN (...)` / `OR NOT IN (...)` | `.whereOrIn("id", ids)` |
| `whereOrNull(col)` / `whereOrNotNull(col)` | `OR IS NULL` / `OR IS NOT NULL` | `.whereOrNull("deleted_at")` |
| `where(col, "op", val)` | operator as a string (`"="`, `"!="`, `">"`, `"<"`, `"LIKE"`, `"NOT LIKE"`, …) | `.where("title", "NOT LIKE", "kw")` |

`whereOrGT` / `whereOrGE` and `whereOrEQ` / `whereOrNE` do not exist: `whereOr(col, val)`
already means `OR col = val`, and `>` / `>=` are spelled `whereOrBT` / `whereOrBE`.

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
| `order(col, "ASC")` / `order(col, "DESC")` | `ORDER BY` from a `cols` enum and a direction string — the column must be the enum, there is no `order("aid", "DESC")` overload | `.order(orm::cms::article_info::cols::aid, "DESC")` |
| `order("expr")` / `asc("expr")` / `desc("expr")` | `ORDER BY` with the argument copied verbatim | `.order("aid DESC, title ASC")` |
| `asc()` / `desc()` | `ORDER BY` on the primary key | `.desc()` |
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
| `insert()` / `insert(row)` / `insert(rows)` | `[effect, id]`; always INSERT, `rows` is one multi-row statement (§5.6) |
| `update_batch(fields)` | effect; one upsert (or replace, with an empty argument) built from `record` (§5.6) |
| `soft_remove()` / `soft_remove(extra)` | effect; the generated flag-column UPDATE, `0` when the table has none (§5.6) |
| `update_col(col, n, sign)` | increment (`'+'`) or decrement (`'-'`) |
| `replace_col(col, old, new)` | `REPLACE()` inside an UPDATE |
| `update_dirty()` | effect; `0` and no statement when no column is dirty (§5.6) |

**Execution — prepared path** (§5.8): every read and single-row write above has an `exec_`
twin, and the coroutine form prefixes `async_`. No prepared equivalent exists for
`insert(rows)` (`exec_insert_batch()` covers the batch case), `update_batch()`,
`soft_remove()`, or the `query()`/`edit_query()` family.

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

**Result cache** (§5.13) — "store" says which of the two maps a method touches: `meta`
holds one row (`data`), `vector` holds a row set (`record`).

| Method | Store | Notes |
|--------|-------|-------|
| `use_cache(ttl = 0)` | — | chainable; arms the next text-path read |
| `isuse_cache(bydate = false)` | — | the arming flag, not a hit indicator |
| `set_cache_state(bool)` | — | force `iscache`, leaves `exptime` |
| `save_data_cache(ttl = 0)` | `meta` | stores `data` under `hash(sqlstring)` |
| `save_data_cache(name, row, ttl = 0)` | `meta` | caller-named key |
| `save_cache(ttl = 0)` | `vector` | stores `record` under `hash(sqlstring)` |
| `save_cache(key, rows, ttl = 0)` / `save_cache(name, rows, ttl = 0)` / `save_vector_cache(name, rows, ttl = 0)` | `vector` | caller-named key |
| `save_cache(name, row, ttl = 0)` | `meta` | one row under a name |
| `get_data_cache(key)` / `get_record_cache(key)` | `meta` / `vector` | fill `data` / `record`, return `bool`; a miss writes `error_msg` and calls `unlock_conn()` |
| `get_cache(name)` / `get_vector_cache(name)` | `meta` / `vector` | return the stored value; **throw** `std::runtime_error` on a miss |
| `check_cache(key)` | `meta` | `-1` absent, `0` permanent, else seconds remaining |
| `update_cache(ttl = 0)` / `update_cache(key, ttl)` | `vector` | TTL only; `-1` when the key is absent |
| `remove_cache()` / `remove_cache(key)` | `meta` + `record` | the no-arg form keys off `sqlstring`; `true` when either store held the key |
| `clear_cache()` / `remove_exptime_cache()` | `meta` + `record` | whole stores / entries whose TTL lapsed |

Read-through is built into `fetch()`, `fetch_append()`, `fetch_one()`, `get_one()` and their
`async_` twins, plus `fetch_row()`; `fetch_obj()` neither reads nor writes, and no `exec_*`
method reads or writes.

**Untyped result shapes** — for a column list the typed `meta` cannot express. These read
the same `select()/where()/limit()` state as `fetch()`.

| Method | Returns | Notes |
|--------|---------|-------|
| `fetch_obj()` | `std::vector<std::map<std::string, std::string>>` | one map per row; every value is a string and `NULL` becomes `""`, so a `0` and an empty column are indistinguishable |
| `fetch_row()` | `tuple<vector<string>, map<string, unsigned int>, vector<vector<string>>>` | column names, name → index, and the rows; positional access keeps duplicate column names |
| `fetch_json()` / `async_fetch_json()` | `http::obj_val` | an array of string-valued objects, ready to output |
| `get_one(id)` / `async_get_one(id)` | `long long` | one row by primary key into `data`; ignores `wheresql` entirely, so a chained `where()` is silently dropped |

Both forms of `get_one()` return the number of rows read, so `if (m.get_one(id) > 0)` means
the row landed in `data`. The one exception is the result cache: a `use_cache()` hit returns
`0` from every read-through method, `get_one()` included, so when a cache may be armed test
`data` rather than the return code.

**Table and connection switching**

| Method | Description |
|--------|-------------|
| `set_table(name)` | overrides the table name for subsequent statements; an empty argument is ignored |
| `reset_table()` | restores `org_tablename` |
| `switchDB(tag)` | rebinds the model to another `orm.conf` tag **of the same engine** — a MySQL model refuses a `postgresql`/`sqlite` tag. On success `dbtag` follows the switch, so later statements and error text name the pool actually in use |
| `resetDB()` | the inverse: rebinds to `B_BASE::_rmstag`, the tag this model was **generated** from, and so drops whatever `switchDB()` pointed the object at. Chainable, same engine guard as `switchDB()` |
| `get_db_type()` | the model's compile-time engine, `DB_TYPE::MYSQL` / `POSTGRESQL` / `SQLITE` — a property of the generated class, not of the connection |
| `data` | the current row (`{table}_info::meta`) |
| `record` | the loaded row set (`std::vector<meta>`) |
| `error_msg` | last error text |
| `iserror` | latches once set — while it is true, most reads and writes return `0` without touching the database. Only `clear()` / `clearWhere()` reset it (both also clear `error_msg`) |

`switchDB()` changes only the pool the model borrows connections from, which makes it the
hook for same-engine sharding. It is a chainable `M_MODEL &` method, so
`model.switchDB("cms").where("aid", id).fetch()` is the intended spelling. A rejected switch
clears the connection and latches `iserror`, and the chain continues onto that object — the
read that follows returns `0` without touching a database, so check `error_msg` after the
chain rather than after the call. Both rejection texts name **the tag being asked for**:
`conn_pool not found <tag>` when no such section exists, `conn_pool db type error <tag>` when
the section is on another engine.

`resetDB()` is the matching return trip — `model.switchDB("cms").fetch()` then
`model.resetDB().fetch()` reads the shard and then the table's own database again, on one
object. It is a plain rebind, not a repair: it re-attaches the connection pool but, like
every other method, leaves a latched `iserror` alone. After a **failed** `switchDB()`, a
`resetDB()` restores `conn_obj` and `dbtag` yet the model still refuses to read — call
`clear()` (or `clearWhere()`) to drop the latch, and note that `clear()` also wipes
`error_msg`, so read it first.

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

## XIII. Clean Project Initialization

After cloning the Paozhu framework from GitHub, you usually want to strip out the example business code and start with a clean scaffold that keeps the framework core plus the hello annotation. Follow this four‑step cleanup flow.

### Step 1 — Clean up common/ registration files

Six files need to be trimmed: keep `#include "httppeer.h"`, remove every other `#include`, and empty the bodies of registration functions such as `_initauto_control_httpmethodregto()` (keep the signatures):

| File | Purpose |
|------|---------|
| `common/autocontrolmethod.hpp` | Controller method registration (auto-extracted by the build system from `controller/src`) |
| `common/autorestfulpaths.hpp` | RESTful path registration |
| `common/reghttpmethod.hpp` | HTTP method registration |
| `common/reghttpmethod_pre.hpp` | HTTP pre-filter registration |
| `common/sockets_method_reg.hpp` | Socket method registration |
| `common/websockets_method_reg.hpp` | WebSocket method registration |

Each file should end up looking like:

```cpp
#include "httppeer.h"

void _initauto_control_httpmethodregto(std::map<std::string, regmethold_t> &methodcallback)
{
    // empty implementation
}
// other registration functions similarly cleared
```

### Step 2 — Remove the ORM layer

```bash
rm -rf models/* schema/* orm/*
```

These three directories hold database models and generated ORM code — they are not needed for a clean scaffold. Regenerate later with `bin/paozhu_cli orm <tag>` (see §5.1) when you enable a database.

### Step 3 — Clean up view files (keep the registration skeleton)

Do **not** `rm -rf` the whole `viewsrc/` directory — the registration headers under `viewsrc/include/` must be kept as skeletons. Three sub-steps.

#### 3.1 Remove the view source and template directories

```bash
rm -rf viewsrc/view/ view/
```

`viewsrc/view/` holds the C++ implementations; `view/` holds the HTML templates. Both are example code and can be deleted directly.

#### 3.2 Empty the `namespace view` block in `viewsrc/include/viewsrc.h`

Open `viewsrc/include/viewsrc.h`, delete every inner namespace (admin, cms, home, login, superadmin, techempower) and their function declarations inside `namespace http { namespace view { ... } }`. **Keep the outer namespace wrappers and all `#include` lines.** The file should end up like:

```cpp
#ifndef __HTTP_VIEWSRC_ALL_METHOD_H
#define __HTTP_VIEWSRC_ALL_METHOD_H
// ... includes unchanged ...

namespace http { 
namespace view { 

}

}
#endif
```

#### 3.3 Empty the function body in `viewsrc/include/regviewmethod.hpp`

Open `viewsrc/include/regviewmethod.hpp`, delete every `_viewmetholdreg.emplace(...)` line inside `_initview_method_regto`. **Keep the function signature, namespace, and all `#include` lines.** The file should end up like:

```cpp
namespace http
{
  void _initview_method_regto(VIEW_REG  &_viewmetholdreg)
  {
    // empty body
  } 
}
```

This removes all example business code while preserving the full view-registration skeleton. When you add new views later, just declare functions inside `namespace view` and register them in `regviewmethod.hpp`.

### Step 4 — Remove controllers (keep testhello)

```bash
rm -rf controller/include/*
find controller/src -type f ! -name 'testhello.cpp' -delete
```

The remaining `controller/src/testhello.cpp` carries the `//@urlpath(null,hello)` annotation and is the single entry point of the hello world route. Once you rebuild, `http://127.0.0.1/hello` is live again.

### Done

Rebuild the project after the four steps to get a clean scaffold:

```bash
cd build && cmake .. && make -j$(sysctl -n hw.ncpu)
```

Run `./bin/paozhu` and open `http://127.0.0.1/hello` — you should see "Hello world! 🧨 Paozhu c++ web framework", which confirms the framework core is intact.

---

*Last updated: 2026-09-18*
