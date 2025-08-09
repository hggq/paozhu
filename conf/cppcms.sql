-- phpMyAdmin SQL Dump
-- version 5.2.2
-- https://www.phpmyadmin.net/
--
-- 主机： localhost
-- 生成日期： 2025-08-09 05:03:55
-- 服务器版本： 8.4.4
-- PHP 版本： 8.4.8

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
-- 表的结构 `topicset`
--

CREATE TABLE `topicset` (
  `topicsetid` int UNSIGNED NOT NULL,
  `userid` int UNSIGNED NOT NULL,
  `topicid` int UNSIGNED NOT NULL,
  `linktopicid` int UNSIGNED NOT NULL,
  `linkrownum` tinyint UNSIGNED NOT NULL,
  `sidetype` tinyint UNSIGNED NOT NULL COMMENT 'showtype',
  `sidename` varchar(80) COLLATE utf8mb4_general_ci NOT NULL,
  `txtcontent` text COLLATE utf8mb4_general_ci NOT NULL,
  `sort` int UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci COMMENT='边栏设置内容 Sidebar settings content';

--
-- 转存表中的数据 `topicset`
--

INSERT INTO `topicset` (`topicsetid`, `userid`, `topicid`, `linktopicid`, `linkrownum`, `sidetype`, `sidename`, `txtcontent`, `sort`) VALUES
(2, 1, 1, 0, 0, 0, 'bbbb', '<p>666666</p>', 2),
(3, 1, 1, 1, 6, 1, 'aaaa', '', 3),
(5, 1, 1, 1, 8, 1, 'cccc', '', 0);

--
-- 转储表的索引
--

--
-- 表的索引 `topicset`
--
ALTER TABLE `topicset`
  ADD PRIMARY KEY (`topicsetid`);

--
-- 在导出的表使用AUTO_INCREMENT
--

--
-- 使用表AUTO_INCREMENT `topicset`
--
ALTER TABLE `topicset`
  MODIFY `topicsetid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=6;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
