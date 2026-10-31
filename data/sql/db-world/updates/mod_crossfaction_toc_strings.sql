DELETE FROM `module_string` WHERE `module` = 'mod-crossfaction-toc';
INSERT INTO `module_string` (`module`, `id`, `string`) VALUES
('mod-crossfaction-toc', 1, 'Show me the gear I can exchange.'),
('mod-crossfaction-toc', 2, 'You carry no items of the other faction that can be exchanged. Only items still within their loot trade window qualify.'),
('mod-crossfaction-toc', 3, 'That item can no longer be exchanged.');

DELETE FROM `module_string_locale` WHERE `module` = 'mod-crossfaction-toc';
INSERT INTO `module_string_locale` (`module`, `id`, `locale`, `string`) VALUES
('mod-crossfaction-toc', 1, 'koKR', '교환할 수 있는 장비를 보여 주십시오.'),
('mod-crossfaction-toc', 1, 'frFR', 'Montrez-moi l''équipement que je peux échanger.'),
('mod-crossfaction-toc', 1, 'deDE', 'Zeigt mir, welche Ausrüstung ich eintauschen kann.'),
('mod-crossfaction-toc', 1, 'zhCN', '让我看看我可以兑换的装备。'),
('mod-crossfaction-toc', 1, 'zhTW', '讓我看看我可以兌換的裝備。'),
('mod-crossfaction-toc', 1, 'esES', 'Muéstrame el equipo que puedo intercambiar.'),
('mod-crossfaction-toc', 1, 'esMX', 'Muéstrame el equipo que puedo intercambiar.'),
('mod-crossfaction-toc', 1, 'ruRU', 'Покажите снаряжение, которое я могу обменять.'),

('mod-crossfaction-toc', 2, 'koKR', '교환할 수 있는 다른 진영의 아이템을 가지고 있지 않습니다. 전리품 거래 가능 시간이 남아 있는 아이템만 교환할 수 있습니다.'),
('mod-crossfaction-toc', 2, 'frFR', 'Vous ne portez aucun objet de l''autre faction pouvant être échangé. Seuls les objets encore dans leur délai d''échange après le butin sont acceptés.'),
('mod-crossfaction-toc', 2, 'deDE', 'Ihr tragt keine Gegenstände der anderen Fraktion bei Euch, die eingetauscht werden können. Nur Gegenstände, deren Handelszeitraum nach dem Plündern noch nicht abgelaufen ist, kommen in Frage.'),
('mod-crossfaction-toc', 2, 'zhCN', '你没有可以兑换的另一阵营物品。只有仍在拾取交易时限内的物品才符合条件。'),
('mod-crossfaction-toc', 2, 'zhTW', '你沒有可以兌換的另一陣營物品。只有仍在拾取交易時限內的物品才符合條件。'),
('mod-crossfaction-toc', 2, 'esES', 'No llevas objetos de la otra facción que se puedan intercambiar. Solo valen los objetos que aún están dentro de su plazo de comercio tras el botín.'),
('mod-crossfaction-toc', 2, 'esMX', 'No llevas objetos de la otra facción que se puedan intercambiar. Solo valen los objetos que aún están dentro de su plazo de comercio tras el botín.'),
('mod-crossfaction-toc', 2, 'ruRU', 'У вас нет предметов другой фракции, которые можно обменять. Подходят только предметы, срок передачи которых после получения добычи ещё не истек.'),

('mod-crossfaction-toc', 3, 'koKR', '이 아이템은 더 이상 교환할 수 없습니다.'),
('mod-crossfaction-toc', 3, 'frFR', 'Cet objet ne peut plus être échangé.'),
('mod-crossfaction-toc', 3, 'deDE', 'Dieser Gegenstand kann nicht mehr eingetauscht werden.'),
('mod-crossfaction-toc', 3, 'zhCN', '该物品已无法兑换。'),
('mod-crossfaction-toc', 3, 'zhTW', '該物品已無法兌換。'),
('mod-crossfaction-toc', 3, 'esES', 'Ese objeto ya no se puede intercambiar.'),
('mod-crossfaction-toc', 3, 'esMX', 'Ese objeto ya no se puede intercambiar.'),
('mod-crossfaction-toc', 3, 'ruRU', 'Этот предмет больше нельзя обменять.');
