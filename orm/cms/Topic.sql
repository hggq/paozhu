CREATE TABLE `topic` (
  `topicid` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` int unsigned NOT NULL,
  `parentid` int unsigned NOT NULL,
  `cateid` int unsigned NOT NULL COMMENT '类型',
  `isview` tinyint unsigned NOT NULL COMMENT '是否显示',
  `title` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '标题',
  `twotitle` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '第二标题',
  `memo` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '备注',
  `templatename` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '模板名称',
  `url` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '网址',
  `urlpath` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '路径',
  `imgurl` varchar(254) COLLATE utf8mb4_general_ci NOT NULL COMMENT '图片地址',
  `topimg` varchar(254) COLLATE utf8mb4_general_ci NOT NULL COMMENT '头图',
  `accesscode` int unsigned NOT NULL COMMENT '权限代码',
  PRIMARY KEY (`topicid`)
) ENGINE=InnoDB AUTO_INCREMENT=36 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='栏目表'