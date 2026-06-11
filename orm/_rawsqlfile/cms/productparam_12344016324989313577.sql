CREATE TABLE `productparam` (
  `ppid` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` int unsigned NOT NULL,
  `pid` int unsigned NOT NULL COMMENT '产品id',
  `imgurl` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '图片',
  `price` bigint unsigned NOT NULL COMMENT '价格',
  `attachfiles` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '附件',
  `name` varchar(160) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '内容',
  `attachdate` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '时间',
  `sortid` int NOT NULL COMMENT '排序',
  `filesize` int unsigned NOT NULL COMMENT '文件大小',
  `fileext` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '文件扩展名',
  PRIMARY KEY (`ppid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='产品参数'