-- Adminer 4.8.0 MySQL 8.0.22-0ubuntu0.20.10.2 dump

SET NAMES utf8;
SET time_zone = '+00:00';
SET foreign_key_checks = 0;
SET sql_mode = 'NO_AUTO_VALUE_ON_ZERO';

SET NAMES utf8mb4;

DROP TABLE IF EXISTS `gw_files`;
CREATE TABLE `gw_files` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `tg_file_id` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `tg_uniq_id` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL,
  `md5_sum` binary(16) NOT NULL,
  `sha1_sum` binary(20) NOT NULL,
  `ext` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `file_type` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `hit_count` int unsigned NOT NULL DEFAULT '1',
  `description` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_520_ci,
  `created_at` datetime NOT NULL,
  `updated_at` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `tg_uniq_id` (`tg_uniq_id`),
  KEY `tg_file_id` (`tg_file_id`),
  KEY `md5_sum` (`md5_sum`),
  KEY `sha1_sum` (`sha1_sum`),
  KEY `ext` (`ext`),
  KEY `file_type` (`file_type`),
  KEY `hit_count` (`hit_count`),
  KEY `created_at` (`created_at`),
  KEY `updated_at` (`updated_at`),
  FULLTEXT KEY `description` (`description`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_520_ci;


DROP TABLE IF EXISTS `gw_group_history`;
CREATE TABLE `gw_group_history` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `gw_group_id` bigint unsigned NOT NULL,
  `username` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_520_ci NOT NULL,
  `link` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `photo` bigint unsigned DEFAULT NULL,
  `created_at` datetime NOT NULL,
  PRIMARY KEY (`id`),
  KEY `gw_group_id` (`gw_group_id`),
  KEY `photo` (`photo`),
  KEY `username` (`username`),
  KEY `name` (`name`),
  KEY `link` (`link`),
  KEY `created_at` (`created_at`),
  CONSTRAINT `gw_group_history_ibfk_3` FOREIGN KEY (`gw_group_id`) REFERENCES `gw_groups` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `gw_group_history_ibfk_4` FOREIGN KEY (`photo`) REFERENCES `gw_files` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_520_ci;


DROP TABLE IF EXISTS `gw_groups`;
CREATE TABLE `gw_groups` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `tg_group_id` bigint NOT NULL,
  `username` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_520_ci NOT NULL,
  `link` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `photo` bigint unsigned DEFAULT NULL,
  `msg_count` bigint unsigned NOT NULL DEFAULT '0',
  `created_at` datetime NOT NULL,
  `updated_at` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `tg_group_id` (`tg_group_id`),
  KEY `photo` (`photo`),
  KEY `username` (`username`),
  KEY `name` (`name`),
  KEY `link` (`link`),
  KEY `msg_count` (`msg_count`),
  KEY `created_at` (`created_at`),
  KEY `updated_at` (`updated_at`),
  CONSTRAINT `gw_groups_ibfk_2` FOREIGN KEY (`photo`) REFERENCES `gw_files` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_520_ci;


DROP TABLE IF EXISTS `gw_user_history`;
CREATE TABLE `gw_user_history` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `gw_user_id` bigint unsigned NOT NULL,
  `username` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `first_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_520_ci NOT NULL,
  `last_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_520_ci DEFAULT NULL,
  `photo` bigint unsigned DEFAULT NULL,
  `created_at` datetime NOT NULL,
  PRIMARY KEY (`id`),
  KEY `gw_user_id` (`gw_user_id`),
  KEY `photo` (`photo`),
  KEY `username` (`username`),
  KEY `first_name` (`first_name`),
  KEY `last_name` (`last_name`),
  KEY `created_at` (`created_at`),
  CONSTRAINT `gw_user_history_ibfk_3` FOREIGN KEY (`gw_user_id`) REFERENCES `gw_users` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `gw_user_history_ibfk_4` FOREIGN KEY (`photo`) REFERENCES `gw_files` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_520_ci;


DROP TABLE IF EXISTS `gw_users`;
CREATE TABLE `gw_users` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `tg_user_id` bigint unsigned NOT NULL,
  `username` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `first_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_520_ci NOT NULL,
  `last_name` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_520_ci DEFAULT NULL,
  `photo` bigint unsigned DEFAULT NULL,
  `group_msg_count` int unsigned NOT NULL DEFAULT '0',
  `private_msg_count` int unsigned NOT NULL DEFAULT '0',
  `is_bot` enum('0','1') CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci NOT NULL DEFAULT '0',
  `created_at` datetime NOT NULL,
  `updated_at` datetime DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `tg_user_id` (`tg_user_id`),
  KEY `username` (`username`),
  KEY `first_name` (`first_name`),
  KEY `last_name` (`last_name`),
  KEY `photo` (`photo`),
  KEY `group_msg_count` (`group_msg_count`),
  KEY `private_msg_count` (`private_msg_count`),
  KEY `is_bot` (`is_bot`),
  KEY `created_at` (`created_at`),
  KEY `updated_at` (`updated_at`),
  CONSTRAINT `gw_users_ibfk_1` FOREIGN KEY (`photo`) REFERENCES `gw_files` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_520_ci;


-- 2021-04-03 11:37:18
