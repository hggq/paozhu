CREATE TABLE `xtalk` (
  `talkid` int unsigned NOT NULL AUTO_INCREMENT,
  `userid` int unsigned NOT NULL,
  `adminid` int unsigned NOT NULL,
  `taskid` int unsigned NOT NULL,
  `projectid` int unsigned NOT NULL,
  `content` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `isdelete` tinyint unsigned NOT NULL,
  `addtime` int unsigned NOT NULL,
  `update_at` int unsigned NOT NULL,
  `islock` tinyint unsigned NOT NULL,
  `replyid` int unsigned NOT NULL,
  PRIMARY KEY (`talkid`),
  KEY `userid` (`userid`),
  KEY `projectid` (`projectid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='任务讨论'