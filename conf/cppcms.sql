-- phpMyAdmin SQL Dump
-- version 5.2.0
-- https://www.phpmyadmin.net/
--
-- 主机： localhost
-- 生成日期： 2023-04-26 15:07:27
-- 服务器版本： 8.0.28
-- PHP 版本： 8.2.4

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- 数据库： `cppcms`
--

-- --------------------------------------------------------

--
-- 表的结构 `article`
--

CREATE TABLE `article` (
  `aid` int UNSIGNED NOT NULL,
  `classtype` int NOT NULL,
  `userid` int NOT NULL,
  `topicname` varchar(40) COLLATE utf8mb4_general_ci DEFAULT NULL,
  `title` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `keywords` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '关键字',
  `fromsource` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '文章来源',
  `author` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '文章作者',
  `addip` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `createtime` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '显示的创建时间',
  `addtime` bigint UNSIGNED NOT NULL COMMENT '添加或修改时间',
  `readnum` int NOT NULL,
  `review` int NOT NULL,
  `icoimg` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '列表图片',
  `content` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `isopen` tinyint NOT NULL COMMENT '是否开放',
  `iscomment` tinyint NOT NULL COMMENT '是否可以评论',
  `fromlocal` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '发表地址',
  `texturl` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT 'url用英文代替',
  `summary` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '文章摘要',
  `editauthor` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '文章编辑'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- 转存表中的数据 `article`
--

INSERT INTO `article` (`aid`, `classtype`, `userid`, `topicname`, `title`, `keywords`, `fromsource`, `author`, `addip`, `createtime`, `addtime`, `readnum`, `review`, `icoimg`, `content`, `isopen`, `iscomment`, `fromlocal`, `texturl`, `summary`, `editauthor`) VALUES
(4, 0, 0, NULL, '1111', '', '', '', '', '', 0, 0, 0, '', '2222', 1, 0, '', '', '', ''),
(5, 0, 0, NULL, '理解8888完这', '', '', '', '127.0.0.1', '2023-01-09 22:18:17', 1673273897, 0, 0, '', 'sadfafa点点滴滴ggg9999', 1, 0, '', '', '', '');

-- --------------------------------------------------------

--
-- 表的结构 `department`
--

CREATE TABLE `department` (
  `dpid` int UNSIGNED NOT NULL,
  `userid` int NOT NULL COMMENT '用户id',
  `parentid` int NOT NULL COMMENT '父id',
  `name` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '部门名称',
  `depart_code` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '部门代号',
  `bianzhi_num` int NOT NULL COMMENT '编制人数',
  `real_num` int NOT NULL COMMENT '实际人数',
  `quan_weight` int NOT NULL COMMENT '部门权重',
  `isopen` tinyint NOT NULL COMMENT '是否开放',
  `memo` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '备注',
  `addtime` int UNSIGNED NOT NULL COMMENT '添加时间',
  `edittime` int UNSIGNED NOT NULL COMMENT '修改时间',
  `isvirtual` tinyint NOT NULL COMMENT '是否虚拟部门',
  `linkdpid` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '关联部门'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='部门';

--
-- 转存表中的数据 `department`
--

INSERT INTO `department` (`dpid`, `userid`, `parentid`, `name`, `depart_code`, `bianzhi_num`, `real_num`, `quan_weight`, `isopen`, `memo`, `addtime`, `edittime`, `isvirtual`, `linkdpid`) VALUES
(38, 0, 0, 'aabb', '', 11, 0, 0, 1, 'aa desc', 0, 0, 0, ''),
(50, 0, 0, 'bbcc', '', 110, 0, 0, 1, 'bb desc', 0, 0, 0, ''),
(51, 0, 50, 'bbcc11', '', 66, 0, 0, 1, 'bbcc desc', 0, 0, 0, ''),
(52, 0, 0, 'ccdd', '', 20, 0, 0, 1, 'cc desc', 0, 0, 0, ''),
(53, 0, 52, 'ccddee', '', 10, 0, 0, 1, 'ccdd desc', 0, 0, 0, ''),
(54, 0, 51, 'bbcc1122', '', 30, 0, 0, 1, 'bbcc11 desc', 0, 0, 0, '');

-- --------------------------------------------------------

--
-- 表的结构 `testa`
--

CREATE TABLE `testa` (
  `id` int NOT NULL,
  `value` tinyint NOT NULL,
  `content` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- 转存表中的数据 `testa`
--

INSERT INTO `testa` (`id`, `value`, `content`) VALUES
(1, 65, 'commit test'),
(5, 65, 'commit test');

-- --------------------------------------------------------

--
-- 表的结构 `testb`
--

CREATE TABLE `testb` (
  `tid` int NOT NULL,
  `name` varchar(30) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- 转存表中的数据 `testb`
--

INSERT INTO `testb` (`tid`, `name`) VALUES
(2, 'caname'),
(4, 'caname');

-- --------------------------------------------------------

--
-- 表的结构 `user`
--

CREATE TABLE `user` (
  `userid` int UNSIGNED NOT NULL,
  `name` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `password` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `isopen` tinyint(1) NOT NULL,
  `level` int NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- 转存表中的数据 `user`
--

INSERT INTO `user` (`userid`, `name`, `password`, `isopen`, `level`) VALUES
(1, 'admin', 'e10adc3949ba59abbe56e057f20f883e', 1, 191);

--
-- 转储表的索引
--

--
-- 表的索引 `article`
--
ALTER TABLE `article`
  ADD PRIMARY KEY (`aid`);

--
-- 表的索引 `department`
--
ALTER TABLE `department`
  ADD PRIMARY KEY (`dpid`);

--
-- 表的索引 `testa`
--
ALTER TABLE `testa`
  ADD PRIMARY KEY (`id`);

--
-- 表的索引 `testb`
--
ALTER TABLE `testb`
  ADD PRIMARY KEY (`tid`);

--
-- 表的索引 `user`
--
ALTER TABLE `user`
  ADD PRIMARY KEY (`userid`);

--
-- 在导出的表使用AUTO_INCREMENT
--

--
-- 使用表AUTO_INCREMENT `article`
--
ALTER TABLE `article`
  MODIFY `aid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=52;

--
-- 使用表AUTO_INCREMENT `department`
--
ALTER TABLE `department`
  MODIFY `dpid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=55;

--
-- 使用表AUTO_INCREMENT `testa`
--
ALTER TABLE `testa`
  MODIFY `id` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=9;

--
-- 使用表AUTO_INCREMENT `testb`
--
ALTER TABLE `testb`
  MODIFY `tid` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;

--
-- 使用表AUTO_INCREMENT `user`
--
ALTER TABLE `user`
  MODIFY `userid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
