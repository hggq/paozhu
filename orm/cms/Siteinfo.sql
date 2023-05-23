CREATE TABLE `siteinfo` (
  `sid` int NOT NULL AUTO_INCREMENT,
  `userid` int NOT NULL,
  `sitename` varchar(120) COLLATE utf8mb4_general_ci NOT NULL,
  `sitedomain` varchar(120) COLLATE utf8mb4_general_ci NOT NULL,
  `metakeys` text COLLATE utf8mb4_general_ci NOT NULL,
  `metadesc` text COLLATE utf8mb4_general_ci NOT NULL,
  `copyright` text COLLATE utf8mb4_general_ci NOT NULL,
  PRIMARY KEY (`sid`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci