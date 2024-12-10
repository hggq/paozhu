CREATE TABLE `superadmin` (
  `adminid` int unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `password` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `nickname` varchar(30) COLLATE utf8mb4_general_ci NOT NULL,
  `isopen` tinyint NOT NULL,
  `begindate` int unsigned NOT NULL,
  `enddate` int unsigned NOT NULL,
  `regdate` varchar(30) COLLATE utf8mb4_general_ci NOT NULL,
  `mobile` varchar(40) COLLATE utf8mb4_general_ci NOT NULL,
  `email` varchar(40) COLLATE utf8mb4_general_ci NOT NULL,
  `loginnum` int unsigned NOT NULL,
  `qrtemp` int unsigned NOT NULL,
  `wxuuid` varchar(40) COLLATE utf8mb4_general_ci NOT NULL,
  `basesitepath` varchar(256) COLLATE utf8mb4_general_ci NOT NULL,
  PRIMARY KEY (`adminid`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='Super administrator'