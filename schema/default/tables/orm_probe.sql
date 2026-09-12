CREATE TABLE `orm_probe` (
  `id` int NOT NULL,
  `v` varchar(64) DEFAULT NULL,
  `dec_col` decimal(20,4) DEFAULT NULL,
  `t_col` time DEFAULT NULL,
  `dt_col` datetime(6) DEFAULT NULL,
  `big` longblob,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci