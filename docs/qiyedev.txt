# C++ Web 框架开发企业后台所需模块清单（Paozhu 适配版）

> 说明：本表基于 **Paozhu C++20 Web 框架** 的实际内置能力审核修订。
> 状态图例：
> - ✅ 内置 = Paozhu 已原生提供，无需引入第三方库
> - 🔧 半内置 = Paozhu 提供能力基座，业务规则需少量自研
> - ➕ 需引入 = Paozhu 未提供，需第三方库或自研
> 原文档中约 2/3 被标为"需引入第三方库"的能力，在 Paozhu 下已内置，本版已逐一订正。

---

## 一、基础架构层

| 模块 | 说明 | 原推荐方案 | Paozhu 状态 | 修订建议 |
|------|------|-----------|------------|----------|
| HTTP 服务器 | 路由、中间件、请求解析 | Drogon/Crow/Oat++ | ✅ 内置 | 删除第三方推荐，直接用 Paozhu（`vendor/httpserver`，`server.conf` 配端口/路由） |
| 模板引擎 | 服务端渲染 HTML | Inja/Mustache | ✅ 内置 | 删除第三方，用 Paozhu 编译型模板 `view/` + `theme_view` |
| JSON 处理 | API 序列化 | nlohmann/json | ✅ 内置 | 删除第三方，用 `client.val` + `client.out_json()` |
| 配置管理 | ini/yaml/toml 读取 | yaml-cpp/toml++ | ✅ 内置（ini） | 删除第三方，用 Paozhu `server.conf`/`orm.conf` 等 ini 配置 |
| 日志系统 | 分级、轮转、异步 | spdlog/glog | ✅ 内置 | 删除第三方，用框架内置日志 |
| 协程/线程池 | 高并发调度 | libco | ✅ 内置 | 删除第三方，用 Asio + C++20 `co_await` + 内置线程池（`cothreadnum`） |
| HTTP/2 | 多路复用协议 | （原文档遗漏） | ✅ 内置 | **新增**：`http2_parse.cpp`，`http2_enable=1` 开启 |

---

## 二、安全与认证

| 模块 | 说明 | 原推荐方案 | Paozhu 状态 | 修订建议 |
|------|------|-----------|------------|----------|
| 用户认证 | 登录/登出/Token | jwt-cpp | ✅ 内置 | 删除第三方，用内置 `jwt.h` |
| 权限管理 | RBAC/ABAC | Casbin-cpp | 🔧 半内置 | 框架提供 Session/路由钩子，角色权限表与中间件需自研 |
| OAuth2/SSO | 微信/钉钉/LDAP | 自研 + HTTP 客户端 | 🔧 半内置 | 用内置 `httpclient.h` 对接，协议流程自研 |
| 密码加密 | bcrypt/argon2/PBKDF2 | libargon2 | ✅ 内置（OpenSSL） | 用 OpenSSL 哈希，无需额外库 |
| HTTPS/TLS | 传输层加密 | OpenSSL 手写 | ✅ 内置 | 删除"手动集成"，框架已封装 TLS |
| 验证码 | 图形/滑动 | 自研 | 🔧 半内置 | 用 `pzimage` 绘图自研，或接第三方 |
| 防暴力破解 | 登录锁定/IP 限流 | Redis 计数 | 🔧 半内置 | 用内置 `pzredis` 计数 + 自研中间件 |
| CSRF/XSS | Web 安全过滤 | 自研中间件 | 🔧 半内置 | 基于框架钩子自研中间件 |
| 接口签名 | HMAC-SHA256 防篡改 | OpenSSL | ✅ 内置 | 用 OpenSSL 签名校验 |
| 数据脱敏 | 手机/身份证掩码 | 自研工具 | 🔧 半内置 | 写 `libs/` 工具函数 |
| ACME 证书 | 自动签发/续期 | （原文档遗漏） | ✅ 内置 | **新增**：`acme.conf` `acme_auto`，零停机续期 |
| OCSP Stapling | 证书装订 | （原文档遗漏） | ✅ 内置 | **新增**：`ocsp_intv_time` 自动更新 |

---

