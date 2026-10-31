SET @ENTRY := 210000;

DELETE FROM `creature_template` WHERE `entry` = @ENTRY;
INSERT INTO `creature_template` (`entry`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `unit_class`, `unit_flags`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `AIName`, `MovementType`, `HoverHeight`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`) VALUES
(@ENTRY, 'Argent Exchange Officer', 'Crusader Gear Exchange', NULL, 0, 80, 80, 2, 35, 1, 0, 0, 2000, 0, 1, 0, 7, 0, 0, 0, 0, '', 0, 1, 0, 0, 1, 0, 2, 'npc_crossfaction_toc_exchange');

DELETE FROM `creature_template_model` WHERE `CreatureID` = @ENTRY;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(@ENTRY, 0, 27020, 1, 1, 0);

-- Trial of the Crusader entrance hall (Argent Tournament Grounds), in the gap of the east row of champions.
-- No fixed guid so the spawn can't collide with another server's custom spawns.
DELETE FROM `creature` WHERE `id` = @ENTRY;
INSERT INTO `creature` (`id`, `map`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`) VALUES
(@ENTRY, 571, 1, 1, 8528.9, 716.3, 558.61, 3.14, 300);
