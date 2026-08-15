# Paozhu C++ Web Framework - AI Development Guide

## I. Project Overview

Paozhu (炮竹) is a high-performance C++20 web framework built on the Asio asynchronous networking library, supporting HTTP/HTTPS/HTTP2 protocols. The framework adopts an MVC architecture and integrates ORM, template engine, coroutine support, and a rich set of functional modules.

### Core Features
- **Asynchronous I/O**: Asio‑based non‑blocking network model
- **Coroutine Support**: Use C++20 `co_await` for asynchronous code
- **HTTP/HTTP2**: Native support for HTTP/1.1 and HTTP/2
- **ORM**: Built‑in ORM with MySQL and PostgreSQL support
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
│   ├── gcc.conf                   # Compiler settings
│   ├── controlbuild.conf          # Controller build settings
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
├── models/                        # Data model layer
│   ├── cms/                       # CMS‑related models
│   ├── ph/                        # PostgreSQL test models
│   └── include/                   # Model headers
│
├── orm/                           # ORM layer (auto‑generated)
│   ├── cms/include/               # CMS ORM operation classes
│   │   ├── *_base.h               # Base field definitions (auto‑generated)
│   │   └── *_opsql.h              # SQL operation intermediate layer (auto‑generated)
│   ├── include/                   # Common ORM files
│   └── orm.h                      # ORM unified entry
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
│   │   │   ├── httppeer.h         # HTTP client wrapper
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
└── AI_RULES.md                    # This document
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
[section_name]     ; Database identifier (e.g., default, cms, ph)
type = main        ; main (primary) or second (replica)
host = 127.0.0.1   ; Database host
port = 3306        ; Port (MySQL: 3306, PG: 5432)
dbname = database_name  ; Database name
user = username    ; Username
password = pass    ; Password
pretable =         ; Table prefix
maxpool = 5        ; Maximum connection pool size
dbtype = mysql     ; mysql or postgresql
charset = utf8mb4  ; Character set
#ssl = ON          ; Enable SSL
#sslverify = ON    ; Verify certificate chain
#sslhost =         ; Certificate domain
```

**Database identifier conventions**:
- `default` – Benchmark testing database
- `cms` – CMS system database
- `docs` – Documentation database
- `ph` – PostgreSQL test database

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
#include "httppeer.h"         // HTTP client wrapper
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

**Real‑world examples** (from [products.cpp](file:///Users/hzq/newpoint/controller/src/saas.com/products.cpp)):
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
    
    auto [minpage, maxpage, curpage, totalpage] = model.page(page, 10);
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
    model.setPK(model.save());
    
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
    model.where("aid", id).delete();
    
    client.val["code"] = 1;
    client.val["msg"] = "Deleted successfully";
    client.out_json();
    return "";
}
}
```

---

## V. ORM Usage Guide

### 5.1 Generating ORM Code

Use the CLI tool to automatically generate ORM code from the database:

```bash
# Generate ORM for the specified database
./bin/paozhu_cli orm cms      # Generate from the [cms] database configuration
./bin/paozhu_cli orm default  # Generate from the [default] database configuration
./bin/paozhu_cli orm ph       # Generate from the [ph] PostgreSQL configuration
```

### 5.2 Model File Composition

Each table corresponds to three files:
- `models/{db}/ModelName.cpp` – Model implementation
- `orm/{db}/include/modelname_base.h` – Field base definitions (auto‑generated, do not modify manually)
- `orm/{db}/include/modelname_opsql.h` – SQL operation layer (auto‑generated, do not modify manually)

### 5.3 Basic ORM Queries

```cpp
#include "orm.h"

// Get all records
auto model = orm::cms::Article();
model.fetch();

// Conditional query
model.where("userid", 123).fetch();
model.where("title", "keyword").like();    // LIKE query
model.where("status", 1).eq();             // Equals

// Multiple conditions
model.where("userid", 123)
     .whereAnd("status", 1)
     .fetch();

// Ordering
model.desc("aid").fetch();           // Descending
model.asc("addtime").fetch();        // Ascending

// Pagination
auto [minpage, maxpage, curpage, total] = model.page(1, 10);
model.fetch();

// Count
unsigned int total = model.count();

// Aggregation
unsigned int total = model.where("status", 1).count();
```

### 5.4 Advanced ORM Queries

```cpp
// Limit results
model.limit(10).fetch();
model.limit(5, 10).fetch();  // offset, count

// Select fields
model.select("aid,title,addtime").fetch();

// Grouping
model.group("category_id").fetch();

// Aggregate functions
model.where("userid", 1).sum("price");
model.where("userid", 1).avg("score");
model.where("userid", 1).max("time");
model.where("userid", 1).min("time");

// Subqueries
model.whereIn("category_id", subQuery);

// Locking queries (within transactions)
model.where("id", 1).lock().fetch_one();
```

### 5.5 ORM Data Manipulation

```cpp
auto model = orm::cms::Article();

// Insert
model.data.title = "Title";
model.data.content = "Content";
model.data.userid = 1;
unsigned int newId = model.save();
model.setPK(newId);

// Update
model.data.title = "New Title";
model.where("aid", id).update("title");

// Batch update
model.data.status = 1;
model.where("userid", 1).update("status");

// Delete
model.where("aid", id).delete();

// Increment / decrement
model.where("aid", id).update_col("readnum", 1);     // readnum + 1
model.where("aid", id).update_col("stock", -1);      // stock - 1

// Replace in column
model.where("aid", id).replace_col("content", "old", "new");

// Direct primary key access
model.setPK(id).fetch_one();
model.where("pk", model.getPK());
```