## 三、数据层

| 模块 | 说明 | 原推荐方案 | Paozhu 状态 | 修订建议 |
|------|------|-----------|------------|----------|
| ORM/数据库访问 | CRUD/迁移/连接池 | sqlpp11/SOCI | ✅ 内置 | 删除第三方，用 Paozhu ORM + `paozhu_cli orm <tag>` |
| 数据库驱动 | MySQL/PG/SQLite | libmysqlclient 等 | ✅ 内置 | 删除第三方，框架内置三方言驱动 |
| 连接池 | 复用/健康检查 | 自研 | ✅ 内置 | 用 `maxpool`（orm.conf） |
| 缓存 | 热点/Session/锁 | hiredis | ✅ 内置 | 删除第三方，用 `pzredis`（已封装 Redis） |
| ORM 二级缓存 | 查询结果缓存 | （原文档遗漏） | ✅ 内置 | **新增**：`model_meta_cache` + `use_cache()/save_cache()`，30s 过期清理 |
| 全文搜索 | 商品/文档检索 | Elasticsearch/MeiliSearch | ➕ 需引入 | 保留，经 HTTP API 调用 |
| 数据字典 | 枚举/下拉选项 | DB + 缓存 | 🔧 半内置 | DB 表 + `pzredis` 缓存 |
| 多租户 | SaaS 隔离 | 分库分表 | ✅ 内置 | **修订**：框架 `theme_view` + 域名 `themes` 配置已支持路径/资源隔离 |
| 数据备份 | 定时备份 | mysqldump + cron | 🔧 半内置 | 删除 mysqldump，用内置 `dbdump-all <dbtag> <file.sql>`（`dbdump_all.hpp`，支持跨库类型导出结构+数据）；定时调度配 cron 即可 |

---

## 四、文件与文档处理

| 模块 | 说明 | 原推荐方案 | Paozhu 状态 | 修订建议 |
|------|------|-----------|------------|----------|
| Excel 读写 | .xlsx 导入导出 | OpenXLSX/xlnt | ✅ 内置 | 删除第三方，用 `pzexcel` |
| HTML→PDF | 报表/合同/发票 | wkhtmltopdf | ✅ 内置 | 删除第三方，用 `webpdf` |
| Word 生成 | .docx 模板填充 | libopc | ✅ 内置 | 删除第三方，用 `pzword` |
| CSV 读写 | 大数据量导入导出 | RapidCSV | 🔧 半内置 | 小规模自研；超大用 RapidCSV |
| 文件上传/下载 | 分片/断点续传 | 框架 + 自研 | ✅ 内置 | 用 `client.files` + `upload_max_size` |
| 对象存储 | 图片/文件云端 | MinIO/OSS SDK | ➕ 需引入 | 保留，经 HTTP API 或 SDK |
| 图片处理 | 缩略/裁剪/水印/转码 | OpenCV/ImageMagick | ✅ 内置 | 删除第三方，用 `pzimage` |
| 二维码 | 生成（QR 图片） | libqrencode | ✅ 内置 | 删除第三方，用 `qrcode.h`（`text()`/`encode()`，支持 ECC 等级与点阵缩放） |
| 条码 | 一维码生成/识别 | zxing | ➕ 需引入 | 框架仅内置二维码，条码保留 zxing 等 |
| 压缩包 | zip/tar.gz 打包 | libzip/minizip | ✅ 内置 | 删除第三方，用 `pzzip` |
| 图形报表 | 图表生成 | （原文档遗漏） | ✅ 内置 | **新增**：`pzcharts`（SVG 图表） |
| 文件预览 | Office/PDF 在线预览 | OnlyOffice | ➕ 需引入 | 保留外部服务 |

---

## 五、通信与消息

