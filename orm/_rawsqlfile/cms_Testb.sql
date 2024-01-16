CREATE TABLE `testb` (
  `tid` int NOT NULL AUTO_INCREMENT,
  `score` bigint NOT NULL DEFAULT '0' COMMENT '分数[num*100]',
  `name` varchar(30) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  PRIMARY KEY (`tid`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci