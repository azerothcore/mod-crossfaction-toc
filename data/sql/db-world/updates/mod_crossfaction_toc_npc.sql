SET @ENTRY := 210000;
SET @TEXT  := 210000;

DELETE FROM `creature_template` WHERE `entry` = @ENTRY;
INSERT INTO `creature_template` (`entry`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `rank`, `dmgschool`, `BaseAttackTime`, `RangeAttackTime`, `unit_class`, `unit_flags`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `AIName`, `MovementType`, `HoverHeight`, `RacialLeader`, `movementId`, `RegenHealth`, `CreatureImmunitiesId`, `flags_extra`, `ScriptName`) VALUES
(@ENTRY, 'Argent Exchange Officer', 'Crusader Gear Exchange', NULL, 0, 80, 80, 2, 35, 129, 0, 0, 2000, 0, 1, 0, 7, 0, 0, 0, 0, '', 0, 1, 0, 0, 1, 0, 2, 'npc_crossfaction_toc_exchange');

-- Blood elf Argent Peacekeeper model from the Argent Tournament Grounds
DELETE FROM `creature_template_model` WHERE `CreatureID` = @ENTRY;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(@ENTRY, 0, 28929, 1, 1, 0);

-- Trial of the Crusader entrance hall (Argent Tournament Grounds), in the gap of the east row of champions.
-- No fixed guid so the spawn can't collide with another server's custom spawns.
DELETE FROM `creature` WHERE `id` = @ENTRY;
INSERT INTO `creature` (`id`, `map`, `spawnMask`, `phaseMask`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`) VALUES
(@ENTRY, 571, 1, 1, 8528.9, 716.3, 558.61, 3.14, 300);

DELETE FROM `creature_template_locale` WHERE `entry` = @ENTRY;
INSERT INTO `creature_template_locale` (`entry`, `locale`, `Name`, `Title`, `VerifiedBuild`) VALUES
(@ENTRY, 'koKR', '은빛십자군 교환 담당관', '십자군 장비 교환', 0),
(@ENTRY, 'frFR', 'Officier d''échange de la Croisade d''argent', 'Échange d''équipement de croisé', 0),
(@ENTRY, 'deDE', 'Tauschoffizier des Argentumkreuzzugs', 'Tausch von Kreuzfahrerausrüstung', 0),
(@ENTRY, 'zhCN', '银色北伐军兑换官', '十字军装备兑换', 0),
(@ENTRY, 'zhTW', '銀白十字軍兌換官', '十字軍裝備兌換', 0),
(@ENTRY, 'esES', 'Oficial de intercambio de la Cruzada Argenta', 'Intercambio de equipo de cruzado', 0),
(@ENTRY, 'esMX', 'Oficial de intercambio de la Cruzada Argenta', 'Intercambio de equipo de cruzado', 0),
(@ENTRY, 'ruRU', 'Офицер обмена Серебряного Авангарда', 'Обмен снаряжения крестоносцев', 0);

-- Gossip body text. The client requests it by npc_text ID, so it cannot come from module_string.
DELETE FROM `npc_text` WHERE `ID` = @TEXT;
INSERT INTO `npc_text` (`ID`, `text0_0`, `text0_1`, `BroadcastTextID0`, `lang0`, `Probability0`, `VerifiedBuild`) VALUES
(@TEXT,
'Bal''a dash, malanore. Beneath the Crusade''s banner, Horde and Alliance fight as one... yet the Coliseum''s armorers still forge every reward for one side or the other.$B$BIf the Trial granted you gear meant for the other faction, bring it to me while it is still fresh from the Coliseum, and I shall trade it for its counterpart.$B$BBe warned: the exchange is final. The new item can no longer be traded to those who shared in the loot.',
'Bal''a dash, malanore. Beneath the Crusade''s banner, Horde and Alliance fight as one... yet the Coliseum''s armorers still forge every reward for one side or the other.$B$BIf the Trial granted you gear meant for the other faction, bring it to me while it is still fresh from the Coliseum, and I shall trade it for its counterpart.$B$BBe warned: the exchange is final. The new item can no longer be traded to those who shared in the loot.',
0, 0, 1, 0);

DELETE FROM `npc_text_locale` WHERE `ID` = @TEXT;
INSERT INTO `npc_text_locale` (`ID`, `Locale`, `Text0_0`, `Text0_1`) VALUES
(@TEXT, 'koKR',
'발라 다쉬, 말라노레. 십자군의 깃발 아래 호드와 얼라이언스는 하나가 되어 싸우지만... 원형경기장의 무구 제작자들은 여전히 모든 보상을 한쪽 진영만을 위해 만들고 있지요.$B$B시험에서 다른 진영을 위한 장비를 얻었다면, 원형경기장에서 갓 가져온 것일 때 제게 가져오십시오. 그에 맞는 장비로 바꿔 드리겠습니다.$B$B명심하십시오. 교환은 되돌릴 수 없습니다. 새 아이템은 전리품을 함께 나눈 이들에게 더 이상 거래할 수 없습니다.',
'발라 다쉬, 말라노레. 십자군의 깃발 아래 호드와 얼라이언스는 하나가 되어 싸우지만... 원형경기장의 무구 제작자들은 여전히 모든 보상을 한쪽 진영만을 위해 만들고 있지요.$B$B시험에서 다른 진영을 위한 장비를 얻었다면, 원형경기장에서 갓 가져온 것일 때 제게 가져오십시오. 그에 맞는 장비로 바꿔 드리겠습니다.$B$B명심하십시오. 교환은 되돌릴 수 없습니다. 새 아이템은 전리품을 함께 나눈 이들에게 더 이상 거래할 수 없습니다.'),
(@TEXT, 'frFR',
'Bal''a dash, malanore. Sous la bannière de la Croisade, la Horde et l''Alliance combattent comme un seul homme... et pourtant les armuriers du Colisée forgent chaque récompense pour l''un ou l''autre camp.$B$BSi l''épreuve vous a octroyé de l''équipement destiné à l''autre faction, apportez-le-moi tant qu''il est encore frais du Colisée, et je l''échangerai contre son équivalent.$B$BPrenez garde : l''échange est définitif. Le nouvel objet ne pourra plus être échangé avec ceux qui ont partagé le butin.',
'Bal''a dash, malanore. Sous la bannière de la Croisade, la Horde et l''Alliance combattent comme un seul homme... et pourtant les armuriers du Colisée forgent chaque récompense pour l''un ou l''autre camp.$B$BSi l''épreuve vous a octroyé de l''équipement destiné à l''autre faction, apportez-le-moi tant qu''il est encore frais du Colisée, et je l''échangerai contre son équivalent.$B$BPrenez garde : l''échange est définitif. Le nouvel objet ne pourra plus être échangé avec ceux qui ont partagé le butin.'),
(@TEXT, 'deDE',
'Bal''a dash, malanore. Unter dem Banner des Kreuzzugs kämpfen Horde und Allianz Seite an Seite... und doch schmieden die Rüstmeister des Kolosseums jede Belohnung für die eine oder die andere Seite.$B$BHat Euch die Prüfung Ausrüstung beschert, die für die andere Fraktion bestimmt ist, so bringt sie mir, solange sie noch frisch aus dem Kolosseum stammt, und ich tausche sie gegen ihr Gegenstück.$B$BSeid gewarnt: Der Tausch ist endgültig. Der neue Gegenstand kann nicht mehr mit jenen gehandelt werden, die sich die Beute geteilt haben.',
'Bal''a dash, malanore. Unter dem Banner des Kreuzzugs kämpfen Horde und Allianz Seite an Seite... und doch schmieden die Rüstmeister des Kolosseums jede Belohnung für die eine oder die andere Seite.$B$BHat Euch die Prüfung Ausrüstung beschert, die für die andere Fraktion bestimmt ist, so bringt sie mir, solange sie noch frisch aus dem Kolosseum stammt, und ich tausche sie gegen ihr Gegenstück.$B$BSeid gewarnt: Der Tausch ist endgültig. Der neue Gegenstand kann nicht mehr mit jenen gehandelt werden, die sich die Beute geteilt haben.'),
(@TEXT, 'zhCN',
'Bal''a dash, malanore。在北伐军的旗帜下，部落与联盟并肩作战……然而竞技场的军械师们依然只为其中一方打造每一件奖励。$B$B如果试炼给了你属于另一阵营的装备，趁它刚从竞技场带出来时交给我，我会把它换成对应的装备。$B$B注意：兑换无法撤销。新物品将无法再交易给与你一同分享战利品的人。',
'Bal''a dash, malanore。在北伐军的旗帜下，部落与联盟并肩作战……然而竞技场的军械师们依然只为其中一方打造每一件奖励。$B$B如果试炼给了你属于另一阵营的装备，趁它刚从竞技场带出来时交给我，我会把它换成对应的装备。$B$B注意：兑换无法撤销。新物品将无法再交易给与你一同分享战利品的人。'),
(@TEXT, 'zhTW',
'Bal''a dash, malanore。在十字軍的旗幟下，部落與聯盟並肩作戰……然而競技場的軍械師們依然只為其中一方打造每一件獎勵。$B$B如果試煉給了你屬於另一陣營的裝備，趁它剛從競技場帶出來時交給我，我會把它換成對應的裝備。$B$B注意：兌換無法撤銷。新物品將無法再交易給與你一同分享戰利品的人。',
'Bal''a dash, malanore。在十字軍的旗幟下，部落與聯盟並肩作戰……然而競技場的軍械師們依然只為其中一方打造每一件獎勵。$B$B如果試煉給了你屬於另一陣營的裝備，趁它剛從競技場帶出來時交給我，我會把它換成對應的裝備。$B$B注意：兌換無法撤銷。新物品將無法再交易給與你一同分享戰利品的人。'),
(@TEXT, 'esES',
'Bal''a dash, malanore. Bajo el estandarte de la Cruzada, la Horda y la Alianza luchan codo con codo... y aun así los armeros del Coliseo forjan cada recompensa para un bando u otro.$B$BSi la prueba te ha otorgado equipo destinado a la otra facción, tráemelo mientras siga recién salido del Coliseo y lo cambiaré por su equivalente.$B$BTen en cuenta que el intercambio es definitivo: el nuevo objeto ya no podrá comerciarse con quienes compartieron el botín.',
'Bal''a dash, malanore. Bajo el estandarte de la Cruzada, la Horda y la Alianza luchan codo con codo... y aun así los armeros del Coliseo forjan cada recompensa para un bando u otro.$B$BSi la prueba te ha otorgado equipo destinado a la otra facción, tráemelo mientras siga recién salido del Coliseo y lo cambiaré por su equivalente.$B$BTen en cuenta que el intercambio es definitivo: el nuevo objeto ya no podrá comerciarse con quienes compartieron el botín.'),
(@TEXT, 'esMX',
'Bal''a dash, malanore. Bajo el estandarte de la Cruzada, la Horda y la Alianza luchan codo con codo... y aun así los armeros del Coliseo forjan cada recompensa para un bando u otro.$B$BSi la prueba te otorgó equipo destinado a la otra facción, tráemelo mientras siga recién salido del Coliseo y lo cambiaré por su equivalente.$B$BTen en cuenta que el intercambio es definitivo: el nuevo objeto ya no podrá comerciarse con quienes compartieron el botín.',
'Bal''a dash, malanore. Bajo el estandarte de la Cruzada, la Horda y la Alianza luchan codo con codo... y aun así los armeros del Coliseo forjan cada recompensa para un bando u otro.$B$BSi la prueba te otorgó equipo destinado a la otra facción, tráemelo mientras siga recién salido del Coliseo y lo cambiaré por su equivalente.$B$BTen en cuenta que el intercambio es definitivo: el nuevo objeto ya no podrá comerciarse con quienes compartieron el botín.'),
(@TEXT, 'ruRU',
'Бал''а даш, маланор. Под знаменем Авангарда Орда и Альянс сражаются плечом к плечу... и всё же оружейники Колизея куют каждую награду лишь для одной из сторон.$B$BЕсли испытание наградило вас снаряжением, предназначенным для другой фракции, принесите его мне, пока оно ещё свежо после Колизея, и я обменяю его на равноценное.$B$BУчтите: обмен окончателен. Новый предмет больше нельзя будет передать тем, кто делил с вами добычу.',
'Бал''а даш, маланор. Под знаменем Авангарда Орда и Альянс сражаются плечом к плечу... и всё же оружейники Колизея куют каждую награду лишь для одной из сторон.$B$BЕсли испытание наградило вас снаряжением, предназначенным для другой фракции, принесите его мне, пока оно ещё свежо после Колизея, и я обменяю его на равноценное.$B$BУчтите: обмен окончателен. Новый предмет больше нельзя будет передать тем, кто делил с вами добычу.');