| 模块 | 说明 | 原推荐方案 | Paozhu 状态 | 修订建议 |
|------|------|-----------|------------|----------|
| 邮件发送 | SMTP/模板邮件 | vmime/mailio | ✅ 内置 | 删除第三方，用 `send_email`/`async_send_email` |
| 短信通知 | 验证码/告警 | 云 SDK | ✅ 内置 | 删除第三方，用 `sms` 模块 |
| WebSocket | 实时推送/在线状态 | uWebSockets | ✅ 内置 | 删除第三方，用 `websockets/` |
| 消息队列 | 异步/削峰 | RabbitMQ/Kafka | ➕ 需引入 | 保留（框架无内置 MQ） |
| 站内信/通知 | 公告/待办 | DB + WS | 🔧 半内置 | DB + 内置 WebSocket 推送 |
| HTTP 客户端 | 调第三方 API | libcurl/cpr | ✅ 内置 | 删除第三方，用 `httpclient.h` |
| gRPC | 微服务通信 | gRPC C++ | ➕ 需引入 | 保留（按需） |
| MQTT | IoT 设备通信 | Paho | ✅ 内置 | **修订**：框架内置 **MQTT 5.0 Broker**（`mqtt/`） |

---

## 六、任务与调度

| 模块 | 说明 | 原推荐方案 | Paozhu 状态 | 修订建议 |
|------|------|-----------|------------|----------|
| 定时任务 | Cron 调度 | croncpp | 🔧 半内置 | 框架有定时器基座（ACME/Orm缓存周期），复杂 Cron 自研或 croncpp |
| 异步任务队列 | 后台耗时操作 | 线程池 + Redis | ✅ 内置 | 用内置线程池 + `pzredis` 队列 |
| 任务状态追踪 | 进度/重试 | Redis/DB | 🔧 半内置 | DB 状态表 + `pzredis` |
| 分布式锁 | 防重复执行 | Redis SETNX/Redlock | ✅ 内置 | 用 `pzredis` 实现 |
| 工作流引擎 | 审批流/状态机 | 自研 | ➕ 需引入/自研 | 保留自研状态机 |

---

## 七、业务功能模块（均自研，框架不提供）

| 模块 | 说明 |
|------|------|
| 用户管理 | 增删改查、头像、状态、批量导入 |
| 组织架构 | 部门树、岗位、上下级关系 |
| 角色权限 | 菜单/按钮/数据权限（基于框架钩子自研） |
| 操作日志 | 谁在何时做了何操作 |
| 登录日志 | IP/设备/地点/异常告警 |
| 系统配置 | 动态参数、开关、主题 |
| 代码生成器 | **Paozhu 已内置**：`paozhu_cli orm <tag>` + 控制器 codegen 自动生成 |
| 数据导入导出 | Excel 批量导入 + 校验（用 `pzexcel`） |
| 报表统计 | 图表数据接口（用 `pzcharts` 或前端 ECharts） |
| 审批流程 | 请假/报销/OA（自研状态机） |
| 附件管理 | 合同/凭证上传关联（用 `client.files`） |
| 数据看板 | Dashboard 聚合统计 |

---

## 八、运维与监控

| 模块 | 说明 | 原推荐方案 | Paozhu 状态 | 修订建议 |
|------|------|-----------|------------|----------|
| 健康检查 | /health 接口 | 自研 | 🔧 半内置 | 自写路由即可 |
| 性能监控 | QPS/延迟/错误率 | Prometheus | ➕ 需引入 | 保留 `prometheus-cpp` + 自埋点 |
| 链路追踪 | 分布式调用链 | OpenTelemetry | ➕ 需引入 | 保留外部方案 |
| 告警通知 | 钉钉/企微/邮件 | Webhook + HTTP 客户端 | 🔧 半内置 | 用内置 `httpclient.h` + `send_email` |
| 灰度发布 | 流量切分/A-B | Nginx + 自研 | ➕ 需引入 | 保留 Nginx 层 |
| 限流熔断 | 接口限流/降级 | 自研中间件 | 🔧 半内置 | 基于框架钩子 + `pzredis` 自研 |
| 优雅停机 | 信号/连接排空 | 自研 | ✅ 内置 | 用框架内置信号处理（`startup/`） |

---

## 九、API 与接口规范

