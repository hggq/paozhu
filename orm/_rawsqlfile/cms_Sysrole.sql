CREATE TABLE `sysrole` (
  `roleid` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` int unsigned NOT NULL,
  `name` varchar(60) COLLATE utf8mb4_general_ci NOT NULL,
  `status` tinyint unsigned NOT NULL,
  `rolecode` bigint unsigned NOT NULL,
  `sortid` int NOT NULL,
  `rolevalue` int unsigned NOT NULL,
  PRIMARY KEY (`roleid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='角色表'