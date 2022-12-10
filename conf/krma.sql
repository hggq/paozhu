-- phpMyAdmin SQL Dump
-- version 5.1.0
-- https://www.phpmyadmin.net/
--
-- 主机： localhost
-- 生成日期： 2022-05-17 20:37:45
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
-- 数据库： `krma`
--

-- --------------------------------------------------------

--
-- 表的结构 `web_news`
--

CREATE TABLE `web_news` (
  `newsid` int UNSIGNED NOT NULL,
  `classID` int UNSIGNED NOT NULL DEFAULT '0',
  `topicID` int UNSIGNED NOT NULL DEFAULT '0',
  `channelID` int UNSIGNED NOT NULL DEFAULT '0',
  `clicknum` int NOT NULL,
  `newtitle` varchar(120) DEFAULT NULL,
  `titletwo` varchar(120) NOT NULL,
  `seotitle` varchar(255) NOT NULL,
  `keywork` varchar(255) NOT NULL,
  `depiction` varchar(255) NOT NULL,
  `newsColor` varchar(10) DEFAULT NULL,
  `source` varchar(20) DEFAULT NULL,
  `author` varchar(20) DEFAULT NULL,
  `newsDate` date DEFAULT NULL,
  `isView` enum('Y','N') NOT NULL DEFAULT 'Y',
  `isHome` enum('Y','N') NOT NULL DEFAULT 'Y',
  `isNew` enum('Y','N') NOT NULL DEFAULT 'N',
  `isUp` enum('Y','N') NOT NULL DEFAULT 'N',
  `isComment` enum('Y','N') NOT NULL DEFAULT 'N',
  `sort` int NOT NULL DEFAULT '0',
  `userID` int NOT NULL DEFAULT '0',
  `language` tinyint NOT NULL DEFAULT '0',
  `addtime` varchar(12) DEFAULT NULL,
  `adddate` date NOT NULL,
  `newsIco` varchar(128) DEFAULT NULL,
  `newsImg` varchar(33) DEFAULT NULL,
  `dest` varchar(255) NOT NULL,
  `newsContent` text
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb3;

--
-- 转存表中的数据 `web_news`
--

INSERT INTO `web_news` (`newsid`, `classID`, `topicID`, `channelID`, `clicknum`, `newtitle`, `titletwo`, `seotitle`, `keywork`, `depiction`, `newsColor`, `source`, `author`, `newsDate`, `isView`, `isHome`, `isNew`, `isUp`, `isComment`, `sort`, `userID`, `language`, `addtime`, `adddate`, `newsIco`, `newsImg`, `dest`, `newsContent`) VALUES
(63599, 0, 0, 0, 7, 'asdafafaf', 'asdfa', 'asdf', '', '', NULL, NULL, NULL, '2022-05-17', 'Y', 'Y', 'N', 'N', 'N', 0, 0, 0, 'adfafaf', '2022-05-19', NULL, NULL, '', NULL),
(63697, 0, 0, 0, 0, NULL, '', '', '', '', NULL, NULL, NULL, NULL, 'Y', 'Y', 'N', 'N', 'N', 0, 0, 0, NULL, '0000-00-00', NULL, NULL, 'afda', NULL);

--
-- 转储表的索引
--

--
-- 表的索引 `web_news`
--
ALTER TABLE `web_news`
  ADD PRIMARY KEY (`newsid`),
  ADD KEY `userID` (`userID`),
  ADD KEY `language` (`language`),
  ADD KEY `topicID` (`topicID`),
  ADD KEY `channelID` (`channelID`);

--
-- 在导出的表使用AUTO_INCREMENT
--

--
-- 使用表AUTO_INCREMENT `web_news`
--
ALTER TABLE `web_news`
  MODIFY `newsid` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=63698;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
