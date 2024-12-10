CREATE TABLE `sysperms` (
  `permsid` int unsigned NOT NULL AUTO_INCREMENT,
  `parentid` int unsigned NOT NULL,
  `userid` int unsigned NOT NULL,
  `urlpath` varchar(120) COLLATE utf8mb4_general_ci NOT NULL,
  `urlname` varchar(60) COLLATE utf8mb4_general_ci NOT NULL,
  `name` varchar(60) COLLATE utf8mb4_general_ci NOT NULL,
  `status` tinyint unsigned NOT NULL,
  `permscode` int unsigned NOT NULL,
  `readonly` tinyint unsigned NOT NULL,
  `created_time` int unsigned NOT NULL,
  `updated_time` int unsigned NOT NULL,
  `created_user` int unsigned NOT NULL,
  `updated_user` int unsigned NOT NULL,
  PRIMARY KEY (`permsid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='权限表'