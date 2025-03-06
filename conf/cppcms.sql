-- phpMyAdmin SQL Dump
-- version 5.2.2
-- https://www.phpmyadmin.net/
--
-- 主机： localhost
-- 生成日期： 2025-03-05 09:19:28
-- 服务器版本： 8.4.4
-- PHP 版本： 8.4.3

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
  `mdcontent` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT 'markdown content',
  `isopen` tinyint NOT NULL COMMENT '是否开放',
  `ishome` tinyint NOT NULL DEFAULT '0' COMMENT 'show homepage',
  `iscomment` tinyint NOT NULL COMMENT '是否可以评论',
  `showtype` tinyint NOT NULL DEFAULT '0' COMMENT 'art content display type',
  `fromlocal` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '发表地址',
  `texturl` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT 'url用英文代替',
  `summary` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '文章摘要',
  `editauthor` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '文章编辑',
  `relatecontent` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '相关内容'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- 转存表中的数据 `article`
--

INSERT INTO `article` (`aid`, `topicid`, `classtype`, `userid`, `sortid`, `topicname`, `title`, `keywords`, `fromsource`, `author`, `addip`, `createtime`, `addtime`, `readnum`, `review`, `icoimg`, `content`, `mdcontent`, `isopen`, `ishome`, `iscomment`, `showtype`, `fromlocal`, `texturl`, `summary`, `editauthor`, `relatecontent`) VALUES
(1, 1, 0, 1, 0, '', '我公司典型案例', '', '', '管理员', '113.17.101.12', '2024-11-05 21:54:59', 1730814899, 0, 0, '', '<p>国际大学生</p>', '', 1, 0, 0, 0, '', '', '', '', '');

-- --------------------------------------------------------

--
-- 表的结构 `blogcatalog`
--

