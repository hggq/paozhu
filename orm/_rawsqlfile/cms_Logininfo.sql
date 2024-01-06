CREATE TABLE `logininfo` (
  `lgid` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` int unsigned NOT NULL COMMENT '会员id',
  `username` varchar(40) COLLATE utf8mb4_general_ci NOT NULL COMMENT '会员名字',
  `addtime` varchar(20) COLLATE utf8mb4_general_ci NOT NULL COMMENT '登录时间',
  `addip` varchar(70) COLLATE utf8mb4_general_ci NOT NULL COMMENT '登录ip',
  `addregion` varchar(70) COLLATE utf8mb4_general_ci NOT NULL COMMENT '登录地区',
  `loginstate` varchar(10) COLLATE utf8mb4_general_ci NOT NULL COMMENT '登录状态',
  PRIMARY KEY (`lgid`)
) ENGINE=InnoDB AUTO_INCREMENT=32 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='登录记录'