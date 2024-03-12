CREATE TABLE `testa` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `parentid` int unsigned NOT NULL COMMENT '父id[id tree]',
  `value_id` tinyint DEFAULT NULL,
  `content` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `deleted` tinyint unsigned NOT NULL,
  `deletetime` int unsigned NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=35 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci