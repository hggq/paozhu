-- phpMyAdmin SQL Dump
-- version 5.2.0
-- https://www.phpmyadmin.net/
--
-- 主机： localhost
-- 生成日期： 2023-05-24 12:03:17
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
  `topicid` int UNSIGNED NOT NULL COMMENT '栏目id',
  `classtype` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `sortid` int NOT NULL COMMENT '排序',
  `topicname` varchar(140) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
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
  `mdcontent` longtext COLLATE utf8mb4_general_ci NOT NULL COMMENT 'markdown content',
  `isopen` tinyint NOT NULL COMMENT '是否开放',
  `iscomment` tinyint NOT NULL COMMENT '是否可以评论',
  `fromlocal` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '发表地址',
  `texturl` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT 'url用英文代替',
  `summary` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '文章摘要',
  `editauthor` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '文章编辑',
  `relatecontent` varchar(256) COLLATE utf8mb4_general_ci NOT NULL COMMENT '相关内容'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- 转存表中的数据 `article`
--

INSERT INTO `article` (`aid`, `topicid`, `classtype`, `userid`, `sortid`, `topicname`, `title`, `keywords`, `fromsource`, `author`, `addip`, `createtime`, `addtime`, `readnum`, `review`, `icoimg`, `content`, `mdcontent`, `isopen`, `iscomment`, `fromlocal`, `texturl`, `summary`, `editauthor`, `relatecontent`) VALUES
(54, 33, 0, 0, 0, '', '标题标题标题标题标题rrww', '关键词关键词关键词关键词', '来源来源来源来源来源来源', '作者作者作者作者作者rrrrr', '', '', 0, 0, 0, '', '<p>文章内容文章内容文章内容文章内容文章内容文章内容</p><p>文章内容v文章内容</p>', '', 1, 0, '', '', '文章摘要文章摘要文章摘要文章摘要', '', ''),
(55, 35, 0, 0, 0, '', '标题标题标题标题标题66666', '关键词关键词关键词关键词', '来源来源来源来源来源来源11', '作者作者作者作者作者22', '127.0.0.1', '2023-05-16 22:33:18', 1684247598, 0, 0, '', '<p>文章内容文章内容文章内容文章内容文章内容文章内<font color=\"#cc0000\">容文章内容</font></p><p style=\"line-height: 16px;\"><br></p><p><br></p>', '', 1, 0, '', '', '文章摘要文章摘要文章摘要文章摘要', '', ',55');

-- --------------------------------------------------------

--
-- 表的结构 `catalogue`
--

CREATE TABLE `catalogue` (
  `cid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `parentid` int UNSIGNED NOT NULL,
  `name` varchar(64) COLLATE utf8mb4_general_ci NOT NULL COMMENT '分类名称',
  `isview` tinyint NOT NULL COMMENT '是否显示',
  `sortid` int NOT NULL COMMENT '排序',
  `imgurl` varchar(254) COLLATE utf8mb4_general_ci NOT NULL COMMENT '图片'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- 表的结构 `department`
--

CREATE TABLE `department` (
  `dpid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL COMMENT '用户id',
  `parentid` int UNSIGNED NOT NULL COMMENT '父id',
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
-- 表的结构 `product`
--

CREATE TABLE `product` (
  `pid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `topicid` int UNSIGNED NOT NULL COMMENT '栏目id',
  `bigid` int UNSIGNED NOT NULL COMMENT '大类',
  `smallid` int UNSIGNED NOT NULL COMMENT '小类',
  `isview` tinyint NOT NULL COMMENT '是否显示',
  `isstore` tinyint NOT NULL COMMENT '精品',
  `sntype` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '型号',
  `name` varchar(253) COLLATE utf8mb4_general_ci NOT NULL COMMENT '产品名称',
  `introduce` text COLLATE utf8mb4_general_ci NOT NULL COMMENT '介绍',
  `listimg` varchar(254) COLLATE utf8mb4_general_ci NOT NULL COMMENT '列表图片',
  `bigimg` varchar(254) COLLATE utf8mb4_general_ci NOT NULL COMMENT '主图',
  `maincontent` longtext COLLATE utf8mb4_general_ci NOT NULL COMMENT '产品主内容',
  `paracontent` longtext COLLATE utf8mb4_general_ci NOT NULL COMMENT '参数',
  `samepro` varchar(254) COLLATE utf8mb4_general_ci NOT NULL COMMENT '类似',
  `price` int UNSIGNED NOT NULL COMMENT '价格'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='产品内容';

-- --------------------------------------------------------

--
-- 表的结构 `productparam`
--

CREATE TABLE `productparam` (
  `ppid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `pid` int UNSIGNED NOT NULL COMMENT '产品id',
  `imgurl` varchar(254) COLLATE utf8mb4_general_ci NOT NULL COMMENT '图片',
  `price` decimal(10,2) NOT NULL COMMENT '价格',
  `attachfiles` varchar(254) COLLATE utf8mb4_general_ci NOT NULL COMMENT '附件',
  `name` varchar(128) COLLATE utf8mb4_general_ci NOT NULL COMMENT '内容',
  `attachdate` varchar(20) COLLATE utf8mb4_general_ci NOT NULL COMMENT '时间',
  `sortid` int NOT NULL COMMENT '排序',
  `filesize` int UNSIGNED NOT NULL COMMENT '文件大小'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='产品参数';

-- --------------------------------------------------------

--
-- 表的结构 `siteinfo`
--

CREATE TABLE `siteinfo` (
  `sid` int NOT NULL,
  `userid` int NOT NULL,
  `sitename` varchar(120) COLLATE utf8mb4_general_ci NOT NULL,
  `sitedomain` varchar(120) COLLATE utf8mb4_general_ci NOT NULL,
  `metakeys` text COLLATE utf8mb4_general_ci NOT NULL,
  `metadesc` text COLLATE utf8mb4_general_ci NOT NULL,
  `copyright` text COLLATE utf8mb4_general_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- 转存表中的数据 `siteinfo`
--

INSERT INTO `siteinfo` (`sid`, `userid`, `sitename`, `sitedomain`, `metakeys`, `metadesc`, `copyright`) VALUES
(1, 0, '网站名称', '网站域名', '关键词关键词关键词', '描述描述描述qqqqq', '版权信息版权信息版权信息');

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
-- 表的结构 `topic`
--

CREATE TABLE `topic` (
  `topicid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `parentid` int UNSIGNED NOT NULL,
  `cateid` int UNSIGNED NOT NULL COMMENT '类型',
  `isview` tinyint UNSIGNED NOT NULL COMMENT '是否显示',
  `title` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '标题',
  `twotitle` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '第二标题',
  `memo` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '备注',
  `templatename` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '模板名称',
  `url` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '网址',
  `urlpath` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '路径',
  `imgurl` varchar(254) COLLATE utf8mb4_general_ci NOT NULL COMMENT '图片地址',
  `topimg` varchar(254) COLLATE utf8mb4_general_ci NOT NULL COMMENT '头图',
  `accesscode` int UNSIGNED NOT NULL COMMENT '权限代码'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='栏目表';

--
-- 转存表中的数据 `topic`
--

INSERT INTO `topic` (`topicid`, `userid`, `parentid`, `cateid`, `isview`, `title`, `twotitle`, `memo`, `templatename`, `url`, `urlpath`, `imgurl`, `topimg`, `accesscode`) VALUES
(31, 0, 0, 0, 1, '刚刚刚刚刚', '', '啊啊啊', '', '', '啊啊啊', '', '', 0),
(32, 0, 31, 0, 1, 'hhhhhh', '', 'qqqq', '', '', 'eeeee', '', '', 0),
(34, 0, 31, 0, 1, 'tttttt', '', 'yyyyy', '', '', 'qqqq', '', '', 0),
(35, 0, 32, 0, 1, '钱钱ddddd钱钱钱', '', '让肉肉肉肉', '', '', '钱钱钱', '', '', 0);

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
-- 表的索引 `catalogue`
--
ALTER TABLE `catalogue`
  ADD PRIMARY KEY (`cid`);

--
-- 表的索引 `department`
--
ALTER TABLE `department`
  ADD PRIMARY KEY (`dpid`);

--
-- 表的索引 `product`
--
ALTER TABLE `product`
  ADD PRIMARY KEY (`pid`);

--
-- 表的索引 `productparam`
--
ALTER TABLE `productparam`
  ADD PRIMARY KEY (`ppid`);

--
-- 表的索引 `siteinfo`
--
ALTER TABLE `siteinfo`
  ADD PRIMARY KEY (`sid`);

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
-- 表的索引 `topic`
--
ALTER TABLE `topic`
  ADD PRIMARY KEY (`topicid`);

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
  MODIFY `aid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=59;

--
-- 使用表AUTO_INCREMENT `catalogue`
--
ALTER TABLE `catalogue`
  MODIFY `cid` int UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- 使用表AUTO_INCREMENT `department`
--
ALTER TABLE `department`
  MODIFY `dpid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=55;

--
-- 使用表AUTO_INCREMENT `product`
--
ALTER TABLE `product`
  MODIFY `pid` int UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- 使用表AUTO_INCREMENT `productparam`
--
ALTER TABLE `productparam`
  MODIFY `ppid` int UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- 使用表AUTO_INCREMENT `siteinfo`
--
ALTER TABLE `siteinfo`
  MODIFY `sid` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;

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
-- 使用表AUTO_INCREMENT `topic`
--
ALTER TABLE `topic`
  MODIFY `topicid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=38;

--
-- 使用表AUTO_INCREMENT `user`
--
ALTER TABLE `user`
  MODIFY `userid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
