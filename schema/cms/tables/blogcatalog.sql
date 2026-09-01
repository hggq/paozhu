CREATE TABLE `blogcatalog` (
  `bid` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` int unsigned NOT NULL,
  `dateid` int unsigned NOT NULL COMMENT '月份聚合',
  `articlenum` int unsigned NOT NULL COMMENT '月份文章数量聚合',
  `languageid` int unsigned NOT NULL COMMENT '语言版本',
  PRIMARY KEY (`bid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='blog文章汇总'