CREATE TABLE `blogcatalog` (
  `bid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `dateid` int UNSIGNED NOT NULL COMMENT '月份聚合',
  `articlenum` int UNSIGNED NOT NULL COMMENT '月份文章数量聚合',
  `languageid` int UNSIGNED NOT NULL COMMENT '语言版本'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='blog文章汇总';

-- --------------------------------------------------------

--
-- 表的结构 `brand`
--

CREATE TABLE `brand` (
  `brandid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `topicid` int UNSIGNED NOT NULL,
  `sortid` int NOT NULL COMMENT '排序',
  `logo` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `title` varchar(160) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '品牌名称brandname',
  `name` varchar(160) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT 'other name',
  `introduce` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '简介',
  `detailcontent` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='品牌介绍';

-- --------------------------------------------------------

--
-- 表的结构 `catalogue`
--

CREATE TABLE `catalogue` (
  `cid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `level` int UNSIGNED NOT NULL COMMENT '深度',
  `parentid` int UNSIGNED NOT NULL COMMENT '父id[cid tree]',
  `name` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '分类名称',
  `isview` tinyint NOT NULL COMMENT '是否显示',
  `sortid` int NOT NULL COMMENT '排序',
  `imgurl` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '图片'
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
  `created_time` int UNSIGNED NOT NULL COMMENT '添加时间',
  `created_user` int UNSIGNED NOT NULL,
  `updated_time` int UNSIGNED NOT NULL COMMENT '修改时间',
  `updated_user` int UNSIGNED NOT NULL,
  `isvirtual` tinyint NOT NULL COMMENT '是否虚拟部门',
  `linkdpid` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '关联部门'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='部门';

--
-- 转存表中的数据 `department`
--

INSERT INTO `department` (`dpid`, `userid`, `parentid`, `name`, `depart_code`, `bianzhi_num`, `real_num`, `quan_weight`, `isopen`, `memo`, `created_time`, `created_user`, `updated_time`, `updated_user`, `isvirtual`, `linkdpid`) VALUES
(38, 0, 0, 'aabb', '', 11, 0, 0, 1, 'aa desc', 0, 0, 0, 0, 0, ''),
(50, 0, 0, 'bbcc', '', 110, 0, 0, 1, 'bb desc', 0, 0, 0, 0, 0, ''),
(51, 0, 50, 'bbcc11', '', 66, 0, 0, 1, 'bbcc desc', 0, 0, 0, 0, 0, ''),
(52, 0, 0, 'ccdd', '', 20, 0, 0, 1, 'cc desc', 0, 0, 0, 0, 0, ''),
(53, 0, 52, 'ccddee', '', 10, 0, 0, 1, 'ccdd desc', 0, 0, 0, 0, 0, ''),
(54, 0, 51, 'bbcc777', '', 30, 0, 0, 1, 'bbcc11 desc', 0, 0, 0, 0, 0, '');

-- --------------------------------------------------------

--
-- 表的结构 `homeblock`
--

CREATE TABLE `homeblock` (
  `hbid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `title` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `content` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `jsonconfig` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `viewtype` tinyint UNSIGNED NOT NULL COMMENT 'display type sample or list',
  `gettype` tinyint UNSIGNED NOT NULL,
  `rownum` int UNSIGNED NOT NULL,
  `width` int UNSIGNED NOT NULL,
  `height` int UNSIGNED NOT NULL,
  `strlength` int UNSIGNED NOT NULL,
  `sortid` int NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='home block content display';

-- --------------------------------------------------------

--
-- 表的结构 `logininfo`
--

CREATE TABLE `logininfo` (
  `lgid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL COMMENT '会员id',
  `logtype` tinyint UNSIGNED NOT NULL,
  `username` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '会员名字',
  `addtime` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '登录时间',
  `addip` varchar(70) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '登录ip',
  `addregion` varchar(70) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '登录地区',
  `loginstate` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '登录状态',
  `agent` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `urlpath` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='登录记录';

--
-- 转存表中的数据 `logininfo`
--

INSERT INTO `logininfo` (`lgid`, `userid`, `logtype`, `username`, `addtime`, `addip`, `addregion`, `loginstate`, `agent`, `urlpath`) VALUES
(1, 10000, 0, 'admin', '2025-03-05 17:19:12', '127.0.0.1', '', '成功default', '', '');

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
  `brandid` int UNSIGNED NOT NULL COMMENT '品牌id',
  `isview` tinyint NOT NULL COMMENT '是否显示',
  `isstore` tinyint NOT NULL COMMENT '精品',
  `ishome` tinyint UNSIGNED NOT NULL COMMENT '是否首页',
  `showtype` tinyint NOT NULL COMMENT 'page layout',
  `sntype` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '型号',
  `name` varchar(253) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '产品名称',
  `keywords` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `introduce` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '介绍',
  `listimg` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '列表图片',
  `bigimg` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '主图',
  `maincontent` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '产品主内容',
  `paracontent` longtext CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '参数',
  `samepro` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '类似',
  `attatchfiles` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '附件',
  `price` int UNSIGNED NOT NULL COMMENT '价格',
  `sortid` int UNSIGNED NOT NULL,
  `adddate` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '产品添加日期',
  `editdate` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '产品修改时间'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='产品内容';

-- --------------------------------------------------------

--
-- 表的结构 `productparam`
--

CREATE TABLE `productparam` (
  `ppid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `pid` int UNSIGNED NOT NULL COMMENT '产品id',
  `imgurl` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '图片',
  `price` bigint UNSIGNED NOT NULL COMMENT '价格',
  `attachfiles` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '附件',
  `name` varchar(160) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '内容',
  `attachdate` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '时间',
  `sortid` int NOT NULL COMMENT '排序',
  `filesize` int UNSIGNED NOT NULL COMMENT '文件大小',
  `fileext` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '文件扩展名'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='产品参数';

-- --------------------------------------------------------

--
-- 表的结构 `siteinfo`
--

CREATE TABLE `siteinfo` (
  `sid` int NOT NULL,
  `userid` int NOT NULL,
  `agentid` int UNSIGNED NOT NULL,
  `languagetype` int UNSIGNED NOT NULL COMMENT '语言类型',
  `sitename` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `sitedomain` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `metakeys` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `metadesc` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `copyright` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `beiansn` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '备案号',
  `footscript` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '底部脚本',
  `headscript` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '头部script',
  `introduce` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `sitelogo` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT 'logo url',
  `contactman` varchar(30) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `phone` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `mobile` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `email` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `bankname` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `banksn` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `address` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `zipnum` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `taxsn` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `companyname` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `linkname` varchar(30) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `linkmobile` varchar(30) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `linkaddress` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `theme` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `sitepath` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `isopen` tinyint UNSIGNED NOT NULL,
  `created_at` int UNSIGNED NOT NULL,
  `enddate` int UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- 转存表中的数据 `siteinfo`
--

INSERT INTO `siteinfo` (`sid`, `userid`, `agentid`, `languagetype`, `sitename`, `sitedomain`, `metakeys`, `metadesc`, `copyright`, `beiansn`, `footscript`, `headscript`, `introduce`, `sitelogo`, `contactman`, `phone`, `mobile`, `email`, `bankname`, `banksn`, `address`, `zipnum`, `taxsn`, `companyname`, `linkname`, `linkmobile`, `linkaddress`, `theme`, `sitepath`, `isopen`, `created_at`, `enddate`) VALUES
(1, 1, 1, 0, '名称名称名称', '', 'META关键词aaayyy', 'META描述cccfddd\r\n', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', 'default', 0, 0, 0);

-- --------------------------------------------------------

--
-- 表的结构 `superadmin`
--

CREATE TABLE `superadmin` (
  `adminid` int UNSIGNED NOT NULL,
  `name` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `password` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `nickname` varchar(30) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `isopen` tinyint NOT NULL,
  `begindate` int UNSIGNED NOT NULL,
  `enddate` int UNSIGNED NOT NULL,
  `regdate` varchar(30) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `mobile` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `email` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `loginnum` int UNSIGNED NOT NULL,
  `qrtemp` int UNSIGNED NOT NULL,
  `wxuuid` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `basesitepath` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='Super administrator';

--
-- 转存表中的数据 `superadmin`
--

INSERT INTO `superadmin` (`adminid`, `name`, `password`, `nickname`, `isopen`, `begindate`, `enddate`, `regdate`, `mobile`, `email`, `loginnum`, `qrtemp`, `wxuuid`, `basesitepath`) VALUES
(1, 'superroot', '25d55ad283aa400af464c76d713c07ad', '', 0, 0, 0, '', '', '', 0, 0, '', '/Users/hzq/paozhu/www');

-- --------------------------------------------------------

--
-- 表的结构 `sysperms`
--

CREATE TABLE `sysperms` (
  `permsid` int UNSIGNED NOT NULL,
  `parentid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `urlpath` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `urlname` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `name` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `status` tinyint UNSIGNED NOT NULL,
  `permscode` int UNSIGNED NOT NULL,
  `readonly` tinyint UNSIGNED NOT NULL,
  `created_time` int UNSIGNED NOT NULL,
  `updated_time` int UNSIGNED NOT NULL,
  `created_user` int UNSIGNED NOT NULL,
  `updated_user` int UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='权限表';

-- --------------------------------------------------------

--
-- 表的结构 `sysrole`
--

CREATE TABLE `sysrole` (
  `roleid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `name` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `status` tinyint UNSIGNED NOT NULL,
  `rolecode` bigint UNSIGNED NOT NULL,
  `sortid` int NOT NULL,
  `rolevalue` int UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='角色表';

-- --------------------------------------------------------

--
-- 表的结构 `sysroleperms`
--

CREATE TABLE `sysroleperms` (
  `rolepermsid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `roleid` int UNSIGNED NOT NULL,
  `permsid` int UNSIGNED NOT NULL,
  `sortid` int UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- 表的结构 `sysuser`
--

CREATE TABLE `sysuser` (
  `adminid` int UNSIGNED NOT NULL,
  `name` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `password` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `textword` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `isopen` tinyint(1) NOT NULL,
  `level` int NOT NULL,
  `companyid` int UNSIGNED NOT NULL COMMENT '所属企业',
  `dpid` int UNSIGNED NOT NULL,
  `jobid` int UNSIGNED NOT NULL,
  `roleid` int UNSIGNED NOT NULL,
  `postid` int UNSIGNED NOT NULL,
  `created_at` int UNSIGNED NOT NULL,
  `enddate` int UNSIGNED NOT NULL,
  `qrtemp` int UNSIGNED NOT NULL,
  `gender` tinyint UNSIGNED NOT NULL,
  `nickname` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `realname` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `avatar` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `mobile` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `email` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `wxuuid` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- 转存表中的数据 `sysuser`
--

INSERT INTO `sysuser` (`adminid`, `name`, `password`, `textword`, `isopen`, `level`, `companyid`, `dpid`, `jobid`, `roleid`, `postid`, `created_at`, `enddate`, `qrtemp`, `gender`, `nickname`, `realname`, `avatar`, `mobile`, `email`, `wxuuid`) VALUES
(10000, 'admin', '25d55ad283aa400af464c76d713c07ad', '12345678', 1, 191, 1, 0, 0, 0, 0, 0, 0, 0, 0, '', '', '', '', '', '');

-- --------------------------------------------------------

--
-- 表的结构 `testa`
--

CREATE TABLE `testa` (
  `id` int UNSIGNED NOT NULL,
  `parentid` int UNSIGNED NOT NULL COMMENT '父id[id tree]',
  `value_id` tinyint DEFAULT NULL,
  `content` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `deleted` tinyint UNSIGNED NOT NULL,
  `deletetime` int UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

--
-- 转存表中的数据 `testa`
--

INSERT INTO `testa` (`id`, `parentid`, `value_id`, `content`, `deleted`, `deletetime`) VALUES
(1, 0, 65, 'commit test', 0, 1704978371),
(5, 33, 65, 'commit test', 0, 0),
(33, 1, 2, 'Hwkh', 1, 1705034952),
(34, 33, 122, 'commit test', 0, 0);

-- --------------------------------------------------------

--
-- 表的结构 `testb`
--

CREATE TABLE `testb` (
  `tid` int NOT NULL,
  `score` bigint NOT NULL DEFAULT '0' COMMENT '分数[num*100]',
  `name` varchar(30) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- --------------------------------------------------------

--
-- 表的结构 `topic`
--

CREATE TABLE `topic` (
  `topicid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `parentid` int UNSIGNED NOT NULL,
  `cateid` int UNSIGNED NOT NULL COMMENT '类型',
  `sorttype` tinyint UNSIGNED NOT NULL COMMENT '内容排序',
  `languagetype` int UNSIGNED NOT NULL COMMENT '语言类型',
  `isview` tinyint UNSIGNED NOT NULL COMMENT '是否显示',
  `isside` tinyint UNSIGNED NOT NULL DEFAULT '1' COMMENT '是否边栏',
  `sortid` int UNSIGNED NOT NULL COMMENT '排序id',
  `title` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '标题',
  `twotitle` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '第二标题',
  `memo` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '备注',
  `templatename` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '模板名称',
  `url` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '网址',
  `urlpath` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '路径',
  `imgurl` varchar(254) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '图片地址',
  `topimg` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL COMMENT '头图',
  `accesscode` int UNSIGNED NOT NULL COMMENT '权限代码'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='栏目表';

--
-- 转存表中的数据 `topic`
--

INSERT INTO `topic` (`topicid`, `userid`, `parentid`, `cateid`, `sorttype`, `languagetype`, `isview`, `isside`, `sortid`, `title`, `twotitle`, `memo`, `templatename`, `url`, `urlpath`, `imgurl`, `topimg`, `accesscode`) VALUES
(1, 1, 0, 0, 0, 0, 1, 0, 0, ' 首页', '', '', '', '', 'home', '', '', 0);

-- --------------------------------------------------------

--
-- 表的结构 `xmeet`
--

CREATE TABLE `xmeet` (
  `xmeetid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `xpjid` int UNSIGNED NOT NULL,
  `xtaskid` int UNSIGNED NOT NULL,
  `adminid` int UNSIGNED NOT NULL,
  `title` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `zhuchi` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `jilu` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `meetnotice` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `meetfiles` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `address` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `meettime` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `regdate` int UNSIGNED NOT NULL,
  `expecttime` int UNSIGNED NOT NULL,
  `endtime` int UNSIGNED NOT NULL,
  `presents` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `content` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `postresult` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `postfiles` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `jiluphoto` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='会议纪要';

-- --------------------------------------------------------

--
-- 表的结构 `xproject`
--

CREATE TABLE `xproject` (
  `xpjid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `prexpjid` int UNSIGNED NOT NULL,
  `dpid` int UNSIGNED NOT NULL,
  `grouptype` int UNSIGNED NOT NULL,
  `title` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `adminuserid` int UNSIGNED NOT NULL,
  `regdate` int UNSIGNED NOT NULL,
  `begindate` int UNSIGNED NOT NULL,
  `expiredate` int UNSIGNED NOT NULL,
  `isopen` tinyint NOT NULL,
  `clientid` int UNSIGNED NOT NULL,
  `totalnum` int UNSIGNED NOT NULL,
  `referdocverion` int UNSIGNED NOT NULL,
  `xtheme` int UNSIGNED NOT NULL,
  `xlogo` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `introduce` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `giturl` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `gitname` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `gitpwd` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `xcolor` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `fupan` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `totalvalue` double NOT NULL,
  `expectday` float NOT NULL,
  `realday` float NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='项目表';

-- --------------------------------------------------------

--
-- 表的结构 `xtalk`
--

CREATE TABLE `xtalk` (
  `talkid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `adminid` int UNSIGNED NOT NULL,
  `taskid` int UNSIGNED NOT NULL,
  `projectid` int UNSIGNED NOT NULL,
  `content` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `isdelete` tinyint UNSIGNED NOT NULL,
  `addtime` int UNSIGNED NOT NULL,
  `update_at` int UNSIGNED NOT NULL,
  `islock` tinyint UNSIGNED NOT NULL,
  `replyid` int UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='任务讨论';

-- --------------------------------------------------------

--
-- 表的结构 `xtask`
--

CREATE TABLE `xtask` (
  `xtaskid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `xpjid` int UNSIGNED NOT NULL,
  `adminid` int UNSIGNED NOT NULL,
  `parentid` int UNSIGNED NOT NULL,
  `begindate` int UNSIGNED NOT NULL,
  `enddate` int UNSIGNED NOT NULL,
  `expectbegindate` int UNSIGNED NOT NULL,
  `expectenddate` int UNSIGNED NOT NULL,
  `milestone` int UNSIGNED NOT NULL,
  `subxpjid` int UNSIGNED NOT NULL,
  `depxtaskid` int UNSIGNED NOT NULL,
  `referdocid` int UNSIGNED NOT NULL,
  `isfinish` tinyint UNSIGNED NOT NULL,
  `updatedate` int UNSIGNED NOT NULL,
  `finishdate` int UNSIGNED NOT NULL,
  `iscore` int UNSIGNED NOT NULL,
  `xvalue` double NOT NULL,
  `expectday` float NOT NULL,
  `realday` float NOT NULL,
  `pricevalue` float NOT NULL,
  `title` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `introduce` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `xlogo` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `xcolor` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `pullurl` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `pulltitle` varchar(256) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `pullauthor` varchar(60) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `note` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `itemnote` text CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='项目任务表';

--
-- 转储表的索引
--

--
-- 表的索引 `article`
--
ALTER TABLE `article`
  ADD PRIMARY KEY (`aid`),
  ADD KEY `userid` (`userid`);

--
-- 表的索引 `blogcatalog`
--
ALTER TABLE `blogcatalog`
  ADD PRIMARY KEY (`bid`);

--
-- 表的索引 `brand`
--
ALTER TABLE `brand`
  ADD PRIMARY KEY (`brandid`);

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
-- 表的索引 `homeblock`
--
ALTER TABLE `homeblock`
  ADD PRIMARY KEY (`hbid`);

--
-- 表的索引 `logininfo`
--
ALTER TABLE `logininfo`
  ADD PRIMARY KEY (`lgid`);

--
-- 表的索引 `product`
--
ALTER TABLE `product`
  ADD PRIMARY KEY (`pid`),
  ADD KEY `userid` (`userid`),
  ADD KEY `sortid` (`sortid`);

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
-- 表的索引 `superadmin`
--
ALTER TABLE `superadmin`
  ADD PRIMARY KEY (`adminid`);

--
-- 表的索引 `sysperms`
--
ALTER TABLE `sysperms`
  ADD PRIMARY KEY (`permsid`);

--
-- 表的索引 `sysrole`
--
ALTER TABLE `sysrole`
  ADD PRIMARY KEY (`roleid`);

--
-- 表的索引 `sysroleperms`
--
ALTER TABLE `sysroleperms`
  ADD PRIMARY KEY (`rolepermsid`);

--
-- 表的索引 `sysuser`
--
ALTER TABLE `sysuser`
  ADD PRIMARY KEY (`adminid`);

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
-- 表的索引 `xmeet`
--
ALTER TABLE `xmeet`
  ADD PRIMARY KEY (`xmeetid`),
  ADD KEY `userid` (`userid`);

--
-- 表的索引 `xproject`
--
ALTER TABLE `xproject`
  ADD PRIMARY KEY (`xpjid`),
  ADD KEY `userid` (`userid`);

--
-- 表的索引 `xtalk`
--
ALTER TABLE `xtalk`
  ADD PRIMARY KEY (`talkid`),
  ADD KEY `userid` (`userid`),
  ADD KEY `projectid` (`projectid`);

--
-- 表的索引 `xtask`
--
ALTER TABLE `xtask`
  ADD PRIMARY KEY (`xtaskid`);

--
-- 在导出的表使用AUTO_INCREMENT
--

--
-- 使用表AUTO_INCREMENT `article`
--
ALTER TABLE `article`
  MODIFY `aid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=9;

--
-- 使用表AUTO_INCREMENT `blogcatalog`
--
ALTER TABLE `blogcatalog`
  MODIFY `bid` int UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- 使用表AUTO_INCREMENT `brand`
--
ALTER TABLE `brand`
  MODIFY `brandid` int UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- 使用表AUTO_INCREMENT `catalogue`
--
ALTER TABLE `catalogue`
  MODIFY `cid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=165;

--
-- 使用表AUTO_INCREMENT `department`
--
ALTER TABLE `department`
  MODIFY `dpid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=55;

--
-- 使用表AUTO_INCREMENT `homeblock`
--
ALTER TABLE `homeblock`
  MODIFY `hbid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=16;

--
-- 使用表AUTO_INCREMENT `logininfo`
--
ALTER TABLE `logininfo`
  MODIFY `lgid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;

--
-- 使用表AUTO_INCREMENT `product`
--
ALTER TABLE `product`
  MODIFY `pid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=358;

--
-- 使用表AUTO_INCREMENT `productparam`
--
ALTER TABLE `productparam`
  MODIFY `ppid` int UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- 使用表AUTO_INCREMENT `siteinfo`
--
ALTER TABLE `siteinfo`
  MODIFY `sid` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=7;

--
-- 使用表AUTO_INCREMENT `superadmin`
--
ALTER TABLE `superadmin`
  MODIFY `adminid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;

--
-- 使用表AUTO_INCREMENT `sysperms`
--
ALTER TABLE `sysperms`
  MODIFY `permsid` int UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- 使用表AUTO_INCREMENT `sysrole`
--
ALTER TABLE `sysrole`
  MODIFY `roleid` int UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- 使用表AUTO_INCREMENT `sysroleperms`
--
ALTER TABLE `sysroleperms`
  MODIFY `rolepermsid` int UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- 使用表AUTO_INCREMENT `sysuser`
--
ALTER TABLE `sysuser`
  MODIFY `adminid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=10003;

--
-- 使用表AUTO_INCREMENT `testa`
--
ALTER TABLE `testa`
  MODIFY `id` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=35;

--
-- 使用表AUTO_INCREMENT `testb`
--
ALTER TABLE `testb`
  MODIFY `tid` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=120;

--
-- 使用表AUTO_INCREMENT `xmeet`
--
ALTER TABLE `xmeet`
  MODIFY `xmeetid` int UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- 使用表AUTO_INCREMENT `xproject`
--
ALTER TABLE `xproject`
  MODIFY `xpjid` int UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- 使用表AUTO_INCREMENT `xtalk`
--
ALTER TABLE `xtalk`
  MODIFY `talkid` int UNSIGNED NOT NULL AUTO_INCREMENT;

--
-- 使用表AUTO_INCREMENT `xtask`
--
ALTER TABLE `xtask`
  MODIFY `xtaskid` int UNSIGNED NOT NULL AUTO_INCREMENT;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