### 5.6 Raw SQL Queries

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

### 5.7 PostgreSQL Support

```cpp
// PostgreSQL models use a separate namespace
auto model = orm::ph::Forture();   // The "ph" namespace corresponds to the [ph] configuration in orm.conf
model.fetch();

// Important: MySQL models (orm::cms) and PostgreSQL models (orm::ph) are independent.
// Generated MySQL models are hard‑bound to MySQL connections and cannot be switched to PostgreSQL.
// To use PostgreSQL, you must use models from the orm::ph namespace.
```

### 5.8 Asynchronous ORM Operations

```cpp
// Use asynchronous ORM inside a coroutine
asio::awaitable<void> handle_request()
{
    auto model = orm::cms::Article();
    
    // Asynchronous queries
    co_await model.async_fetch();
    co_await model.async_count();
    
    // Asynchronous operations
    co_await model.async_save();
    co_await model.async_update();
    co_await model.async_delete();
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
# Generate ORM code
./bin/paozhu_cli orm cms
./bin/paozhu_cli orm default
./bin/paozhu_cli orm ph
```

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
// ${title} - output variable
// ${user.name} - access nested object
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
| Cookies | `client.cookies["name"]` | Get cookie |
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

| Method | Description | Example |
|--------|-------------|---------|
| `where(col, val)` | Add WHERE condition | `.where("status", 1)` |
| `whereAnd(col, val)` | AND condition | `.whereAnd("type", 2)` |
| `whereOr(col, val)` | OR condition | `.whereOr("tag", 3)` |
| `like()` | Fuzzy match | `.like()` |
| `eq()` | Equals | `.eq()` |
| `ne()` | Not equals | `.ne()` |
| `gt()` | Greater than | `.gt()` |
| `lt()` | Less than | `.lt()` |
| `ge()` | Greater or equal | `.ge()` |
| `le()` | Less or equal | `.le()` |
| `in(val)` | IN query | `.in("1,2,3")` |
| `nin(val)` | NOT IN | `.nin("4,5")` |
| `desc(col)` | Descending order | `.desc("id")` |
| `asc(col)` | Ascending order | `.asc("time")` |
| `limit(n)` | Limit count | `.limit(10)` |
| `limit(offset, n)` | Limit with offset | `.limit(0, 20)` |
| `select(fields)` | Select fields | `.select("id,name")` |
| `group(col)` | Group by | `.group("category")` |
| `fetch()` | Execute query | Get result set |
| `fetch_one()` | Fetch single record | Get one record |
| `count()` | Count records | Return number |
| `page(page, per)` | Pagination calculation | Return pagination info |

---

## IX. Project Security Guidelines

### 9.1 Path Security
- Path validation must prevent path traversal attacks.
- Use `std::filesystem::canonicalize` to validate paths.
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

1. Create a header file in `controller/include/` (e.g., `newfeature.h`).
2. Create an implementation file in `controller/src/` (e.g., `newfeature.cpp`).
3. Register routes using `//@urlpath`.
4. Add the source file in CMakeLists.txt.
5. Rebuild the project.

### 10.2 Adding a New Database Model

1. Create the table in the database.
2. Run `./bin/paozhu_cli orm <tag>` to generate ORM code.
3. Implement business logic in `models/<db>/ModelName.cpp`.
4. Access data in controllers via `orm::<db>::ModelName()`.

### 10.3 Adding a New View

1. Create an HTML template under `view/`.
2. Use `${var}` to mark variable positions.
3. Run `./bin/paozhu_cli view` to compile the view.
4. Render it in controllers via `client.view()`.

### 10.4 Building the Project

```bash
# CMake configuration
mkdir build && cd build
cmake ..

# Compile
make -j$(nproc)

# Run
./bin/paozhu_cli         # CLI tool
./bin/paozhu             # Web server
```

---

## XI. Frequently Asked Questions

**Q: How can I add a new database connection?**  
Add a new section in `conf/orm.conf` and then use the corresponding namespace in your code.

**Q: Can ORM code be modified manually?**  
The `_base.h` and `_opsql.h` files are auto‑generated and should not be manually modified. Business logic should be implemented in model classes under `models/`.

**Q: How do I switch between PostgreSQL and MySQL?**  
Change the `dbtype` parameter in `orm.conf`, or use the CLI tool’s `dbconver` feature to migrate data.

**Q: What special tags does the view template support?**  
- `${variable}` – variable output
- `${variable|raw}` – unescaped output
- `<%c cpp_code %>` – embed C++ code
- `<% include "file.html" %>` – include sub‑template

**Q: How can I debug SQL queries?**  
Set `debug_enable = 1` in `server.conf`; ORM will log the generated SQL statements.

---

## XII. Technology Stack Dependencies

### Core Dependencies
- **C++20** – Programming language standard
- **Asio** – Asynchronous networking library
- **OpenSSL 3.0+** – SSL/TLS encryption

### Built‑in Database Clients
- The framework integrates MySQL and PostgreSQL clients directly – no need to install MySQL Connector/C++ or libpq separately.
- Supports **MySQL 8.0.4 and above** (including `caching_sha2_password` authentication plugin).
- Supports **MariaDB 12.1+** (starting from 12.1, it supports the `caching_sha2_password` authentication plugin, compatible with MySQL 8.0’s default authentication).
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

*Document generated: 2026-08-15*  
*Version: Based on Paozhu Framework codebase analysis*
