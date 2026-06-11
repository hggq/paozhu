CREATE TABLE `sysroleperms` (
  `rolepermsid` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` int unsigned NOT NULL,
  `roleid` int unsigned NOT NULL,
  `permsid` int unsigned NOT NULL,
  `sortid` int unsigned NOT NULL,
  PRIMARY KEY (`rolepermsid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci