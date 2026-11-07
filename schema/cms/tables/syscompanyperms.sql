CREATE TABLE `syscompanyperms` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `company_id` int unsigned NOT NULL COMMENT '所属企业，对应 sysuser.companyid / siteinfo.sid',
  `permsid` int unsigned NOT NULL COMMENT '权限项ID，对应 sysperms.permsid',
  `created_at` int unsigned NOT NULL,
  `created_by` int unsigned NOT NULL COMMENT '执行分配的 superadmin id，0=系统/回填',
  PRIMARY KEY (`id`),
  UNIQUE KEY `uniq_company_perms` (`company_id`,`permsid`),
  KEY `idx_permsid` (`permsid`)
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='企业权限分配表'