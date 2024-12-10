CREATE TABLE `homeblock` (
  `hbid` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` int unsigned NOT NULL,
  `title` varchar(120) COLLATE utf8mb4_general_ci NOT NULL,
  `content` text COLLATE utf8mb4_general_ci NOT NULL,
  `jsonconfig` text COLLATE utf8mb4_general_ci NOT NULL,
  `viewtype` tinyint unsigned NOT NULL COMMENT 'display type sample or list',
  `gettype` tinyint unsigned NOT NULL,
  `rownum` int unsigned NOT NULL,
  `width` int unsigned NOT NULL,
  `height` int unsigned NOT NULL,
  `strlength` int unsigned NOT NULL,
  `sortid` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`hbid`)
) ENGINE=InnoDB AUTO_INCREMENT=16 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='home block content display'