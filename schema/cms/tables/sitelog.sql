CREATE TABLE `sitelog` (
  `logid` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` int unsigned NOT NULL,
  `memberid` int unsigned NOT NULL,
  `ipport` int unsigned NOT NULL,
  `httpv` tinyint unsigned NOT NULL COMMENT 'https http2',
  `ipaddress` varchar(60) COLLATE utf8mb4_general_ci NOT NULL,
  `visittime` varchar(30) COLLATE utf8mb4_general_ci NOT NULL,
  `useragent` varchar(512) COLLATE utf8mb4_general_ci NOT NULL,
  `referer` varchar(512) COLLATE utf8mb4_general_ci NOT NULL,
  `cururl` varchar(512) COLLATE utf8mb4_general_ci NOT NULL,
  `address` varchar(120) COLLATE utf8mb4_general_ci NOT NULL,
  `hostname` varchar(60) COLLATE utf8mb4_general_ci NOT NULL,
  `derefererurl` varchar(512) COLLATE utf8mb4_general_ci NOT NULL,
  `deurl` varchar(512) COLLATE utf8mb4_general_ci NOT NULL,
  PRIMARY KEY (`logid`),
  KEY `userid` (`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='visitlog'