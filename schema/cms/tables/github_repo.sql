CREATE TABLE `github_repo` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `account_id` int unsigned NOT NULL DEFAULT '0' COMMENT 'github_account.id',
  `gh_repo_id` bigint unsigned NOT NULL DEFAULT '0' COMMENT 'GitHub 的 repo id',
  `full_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '' COMMENT 'owner/repo',
  `is_private` tinyint NOT NULL DEFAULT '0',
  `description` varchar(512) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '',
  `primary_language` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '',
  `stars` int unsigned NOT NULL DEFAULT '0',
  `pushed_at` int unsigned NOT NULL DEFAULT '0' COMMENT 'GitHub 报的最后推送时间',
  `synced_at` int unsigned NOT NULL DEFAULT '0' COMMENT '这一行是哪一轮写进来的',
  `is_deleted` tinyint NOT NULL DEFAULT '0' COMMENT '1=本轮同步没拉到（GitHub 侧删除或改私有超出 scope）；软删，下次拉到自动翻回 0',
  `deleted_at` int unsigned NOT NULL DEFAULT '0' COMMENT '标记 is_deleted=1 的时间；0=未删过',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uq_account_repo` (`account_id`,`gh_repo_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci