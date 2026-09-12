CREATE TABLE `fk_child` (
  `id` int NOT NULL AUTO_INCREMENT,
  `parent_id` int DEFAULT NULL,
  `extra` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_child_ibfk_1` (`parent_id`),
  CONSTRAINT `fk_child_ibfk_1` FOREIGN KEY (`parent_id`) REFERENCES `fk_parent` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci