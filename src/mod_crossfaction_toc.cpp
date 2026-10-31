/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "AreaDefines.h"
#include "Bag.h"
#include "Chat.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "GossipDef.h"
#include "Group.h"
#include "Item.h"
#include "Log.h"
#include "LootMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "WorldPacket.h"
#include <unordered_map>
#include <vector>

namespace
{
    constexpr uint32 NPC_TEXT_EXCHANGE_OFFICER = 210000;
    constexpr char const* MODULE_NAME = "mod-crossfaction-toc";
    constexpr char const* EXCHANGE_SCRIPT_NAME = "npc_crossfaction_toc_exchange";

    enum ModuleStrings : uint32
    {
        LANG_EXCHANGE_BROWSE      = 1,
        LANG_EXCHANGE_NO_ITEMS    = 2,
        LANG_EXCHANGE_UNAVAILABLE = 3
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

    // Returns the counterpart entry the player can use in place of an item locked to the other faction, or 0.
    // Only items still in their loot trade window qualify; enchanting or socketing an item also ends that window.
    uint32 GetUsableCounterpart(Player const* player, Item const* item)
    {
        if (!item->IsBOPTradable())
            return 0;

        ItemTemplate const* proto = item->GetTemplate();
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

    // The looter rotates between kills, so it only breaks ties when the group leader is offline
    TeamId GetTieBreakTeam(Player const* looter)
    {
        if (Group const* group = looter->GetGroup())
            if (Player const* leader = ObjectAccessor::FindConnectedPlayer(group->GetLeaderGUID()))
                return leader->GetTeamId(true);

        return looter->GetTeamId(true);
    }

    // Ties go to the group leader's faction
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
            return GetTieBreakTeam(looter);

        return alliance > horde ? TEAM_ALLIANCE : TEAM_HORDE;
    }

    std::string const& GetText(Player const* player, uint32 id)
    {
        return *player->GetSession()->GetModuleString(MODULE_NAME, id);
    }

    void SendError(Player const* player, uint32 id)
    {
        ChatHandler(player->GetSession()).SendNotification(GetText(player, id));
    }

    bool IsExchangeOfficer(Creature const* creature)
    {
        return creature && creature->GetScriptName() == EXCHANGE_SCRIPT_NAME;
    }

    struct ExchangeOffer
    {
        Item* item;
        uint32 counterpart;
    };

    // Vendor slots index into this list, so its order must stay the same between listing and buying
    std::vector<ExchangeOffer> GetExchangeOffers(Player* player)
    {
        std::vector<ExchangeOffer> offers;
        auto addOffer = [&](Item* item)
        {
            if (offers.size() >= MAX_VENDOR_ITEMS)
                return;

            if (uint32 counterpart = GetUsableCounterpart(player, item))
                offers.push_back({ item, counterpart });
        };

        for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                addOffer(item);

        for (uint8 bagSlot = INVENTORY_SLOT_BAG_START; bagSlot < INVENTORY_SLOT_BAG_END; ++bagSlot)
            if (Bag* bag = player->GetBagByPos(bagSlot))
                for (uint8 slot = 0; slot < bag->GetBagSize(); ++slot)
                    if (Item* item = bag->GetItemByPos(slot))
                        addOffer(item);

        return offers;
    }

    // Lists the counterparts as free goods; buying one is intercepted and performs the exchange
    void SendExchangeList(Player* player, Creature const* creature, std::vector<ExchangeOffer> const& offers)
    {
        WorldPacket data(SMSG_LIST_INVENTORY, 8 + 1 + offers.size() * 8 * 4);
        data << creature->GetGUID();
        data << uint8(offers.size());

        uint32 vendorSlot = 0;
        for (ExchangeOffer const& offer : offers)
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(offer.counterpart);
            data << uint32(++vendorSlot);
            data << uint32(proto->ItemId);
            data << uint32(proto->DisplayInfoID);
            data << int32(-1); // Unlimited stock
            data << uint32(0); // Price
            data << uint32(proto->MaxDurability);
            data << uint32(1); // Buy count
            data << uint32(0); // Extended cost
        }

