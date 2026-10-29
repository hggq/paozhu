CREATE TABLE `oauth_client` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `client_id` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '客户端标识，协议层唯一键',
  `client_secret` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL COMMENT '保密客户端的 SHA-256(口令) 小写hex；公开客户端为 NULL',
  `redirect_uris` text CHARACTER SET utf8mb4 COLLATE utf8mb4_bin COMMENT '逗号分隔的回调地址白名单，逐字节精确匹配',
  `name` varchar(128) COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '同意页展示名',
  `is_confidential` tinyint NOT NULL DEFAULT '1' COMMENT '1=保密客户端需要口令 0=公开客户端只靠 PKCE',
  `isopen` tinyint NOT NULL DEFAULT '1' COMMENT '0=停用：授权与令牌端点一律当没有这个客户端（比删行安全，refresh 行还在）',
  `allowed_scopes` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '空格分隔的 scope 白名单；空串=不设上限',
  `grant_types` varchar(127) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT '逗号分隔：authorization_code,refresh_token,client_credentials；空串=三种全开',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_client_id` (`client_id`)
) ENGINE=InnoDB AUTO_INCREMENT=16 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci