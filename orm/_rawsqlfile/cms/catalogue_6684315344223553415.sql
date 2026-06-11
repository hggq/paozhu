CREATE TABLE `catalogue` (
  `cid` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` int unsigned NOT NULL,
  `level` int unsigned NOT NULL COMMENT '深度',
  `parentid` int unsigned NOT NULL COMMENT '父id[cid tree]',
  `name` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '分类名称',
  `isview` tinyint NOT NULL COMMENT '是否显示',
  `sortid` int NOT NULL COMMENT '排序',
  `imgurl` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '图片',
  PRIMARY KEY (`cid`)
) ENGINE=InnoDB AUTO_INCREMENT=165 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci