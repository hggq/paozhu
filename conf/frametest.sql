-- phpMyAdmin SQL Dump
-- version 5.1.0
-- https://www.phpmyadmin.net/
--
-- 主机： localhost
-- 生成日期： 2022-05-17 20:37:13
-- 服务器版本： 8.0.29
-- PHP 版本： 7.4.3

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- 数据库： `aaa`
--

-- --------------------------------------------------------

--
-- 表的结构 `marlist`
--

CREATE TABLE `marlist` (
  `mid` int NOT NULL,
  `name` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '名字',
  `phone` bigint DEFAULT '0' COMMENT '手机号',
  `email` varchar(40) DEFAULT NULL COMMENT '邮件',
  `area` varchar(40) DEFAULT NULL COMMENT '区域',
  `stateus` int DEFAULT '1' COMMENT '状态'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='主持人表';

-- --------------------------------------------------------

--
-- 表的结构 `moduleauth`
--

CREATE TABLE `moduleauth` (
  `mid` int NOT NULL,
  `name` varchar(20) NOT NULL COMMENT '名称',
  `siteurl` varchar(200) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT 'url',
  `sort` bigint UNSIGNED NOT NULL DEFAULT '0' COMMENT '排序',
  `addtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '添加时间',
  `dsssggg` date DEFAULT NULL,
  `testdatetime` datetime DEFAULT CURRENT_TIMESTAMP,
  `testenum` enum('已婚','未婚','离异','不婚') NOT NULL,
  `score` float NOT NULL DEFAULT '0',
  `price` decimal(7,2) NOT NULL DEFAULT '0.00'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='模块列表';

-- --------------------------------------------------------

--
-- 表的结构 `user`
--

CREATE TABLE `user` (
  `uid` int UNSIGNED NOT NULL,
  `name` varchar(120) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '显示名字',
  `loginname` char(35) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci NOT NULL COMMENT '登录名',
  `loginpwd` varchar(40) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci DEFAULT NULL COMMENT '登录密码',
  `phone` varchar(30) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci DEFAULT NULL COMMENT '电话',
  `regtime` bigint DEFAULT NULL COMMENT '时间戳',
  `level` tinyint NOT NULL DEFAULT '0' COMMENT '权限等级',
  `prices` decimal(7,2) NOT NULL DEFAULT '0.00'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='登录表';

--
-- 转储表的索引
--

--
-- 表的索引 `marlist`
--
ALTER TABLE `marlist`
  ADD PRIMARY KEY (`mid`);

--
-- 表的索引 `moduleauth`
--
ALTER TABLE `moduleauth`
  ADD PRIMARY KEY (`mid`);

--
-- 表的索引 `user`
--
ALTER TABLE `user`
  ADD PRIMARY KEY (`uid`);

--
-- 在导出的表使用AUTO_INCREMENT
--

--
-- 使用表AUTO_INCREMENT `marlist`
--
ALTER TABLE `marlist`
  MODIFY `mid` int NOT NULL AUTO_INCREMENT;

--
-- 使用表AUTO_INCREMENT `moduleauth`
--
ALTER TABLE `moduleauth`
  MODIFY `mid` int NOT NULL AUTO_INCREMENT;

--
-- 使用表AUTO_INCREMENT `user`
--
ALTER TABLE `user`
  MODIFY `uid` int UNSIGNED NOT NULL AUTO_INCREMENT;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
