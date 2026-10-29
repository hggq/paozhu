CREATE TABLE `oauth_refresh` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `family_id` int unsigned NOT NULL DEFAULT '0' COMMENT '轮换族根：首次签发那一行的 id，重放时按族整批作废',
  `token_hash` char(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT 'SHA-256(refresh token) 小写hex，明文只发一次',
  `client_id` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '所属客户端',
  `user_id` bigint NOT NULL COMMENT '授权人=sysuser.adminid（不是 companyid）',
  `scope` varchar(256) COLLATE utf8mb4_general_ci NOT NULL DEFAULT '' COMMENT '轮换时保持不变，禁止收缩',
  `expires_at` bigint NOT NULL COMMENT '过期 Unix 秒',
  `revoked` tinyint NOT NULL DEFAULT '0' COMMENT '0=有效 1=已撤销',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_token_hash` (`token_hash`),
  KEY `idx_family` (`family_id`),
  KEY `idx_user` (`client_id`,`user_id`),
  KEY `idx_expires` (`expires_at`)
) ENGINE=InnoDB AUTO_INCREMENT=16 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci