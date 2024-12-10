CREATE TABLE `sysuser` (
  `adminid` int unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `password` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `textword` varchar(40) COLLATE utf8mb4_general_ci NOT NULL,
  `isopen` tinyint(1) NOT NULL,
  `level` int NOT NULL,
  `companyid` int unsigned NOT NULL COMMENT '所属企业',
  `dpid` int unsigned NOT NULL,
  `jobid` int unsigned NOT NULL,
  `roleid` int unsigned NOT NULL,
  `postid` int unsigned NOT NULL,
  `created_at` int unsigned NOT NULL,
  `enddate` int unsigned NOT NULL,
  `qrtemp` int unsigned NOT NULL,
  `gender` tinyint unsigned NOT NULL,
  `nickname` varchar(60) COLLATE utf8mb4_general_ci NOT NULL,
  `realname` varchar(40) COLLATE utf8mb4_general_ci NOT NULL,
  `avatar` varchar(256) COLLATE utf8mb4_general_ci NOT NULL,
  `mobile` varchar(40) COLLATE utf8mb4_general_ci NOT NULL,
  `email` varchar(60) COLLATE utf8mb4_general_ci NOT NULL,
  `wxuuid` varchar(40) COLLATE utf8mb4_general_ci NOT NULL,
  PRIMARY KEY (`adminid`)
) ENGINE=InnoDB AUTO_INCREMENT=10003 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci