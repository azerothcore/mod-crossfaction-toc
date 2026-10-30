/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Bag.h"
#include "Chat.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "GossipDef.h"
#include "Item.h"
#include "Log.h"
#include "LootMgr.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "StringFormat.h"
#include <array>
#include <unordered_map>

namespace
{
    constexpr uint32 MAP_TRIAL_OF_THE_CRUSADER = 649;

    // Enchant slots carried over to the exchanged item; temporary and random property slots are left out
    constexpr std::array<EnchantmentSlot, 6> CopiedEnchantSlots =
    {
        PERM_ENCHANTMENT_SLOT,
        SOCK_ENCHANTMENT_SLOT,
        SOCK_ENCHANTMENT_SLOT_2,
        SOCK_ENCHANTMENT_SLOT_3,
        BONUS_ENCHANTMENT_SLOT,
        PRISMATIC_ENCHANTMENT_SLOT
    };

    bool Enabled = false;

    // Reference loot templates holding only items locked to a single faction
    std::unordered_map<uint32, TeamId> FactionReferences;

    // player_factionchange_items in both directions
    std::unordered_map<uint32, uint32> CounterpartItems;

    void LoadFactionReferences()
    {
        FactionReferences.clear();

        enum ReferenceContent : uint8
        {
            CONTENT_ALLIANCE = 0x1,
            CONTENT_HORDE    = 0x2,
            CONTENT_OTHER    = 0x4
        };

        QueryResult result = WorldDatabase.Query("SELECT `Entry`, `Item`, `Reference` FROM `reference_loot_template`");
        if (!result)
            return;

        std::unordered_map<uint32, uint8> contents;
        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].Get<uint32>();
            uint32 itemId = fields[1].Get<uint32>();
            int32 reference = fields[2].Get<int32>();

            uint8& content = contents[entry];
            ItemTemplate const* proto = reference ? nullptr : sObjectMgr->GetItemTemplate(itemId);
            if (proto && proto->HasFlag2(ITEM_FLAG2_FACTION_ALLIANCE))
                content |= CONTENT_ALLIANCE;
            else if (proto && proto->HasFlag2(ITEM_FLAG2_FACTION_HORDE))
                content |= CONTENT_HORDE;
            else
                content |= CONTENT_OTHER;
        } while (result->NextRow());

        for (auto const& [entry, content] : contents)
        {
            if (content == CONTENT_ALLIANCE)
                FactionReferences[entry] = TEAM_ALLIANCE;
            else if (content == CONTENT_HORDE)
                FactionReferences[entry] = TEAM_HORDE;
        }

        LOG_INFO("server.loading", ">> mod-crossfaction-toc: {} single-faction loot references", FactionReferences.size());
    }

    void LoadCounterpartItems()
    {
        CounterpartItems.clear();

        for (auto const& [allianceItem, hordeItem] : sObjectMgr->FactionChangeItems)
        {
            CounterpartItems[allianceItem] = hordeItem;
            CounterpartItems.emplace(hordeItem, allianceItem);
        }
    }

    bool IsLockedToOtherFaction(ItemTemplate const* proto, TeamId team)
    {
        return (proto->HasFlag2(ITEM_FLAG2_FACTION_HORDE) && team != TEAM_HORDE) ||
            (proto->HasFlag2(ITEM_FLAG2_FACTION_ALLIANCE) && team != TEAM_ALLIANCE);
    }

    // Returns the counterpart entry the player can use in place of an item locked to the other faction, or 0
    uint32 GetUsableCounterpart(Player const* player, ItemTemplate const* proto)
    {
        TeamId team = player->GetTeamId(true);
        if (!IsLockedToOtherFaction(proto, team))
            return 0;

        auto itr = CounterpartItems.find(proto->ItemId);
        if (itr == CounterpartItems.end())
            return 0;

        ItemTemplate const* counterpart = sObjectMgr->GetItemTemplate(itr->second);
        if (!counterpart || IsLockedToOtherFaction(counterpart, team))
            return 0;

        return itr->second;
    }

    // Ties go to the looter's faction
    TeamId GetMajorityTeam(Player const* looter)
    {
        uint32 alliance = 0;
        uint32 horde = 0;

        for (auto const& itr : looter->GetMap()->GetPlayers())
        {
            Player* player = itr.GetSource();
            if (!player || player->IsGameMaster())
                continue;

            if (player->GetTeamId(true) == TEAM_ALLIANCE)
                ++alliance;
            else
                ++horde;
        }

        if (alliance == horde)
            return looter->GetTeamId(true);

        return alliance > horde ? TEAM_ALLIANCE : TEAM_HORDE;
    }

    std::string GetItemName(Player const* player, ItemTemplate const* proto)
    {
        std::string name = proto->Name1;
        if (ItemLocale const* locale = sObjectMgr->GetItemLocale(proto->ItemId))
            ObjectMgr::GetLocaleString(locale->Name, player->GetSession()->GetSessionDbLocaleIndex(), name);

        return name;
    }
}

class mod_crossfaction_toc_worldscript : public WorldScript
{
public:
    mod_crossfaction_toc_worldscript() : WorldScript("mod_crossfaction_toc_worldscript", {
        WORLDHOOK_ON_AFTER_CONFIG_LOAD,
        WORLDHOOK_ON_STARTUP
    }) { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        Enabled = sConfigMgr->GetOption<bool>("CrossFactionToC.Enable", true);
    }

