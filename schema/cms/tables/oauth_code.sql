CREATE TABLE `oauth_code` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `code_hash` char(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT 'SHA-256(授权码) 小写hex，明文码不落库',
  `client_id` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '签发目标客户端',
  `user_id` bigint NOT NULL COMMENT '授权人=sysuser.adminid（不是 companyid）',
  `redirect_uri` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '授权时登记的回调地址，换码时逐字节比对',
  `scope` varchar(256) COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '授权范围，空格分隔',
  `code_challenge` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT 'PKCE S256 挑战值，只支持 S256',
  `expires_at` bigint NOT NULL COMMENT '过期 Unix 秒',
  `used` tinyint NOT NULL DEFAULT '0' COMMENT '0=未消费 1=已消费，一次性',
  `family_id` int unsigned NOT NULL DEFAULT '0' COMMENT '换码成功时回填的 refresh 族根 id；0=还没换过码或回填失败',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_code_hash` (`code_hash`),
  KEY `idx_client` (`client_id`),
  KEY `idx_expires` (`expires_at`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci