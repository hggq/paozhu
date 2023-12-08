-- phpMyAdmin SQL Dump
-- version 5.2.0
-- https://www.phpmyadmin.net/
--
-- 主机： localhost
-- 生成日期： 2023-12-08 15:13:55
-- 服务器版本： 8.0.28
-- PHP 版本： 8.2.7

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
(1, 1, 0, 1, 1, '', '下载说明', '', '', '', '127.0.0.1', '2023-12-01 14:06:26', 1701410786, 0, 0, '', '<div>github</div><div><br></div>', '', 1, 0, '', 'downloads', '', '', ''),
(2, 2, 0, 1, 2, '', '文档介绍', '', '', '', '127.0.0.1', '2023-12-01 14:07:26', 1701410846, 0, 0, '', '文档介绍<br>', '', 1, 0, '', 'docs', '', '', ''),
(3, 3, 0, 1, 3, '', '博客总结', '', '', '', '127.0.0.1', '2023-12-01 14:08:07', 1701410887, 0, 0, '', '发布，参会<br>', '', 1, 0, '', 'blogs', '', '', ''),
(4, 4, 0, 1, 4, '', '关于我们', '', '', '', '127.0.0.1', '2023-12-01 14:09:04', 1701410944, 0, 0, '', '关于我们内容<br>', '', 1, 0, '', 'about', '', '', ''),
(5, 6, 0, 1, 5, '', '框架简介', '', '', '', '127.0.0.1', '2023-12-02 09:10:33', 1701479433, 0, 0, '', '框架简介<br>', '', 1, 0, '', 'docs_introduce', '', '', ''),
(6, 3, 0, 1, 6, '', '博客发布', '', '', '', '127.0.0.1', '2023-12-02 09:12:31', 1701479551, 0, 0, '', '博客发布release<br>', '', 1, 0, '', 'blog_release', '', '', ''),
(7, 3, 0, 1, 7, '', '博客发布2', '', '', '', '127.0.0.1', '2023-12-02 09:13:46', 1701479626, 0, 0, '', '博客发布2<br>', '', 1, 0, '', 'blog_release2', '', '', '');

-- --------------------------------------------------------

--
-- 表的结构 `blogcatalog`
--

CREATE TABLE `blogcatalog` (
  `bid` int UNSIGNED NOT NULL,
  `dateid` int UNSIGNED NOT NULL COMMENT '月份聚合',
  `articlenum` int UNSIGNED NOT NULL COMMENT '月份文章数量聚合',
  `languageid` int UNSIGNED NOT NULL COMMENT '语言版本'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='blog文章汇总';

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
(54, 0, 51, 'bbcc777', '', 30, 0, 0, 1, 'bbcc11 desc', 0, 0, 0, '');

-- --------------------------------------------------------

--
-- 表的结构 `logininfo`
--

CREATE TABLE `logininfo` (
  `lgid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL COMMENT '会员id',
  `username` varchar(40) COLLATE utf8mb4_general_ci NOT NULL COMMENT '会员名字',
  `addtime` varchar(20) COLLATE utf8mb4_general_ci NOT NULL COMMENT '登录时间',
  `addip` varchar(70) COLLATE utf8mb4_general_ci NOT NULL COMMENT '登录ip',
  `addregion` varchar(70) COLLATE utf8mb4_general_ci NOT NULL COMMENT '登录地区',
  `loginstate` varchar(10) COLLATE utf8mb4_general_ci NOT NULL COMMENT '登录状态'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='登录记录';

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
  `languagetype` int UNSIGNED NOT NULL COMMENT '语言类型',
  `sitename` varchar(120) COLLATE utf8mb4_general_ci NOT NULL,
  `sitedomain` varchar(120) COLLATE utf8mb4_general_ci NOT NULL,
  `metakeys` text COLLATE utf8mb4_general_ci NOT NULL,
  `metadesc` text COLLATE utf8mb4_general_ci NOT NULL,
  `copyright` text COLLATE utf8mb4_general_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- 转存表中的数据 `siteinfo`
--

INSERT INTO `siteinfo` (`sid`, `userid`, `languagetype`, `sitename`, `sitedomain`, `metakeys`, `metadesc`, `copyright`) VALUES
(1, 1, 0, '网站名称ww', '网站域名', '88关键词关键词关键词', '描述描述描述', '版权信息版权信息版权信息');

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
  `languagetype` int UNSIGNED NOT NULL COMMENT '语言类型',
  `isview` tinyint UNSIGNED NOT NULL COMMENT '是否显示',
  `sortid` int UNSIGNED NOT NULL COMMENT '排序id',
  `title` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '标题',
  `twotitle` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '第二标题',
  `memo` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '备注',
  `templatename` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '模板名称',
  `url` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '网址',
  `urlpath` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '路径',
  `imgurl` varchar(254) COLLATE utf8mb4_general_ci NOT NULL COMMENT '图片地址',
  `topimg` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '头图',
  `accesscode` int UNSIGNED NOT NULL COMMENT '权限代码'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='栏目表';

--
-- 转存表中的数据 `topic`
--

INSERT INTO `topic` (`topicid`, `userid`, `parentid`, `cateid`, `languagetype`, `isview`, `sortid`, `title`, `twotitle`, `memo`, `templatename`, `url`, `urlpath`, `imgurl`, `topimg`, `accesscode`) VALUES
(1, 1, 0, 0, 0, 1, 1, 'Download', '', '下载3', '', '', 'downloads', '', '', 0),
(2, 1, 0, 4, 0, 1, 2, 'Docs', '', '', '', '', 'docs', '', '', 0),
(3, 1, 0, 13, 0, 1, 3, 'Blog', '', '', '', '', 'blogs', '', '', 0),
(4, 1, 0, 1, 0, 1, 4, 'About', '', '', '', '', 'about', '', '', 0),
(5, 1, 2, 0, 0, 1, 21, '入门概述', '', '', '', '', 'introduce', '', '', 0),
(6, 1, 5, 0, 0, 1, 21, '框架简介', '', '', '', '', 'framework', '', '', 0),
(7, 1, 5, 0, 0, 1, 22, 'HelloWorld', '', '', '', '', 'helloworld', '', '', 0),
(8, 1, 5, 0, 0, 1, 23, 'Ubuntu安装', '', '', '', '', 'ubuntu_install', '', '', 0),
(9, 1, 5, 0, 0, 1, 24, 'MacOS安装', '', '', '', '', 'macos_install', '', '', 0),
(10, 1, 5, 0, 0, 1, 25, 'RedHat安装', '', '', '', '', 'redhat_install', '', '', 0),
(11, 1, 5, 0, 0, 1, 26, 'Windows安装', '', '', '', '', 'windows_install', '', '', 0),
(12, 1, 5, 0, 0, 1, 0, '框架功能详细', '', '', '', '', 'framework_fun', '', '', 0),
(13, 1, 2, 0, 0, 1, 31, '框架功能', '', '', '', '', 'framefuns', '', '', 0),
(14, 1, 13, 0, 0, 1, 0, '业务代码入口', '', '', '', '', 'urlenters', '', '', 0),
(15, 1, 13, 0, 0, 1, 0, '静态文件浏览', '', '', '', '', 'staticfiles', '', '', 0),
(16, 1, 13, 0, 0, 1, 0, '框架配置conf', '', '', '', '', 'frameconf', '', '', 0),
(17, 1, 2, 0, 0, 1, 40, 'ORM功能', '', '', '', '', 'ormfun', '', '', 0),
(18, 1, 2, 0, 0, 1, 50, 'HttpClient', '', '', '', '', 'httpclients', '', '', 0),
(19, 1, 2, 0, 0, 1, 60, 'WebSocket介绍', '', '', '', '', 'websockets', '', '', 0),
(20, 1, 19, 0, 0, 1, 0, 'WebSocket入门', '', '', '', '', 'websocket_introduce', '', '', 0),
(21, 1, 18, 0, 0, 1, 0, 'HttpClient入门', '', '', '', '', 'httpclient_introduce', '', '', 0),
(22, 1, 17, 0, 0, 1, 0, 'ORM入门', '', '', '', '', 'orm_introduce', '', '', 0);

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
-- 表的索引 `blogcatalog`
--
ALTER TABLE `blogcatalog`
  ADD PRIMARY KEY (`bid`);

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
-- 表的索引 `logininfo`
--
ALTER TABLE `logininfo`
  ADD PRIMARY KEY (`lgid`);

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
  MODIFY `aid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=8;

--
-- 使用表AUTO_INCREMENT `blogcatalog`
--
ALTER TABLE `blogcatalog`
  MODIFY `bid` int UNSIGNED NOT NULL AUTO_INCREMENT;

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
-- 使用表AUTO_INCREMENT `logininfo`
--
ALTER TABLE `logininfo`
  MODIFY `lgid` int UNSIGNED NOT NULL AUTO_INCREMENT;

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
  MODIFY `topicid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=23;

--
-- 使用表AUTO_INCREMENT `user`
--
ALTER TABLE `user`
  MODIFY `userid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
