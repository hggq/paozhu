CREATE TABLE `orm_probe2` (
  `id` int NOT NULL,
  `v` varchar(64) DEFAULT NULL,
  `big_id` bigint unsigned DEFAULT NULL,
  `pad` varchar(2000) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci