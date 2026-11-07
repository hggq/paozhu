CREATE TABLE `sysuserrole` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int unsigned NOT NULL COMMENT '用户ID，对应 sysuser.adminid',
  `role_id` int unsigned NOT NULL COMMENT '角色ID，对应 sysrole.roleid',
  `company_id` int unsigned NOT NULL COMMENT '所属企业，对应 sysuser.companyid',
  `created_at` int unsigned NOT NULL,
  `created_by` int unsigned NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uniq_user_role` (`user_id`,`role_id`),
  KEY `idx_role_id` (`role_id`),
  KEY `idx_company_id` (`company_id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='用户角色关联表'