CREATE TABLE `topicset` (
  `topicsetid` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` int unsigned NOT NULL,
  `topicid` int unsigned NOT NULL,
  `linktopicid` int unsigned NOT NULL,
  `linkrownum` tinyint unsigned NOT NULL,
  `sidetype` tinyint unsigned NOT NULL COMMENT 'showtype',
  `sidename` varchar(80) COLLATE utf8mb4_general_ci NOT NULL,
  `txtcontent` text COLLATE utf8mb4_general_ci NOT NULL,
  `sort` int unsigned NOT NULL,
  PRIMARY KEY (`topicsetid`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='边栏设置内容 Sidebar settings content'