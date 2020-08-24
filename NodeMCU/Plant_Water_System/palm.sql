-- phpMyAdmin SQL Dump
-- version 4.9.2
-- https://www.phpmyadmin.net/
--
-- Host: localhost
-- Generation Time: Aug 24, 2020 at 06:59 PM
-- Server version: 10.3.22-MariaDB-log
-- PHP Version: 7.2.31

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `148523`
--

-- --------------------------------------------------------

--
-- Table structure for table `palm`
--

CREATE TABLE `palm` (
  `trend_id` int(11) NOT NULL,
  `temp_level` float NOT NULL,
  `soil_mois_level` float NOT NULL,
  `water_amount` float NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `palm`
--

INSERT INTO `palm` (`trend_id`, `temp_level`, `soil_mois_level`, `water_amount`) VALUES
(1, 10, 20, 167.67),
(2, 10, 40, 121.08),
(3, 10, 60, 62.03),
(4, 10, 80, 13),
(5, 10, 100, 13),
(6, 20, 20, 183.55),
(7, 20, 40, 142.22),
(8, 20, 60, 72.9),
(9, 20, 80, 29.15),
(10, 20, 100, 29.15),
(11, 30, 20, 208.47),
(12, 30, 40, 161.4),
(13, 30, 60, 92.3),
(14, 30, 80, 47.17),
(15, 30, 100, 47.17),
(16, 40, 20, 228.52),
(17, 40, 40, 178.27),
(18, 40, 60, 107.76),
(19, 40, 80, 60),
(20, 40, 100, 60),
(21, 50, 20, 228.52),
(22, 50, 40, 178.27),
(23, 50, 60, 107.76),
(24, 50, 80, 60),
(25, 50, 100, 60);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `palm`
--
ALTER TABLE `palm`
  ADD PRIMARY KEY (`trend_id`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `palm`
--
ALTER TABLE `palm`
  MODIFY `trend_id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=26;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