        player->GetSession()->SendPacket(&data);
    }

    void ExchangeItem(Player* player, Item* item, uint32 newEntry)
    {
        uint32 oldEntry = item->GetEntry();

        // Full bags are fine since the new item takes the old one's slot; anything else (e.g. unique) is not
        ItemPosCountVec check;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, check, newEntry, 1);
        if (msg != EQUIP_ERR_OK && msg != EQUIP_ERR_INVENTORY_FULL)
        {
            player->SendEquipError(msg, nullptr, nullptr, newEntry);
            return;
        }

        uint8 bag = item->GetBagSlot();
        uint8 slot = item->GetSlot();
        player->DestroyItem(bag, slot, true);

        ItemPosCountVec dest;
        msg = player->CanStoreNewItem(bag, slot, dest, newEntry, 1);
        if (msg != EQUIP_ERR_OK)
        {
            LOG_ERROR("module", "mod-crossfaction-toc: {} lost item {} exchanging it for {} (error {})", player->GetName(), oldEntry, newEntry, static_cast<uint32>(msg));
            player->SendEquipError(msg, nullptr, nullptr, newEntry);
            return;
        }

        Item* newItem = player->StoreNewItem(dest, newEntry, true, Item::GenerateItemRandomPropertyId(newEntry));
        if (!newItem)
            return;

        player->SendNewItem(newItem, 1, true, false);
        LOG_INFO("module", "mod-crossfaction-toc: {} exchanged item {} for {}", player->GetName(), oldEntry, newEntry);
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

class mod_crossfaction_toc_playerscript : public PlayerScript
{
public:
    mod_crossfaction_toc_playerscript() : PlayerScript("mod_crossfaction_toc_playerscript", {
        PLAYERHOOK_ON_BEFORE_BUY_ITEM_FROM_VENDOR,
        PLAYERHOOK_CAN_SELL_ITEM
    }) { }

    void OnPlayerBeforeBuyItemFromVendor(Player* player, ObjectGuid vendorguid, uint32 vendorslot, uint32& itemEntry, uint8 /*count*/, uint8 /*bag*/, uint8 /*slot*/) override
    {
        if (!IsExchangeOfficer(player->GetMap()->GetCreature(vendorguid)))
            return;

        uint32 requested = itemEntry;
        itemEntry = 0; // Skips the core vendor handling

        Creature* creature = player->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_VENDOR);
        if (!creature)
            return;

        std::vector<ExchangeOffer> offers = GetExchangeOffers(player);
        if (vendorslot < offers.size() && offers[vendorslot].counterpart == requested && !offers[vendorslot].item->IsInTrade())
        {
            ExchangeItem(player, offers[vendorslot].item, requested);
            offers = GetExchangeOffers(player);
        }
        else
            SendError(player, LANG_EXCHANGE_UNAVAILABLE);

        SendExchangeList(player, creature, offers);
    }

    // Right-clicking a bag item while the exchange list is open would otherwise sell it
    bool OnPlayerCanSellItem(Player* /*player*/, Item* /*item*/, Creature* creature) override
    {
        return !IsExchangeOfficer(creature);
    }
};

class npc_crossfaction_toc_exchange : public CreatureScript
{
public:
    npc_crossfaction_toc_exchange() : CreatureScript(EXCHANGE_SCRIPT_NAME) { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON_VENDOR, GetText(player, LANG_EXCHANGE_BROWSE), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        SendGossipMenuFor(player, NPC_TEXT_EXCHANGE_OFFICER, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 /*action*/) override
    {
        CloseGossipMenuFor(player);

        std::vector<ExchangeOffer> offers = GetExchangeOffers(player);
        if (offers.empty())
        {
            SendError(player, LANG_EXCHANGE_NO_ITEMS);
            return true;
        }

        SendExchangeList(player, creature, offers);
        return true;
    }
};

void AddSC_mod_crossfaction_toc()
{
    new mod_crossfaction_toc_worldscript();
    new mod_crossfaction_toc_globalscript();
    new mod_crossfaction_toc_playerscript();
    new npc_crossfaction_toc_exchange();
}