    void OnStartup() override
    {
        LoadFactionReferences();
        LoadCounterpartItems();
    }
};

class mod_crossfaction_toc_globalscript : public GlobalScript
{
public:
    mod_crossfaction_toc_globalscript() : GlobalScript("mod_crossfaction_toc_globalscript", {
        GLOBALHOOK_ON_ITEM_ROLL,
        GLOBALHOOK_CAN_LOOT_OTHER_FACTION_ITEM
    }) { }

    // Each boss rolls one Alliance and one Horde reference; keep only the one matching the raid majority
    bool OnItemRoll(Player const* player, LootStoreItem const* lootStoreItem, float& /*chance*/, Loot& /*loot*/, LootStore const& /*store*/) override
    {
        if (!Enabled || !player || !lootStoreItem->reference || player->GetMapId() != MAP_TRIAL_OF_THE_CRUSADER)
            return true;

        auto itr = FactionReferences.find(std::abs(lootStoreItem->reference));
        if (itr == FactionReferences.end())
            return true;

        return itr->second == GetMajorityTeam(player);
    }

    // Equipping stays faction locked; players exchange the item at the NPC
    bool CanLootOtherFactionItem(Player const* player, ItemTemplate const* proto, ObjectGuid /*source*/) override
    {
        return Enabled && player->GetMapId() == MAP_TRIAL_OF_THE_CRUSADER && CounterpartItems.contains(proto->ItemId);
    }
};

class npc_crossfaction_toc_exchange : public CreatureScript
{
public:
    npc_crossfaction_toc_exchange() : CreatureScript("npc_crossfaction_toc_exchange") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);

        uint32 options = 0;
        auto addOption = [&](Item const* item)
        {
            if (options >= GOSSIP_MAX_MENU_ITEMS)
                return;

            uint32 counterpart = GetUsableCounterpart(player, item->GetTemplate());
            if (!counterpart)
                return;

            std::string text = Acore::StringFormat("{} -> {}", GetItemName(player, item->GetTemplate()),
                GetItemName(player, sObjectMgr->GetItemTemplate(counterpart)));

            if (item->IsBOPTradable())
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, text, GOSSIP_SENDER_MAIN, item->GetGUID().GetCounter(),
                    "The exchanged item can no longer be traded to the players who were eligible to loot it.", 0, false);
            else
                AddGossipItemFor(player, GOSSIP_ICON_VENDOR, text, GOSSIP_SENDER_MAIN, item->GetGUID().GetCounter());

            ++options;
        };

        for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                addOption(item);

        for (uint8 bagSlot = INVENTORY_SLOT_BAG_START; bagSlot < INVENTORY_SLOT_BAG_END; ++bagSlot)
            if (Bag* bag = player->GetBagByPos(bagSlot))
                for (uint32 slot = 0; slot < bag->GetBagSize(); ++slot)
                    if (Item* item = bag->GetItemByPos(slot))
                        addOption(item);

        if (!options)
            ChatHandler(player->GetSession()).SendSysMessage("You carry no items of the other faction that can be exchanged.");

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action) override
    {
        CloseGossipMenuFor(player);

        Item* item = player->GetItemByGuid(ObjectGuid::Create<HighGuid::Item>(action));
        if (!item || item->IsInTrade() || !Player::IsInventoryPos(item->GetPos()))
            return true;

        uint32 oldEntry = item->GetEntry();
        uint32 newEntry = GetUsableCounterpart(player, item->GetTemplate());
        if (!newEntry)
            return true;

        // Full bags are fine since the new item takes the old one's slot; anything else (e.g. unique) is not
        ItemPosCountVec check;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, check, newEntry, 1);
        if (msg != EQUIP_ERR_OK && msg != EQUIP_ERR_INVENTORY_FULL)
        {
            player->SendEquipError(msg, nullptr, nullptr, newEntry);
            return true;
        }

        std::array<uint32, CopiedEnchantSlots.size()> enchants;
        for (std::size_t i = 0; i < CopiedEnchantSlots.size(); ++i)
            enchants[i] = item->GetEnchantmentId(CopiedEnchantSlots[i]);

        uint8 bag = item->GetBagSlot();
        uint8 slot = item->GetSlot();
        player->DestroyItem(bag, slot, true);

        ItemPosCountVec dest;
        msg = player->CanStoreNewItem(bag, slot, dest, newEntry, 1);
        if (msg != EQUIP_ERR_OK)
        {
            LOG_ERROR("module", "mod-crossfaction-toc: {} lost item {} exchanging it for {} (error {})", player->GetName(), oldEntry, newEntry, static_cast<uint32>(msg));
            player->SendEquipError(msg, nullptr, nullptr, newEntry);
            return true;
        }

        Item* newItem = player->StoreNewItem(dest, newEntry, true, Item::GenerateItemRandomPropertyId(newEntry));
        if (!newItem)
            return true;

        for (std::size_t i = 0; i < CopiedEnchantSlots.size(); ++i)
            if (enchants[i])
                newItem->SetEnchantment(CopiedEnchantSlots[i], enchants[i], 0, 0);

        player->SendNewItem(newItem, 1, true, false);
        LOG_INFO("module", "mod-crossfaction-toc: {} exchanged item {} for {}", player->GetName(), oldEntry, newEntry);
        return true;
    }
};

void AddSC_mod_crossfaction_toc()
{
    new mod_crossfaction_toc_worldscript();
    new mod_crossfaction_toc_globalscript();
    new npc_crossfaction_toc_exchange();
}
