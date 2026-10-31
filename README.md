# mod-crossfaction-toc

Makes Trial of the Crusader (all four difficulties, map 649) friendly to cross-faction raids.

Every boss and chest there rolls an Alliance and a Horde loot reference, and each item is flagged for
one faction, so a mixed raid gets two sets and each player only sees half of the loot.

With this module:

- **One set drops.** References that contain only items of one faction roll only when that faction is
  the majority of the (non-GM) players inside the instance. Ties go to the looter's faction.
- **Everyone can loot it.** Inside the raid, any item listed in `player_factionchange_items` is visible
  and rollable for both factions. Equipping is still faction-locked.
- **Exchange NPC.** The Argent Exchange Officer (creature `210000`) lists the items in the player's bags
  that are locked to the other faction and swaps each for its counterpart from `player_factionchange_items`.
  The new item takes the old one's bag slot.
  - Only items still within their loot trade window qualify: 2 hours of played time after looting, and
    never enchanted or socketed (both end the window). This keeps the NPC to fresh raid loot.
  - The exchanged item is no longer tradeable. The gossip option asks for confirmation before swapping.
  - The trade window requires `Item.SetItemTradeable = 1` in `worldserver.conf` (the default).

## Requirements

Core hook `GlobalScript::CanLootOtherFactionItem` (called from `LootItem::AllowedForPlayer`), added in
[azerothcore/azerothcore-wotlk#28320](https://github.com/azerothcore/azerothcore-wotlk/pull/28320).

## Installation

1. Place the module in `modules/` and re-run CMake.
2. Apply `data/sql/db-world/updates/mod_crossfaction_toc_npc.sql` (or let the DB updater do it).
3. Copy `conf/mod-crossfaction-toc.conf.dist` to your config folder.
The SQL spawns the NPC in the Trial of the Crusader entrance hall at the Argent Tournament Grounds.
Add more spawns with `.npc add 210000`. Re-applying the SQL removes every spawn of the NPC and
recreates only the entrance one.

## Configuration

| Option | Default | Description |
|--------|---------|-------------|
| `CrossFactionToC.Enable` | `1` | Enables the loot filtering and cross-faction loot visibility. The exchange NPC works either way. |