| 模块 | 说明 | 原推荐方案 | Paozhu 状态 | 修订建议 |
|------|------|-----------|------------|----------|
| RESTful API | 统一 URL/方法 | 框架路由 | ✅ 内置 | `//@urlpath` 注解路由 |
| 统一响应格式 | `{code,msg,data}` | 自研中间件/基类 | 🔧 半内置 | `client.val` + `out_json()` 封装基类 |
| 参数校验 | 必填/类型/范围/正则 | 自研 Validator | 🔧 半内置 | `client.post/get` 取值 + 自研校验 |
| 接口文档 | Swagger/OpenAPI | 自研/手写 YAML | ➕ 需引入 | 保留（框架无内置注解生成） |
| 版本管理 | /api/v1 | 路由前缀 | ✅ 内置 | 路由前缀即可 |
| 分页/排序/过滤 | 通用查询参数 | 自研工具 | 🔧 半内置 | ORM + 自研工具类 |
| 幂等性 | 防重复提交 | Token/唯一键 | 🔧 半内置 | 基于 `pzredis` 唯一键自研 |

---

## 十、前端配套（前后端分离时，与框架无关，保留）

| 模块 | 说明 |
|------|------|
| 管理后台 UI | Vue3 + Element Plus / React + Ant Design |
| 图表 | ECharts / Chart.js（或后端 `pzcharts`） |
| 富文本编辑器 | TinyMCE / WangEditor |
| 文件上传组件 | 分片上传 + 进度条（对应后端 `client.files`） |
| 权限路由 | 动态菜单、按钮级控制 |
| 国际化 | i18n 多语言 |

> Paozhu 额外提供 `theme_view` 多主题机制，前端可配合做 SaaS 多租户样式隔离。

---

## 模块优先级建议（开发顺序，按 Paozhu 内置度重排）

```
第一阶段（MVP，几乎零额外依赖）
├── Paozhu HTTP/路由 + HTTP/2
├── ORM + 连接池 + ORM 二级缓存
├── 用户认证（内置 jwt.h）
├── RBAC 权限（Session + 钩子自研）
├── 统一响应 + 参数校验（client.val/out_json）
└── 内置日志 + ACME/OCSP（HTTPS 免运维）

第二阶段（核心业务，多为内置模块）
├── Excel 导入导出（pzexcel）
├── HTML→PDF（webpdf）/ Word（pzword）
├── 文件上传 / 对象存储（框架 + OSS SDK）
├── 操作日志 / 登录日志
├── 数据字典（DB + pzredis）
└── 定时任务（定时器基座 + 自研）

第三阶段（增强，少量需引入）
├── 邮件（send_email）/ 短信（sms）/ WebSocket（内置）
├── 消息队列 + 异步任务（MQ 需引入）
├── 缓存（pzredis 已内置）
├── 图片处理（pzimage）/ 二维码（内置 qrcode）/ 报表（pzcharts）
├── 工作流 / 审批（自研）
└── 站内信（DB + WebSocket）

第四阶段（运维）
├── 监控 + 告警（Prometheus + httpclient）
├── 限流熔断（pzredis + 中间件）
├── 链路追踪（OpenTelemetry）
├── 灰度发布（Nginx）
└── 自动化部署（Docker / CI/CD）
```

---

## 总结

选用 **Paozhu** 后，原文档"需集成 15~25 个第三方库"的结论不再成立：
- **已内置（无需引入）**：HTTP/HTTP2 服务器、模板、JSON、ini 配置、日志、协程/线程池、JWT、TLS/ACME/OCSP、ORM+连接池、Redis 缓存、ORM 二级缓存、Excel/PDF/Word/图片/压缩包处理、短信、邮件、WebSocket、HTTP 客户端、MQTT 5.0 Broker、多主题/多租户、优雅停机、代码生成器、图形报表。
- **仍需引入/自研**：全文搜索（ES/MeiliSearch）、对象存储 SDK、消息队列（RabbitMQ/Kafka）、gRPC、条码（一维码）、文件预览、Prometheus 监控、OpenTelemetry 链路追踪、Swagger 文档、RBAC/工作流/校验/限流等具体业务规则。

核心难点不在单个模块集成，而在于：统一异常处理、并发安全（连接池/缓存/队列）、可配置化（功能开关热更新）、可观测性（埋点）。
