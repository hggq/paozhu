CREATE TABLE `department` (
  `dpid` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` int NOT NULL COMMENT '用户id',
  `parentid` int NOT NULL COMMENT '父id',
  `name` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '部门名称',
  `depart_code` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '部门代号',
  `bianzhi_num` int NOT NULL COMMENT '编制人数',
  `real_num` int NOT NULL COMMENT '实际人数',
  `quan_weight` int NOT NULL COMMENT '部门权重',
  `isopen` tinyint NOT NULL COMMENT '是否开放',
  `memo` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '备注',
  `addtime` int unsigned NOT NULL COMMENT '添加时间',
  `edittime` int unsigned NOT NULL COMMENT '修改时间',
  `isvirtual` tinyint NOT NULL COMMENT '是否虚拟部门',
  `linkdpid` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '关联部门',
  PRIMARY KEY (`dpid`)
) ENGINE=InnoDB AUTO_INCREMENT=55 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='部门'