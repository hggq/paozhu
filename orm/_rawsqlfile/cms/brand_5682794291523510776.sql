CREATE TABLE `brand` (
  `brandid` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` int unsigned NOT NULL,
  `topicid` int unsigned NOT NULL,
  `sortid` int NOT NULL COMMENT '排序',
  `logo` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `title` varchar(160) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '品牌名称brandname',
  `name` varchar(160) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT 'other name',
  `introduce` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '简介',
  `detailcontent` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  PRIMARY KEY (`brandid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='品牌介绍'