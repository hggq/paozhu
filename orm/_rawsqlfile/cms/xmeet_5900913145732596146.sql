CREATE TABLE `xmeet` (
  `xmeetid` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` int unsigned NOT NULL,
  `xpjid` int unsigned NOT NULL,
  `xtaskid` int unsigned NOT NULL,
  `adminid` int unsigned NOT NULL,
  `title` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `zhuchi` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `jilu` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `meetnotice` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `meetfiles` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `address` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `meettime` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `regdate` int unsigned NOT NULL,
  `expecttime` int unsigned NOT NULL,
  `endtime` int unsigned NOT NULL,
  `presents` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `content` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `postresult` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `postfiles` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `jiluphoto` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  PRIMARY KEY (`xmeetid`),
  KEY `userid` (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='会议纪要'