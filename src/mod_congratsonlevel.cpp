#include "Configuration/Config.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"

struct COL
{
    uint32 acoreMessageId;
    bool congratsAnnounce, congratsEnable;
    bool CongratsPerLevelEnable;
};

COL col;

uint32 giveAward(Player* player)
{
    QueryResult result = WorldDatabase.Query("SELECT * FROM `mod_congrats_on_level_items` WHERE `level`={} AND (`race`={} OR `race`=0) AND (`class`={} OR `class`=0)", player->getLevel(), player->getRace(), player->getClass());

    uint32 money = 0;

    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            if (fields[1].Get<uint32>() > 0)
            {
                player->ModifyMoney(fields[1].Get<uint32>() * GOLD);
                money += fields[1].Get<uint32>() * GOLD;
            }
            if (fields[2].Get<uint32>() > 0)
            {
                if (fields[3].Get<uint32>() == 0)
                    player->CastSpell(player, fields[2].Get<uint32>());
                else
                    player->learnSpell(fields[2].Get<uint32>());
            }
            if (fields[4].Get<uint32>() > 0)
                player->AddItem(fields[4].Get<uint32>(), 1);
            if (fields[5].Get<uint32>())
                player->AddItem(fields[5].Get<uint32>(), 1);
        }
        while (result->NextRow());

        return money;
    }
    return money;
}

std::string GetClassColor(Player* player)
{
    switch (player->getClass())
    {
        case CLASS_WARRIOR:      return "|cffC69B6D";
        case CLASS_PALADIN:      return "|cffF48CBA";
        case CLASS_HUNTER:       return "|cffAAD372";
        case CLASS_ROGUE:        return "|cffFFF468";
        case CLASS_PRIEST:       return "|cffFFFFFF";
        case CLASS_DEATH_KNIGHT: return "|cffC41E3A";
        case CLASS_SHAMAN:       return "|cff0070DD";
        case CLASS_MAGE:         return "|cff3FC7EB"; 
        case CLASS_WARLOCK:      return "|cff8788EE"; 
        case CLASS_DRUID:        return "|cffFF7C0A"; 
        default:                 return "|cff636363"; 
    }
}

class CongratsAnnounce : public PlayerScript
{
public:
    CongratsAnnounce() : PlayerScript("CongratsAnnounce") {}

    void OnLogin(Player* player) override
    {
        // Announce module
        if (col.congratsAnnounce)
        {
            ChatHandler(player->GetSession()).SendSysMessage(col.acoreMessageId);
        }
    }
};

class CongratsOnLevel : public PlayerScript
{
public:
    CongratsOnLevel() : PlayerScript("CongratsOnLevel") { }

    void OnLevelChanged(Player* player, uint8 oldLevel) override
    {
        if (col.congratsEnable && col.CongratsPerLevelEnable)
        {
            uint8 level = player->getLevel();
            uint32 money = 0;
            std::string classColor = GetClassColor(player);

            switch (level)
            {
                // Level 2 reward
                case 2:
                    if (oldLevel < 2)
                    {
						// Pumpkin bag / 16 slot BoP
                        player->AddItem(20400, 1);

                        // Reward notification
                        switch (player->GetSession()->GetSessionDbLocaleIndex())
                        {
                            case LOCALE_enUS:
                            case LOCALE_koKR:
                            case LOCALE_frFR:
                            case LOCALE_deDE:
                            case LOCALE_zhCN:
                            case LOCALE_zhTW:
                            case LOCALE_ruRU:
                                // Send a whisper to the player
                                ChatHandler(player->GetSession()).PSendSysMessage("|cFFFFA500Thank you for playing, %s! You've been awarded a bag. Good luck!|r", player->GetName().c_str());
                                break;

                            case LOCALE_esES:
                            case LOCALE_esMX:
                                ChatHandler(player->GetSession()).PSendSysMessage("|cFFFFA500¡Gracias por jugar, %s! Se le ha concedido una bag. ¡Buena suerte!|r", player->GetName().c_str());
                                break;

                            default:
                                break;
                        }
                    }
                    break;

                default:
					// Announcement and reward on every 10th level
                    if (level % 10 == 0 && oldLevel < level)
                    {
                        money = giveAward(player); 

                        // Server announcement
                        std::ostringstream ss;
                        switch (player->GetSession()->GetSessionDbLocaleIndex())
                        {
                            case LOCALE_enUS:
                            case LOCALE_koKR:
                            case LOCALE_frFR:
                            case LOCALE_deDE:
                            case LOCALE_zhCN:
                            case LOCALE_zhTW:
                            case LOCALE_ruRU:
                                ss << "|cffFFFFFF[ DING! ] : " << classColor << player->GetName() << "|cffFFFFFF has reached " << classColor << "Level " << std::to_string(player->getLevel());
                                break;

                            case LOCALE_esES:
                            case LOCALE_esMX:
                                ss << "|cffFFFFFF[ FELICITACIONES! ] : |cff4CFF00 " << player->GetName() << " |cffFFFFFFha alcanzado |cff4CFF00el nivel " << std::to_string(player->getLevel()) << "|cffFFFFFF!";
                                break;

                            default:
                                break;
                        }
                        sWorld->SendServerMessage(SERVER_MSG_STRING, ss.str().c_str());

                        // Reward notification
                        std::ostringstream ss2;
                        switch (player->GetSession()->GetSessionDbLocaleIndex())
                        {
                            case LOCALE_enUS:
                            case LOCALE_koKR:
                            case LOCALE_frFR:
                            case LOCALE_deDE:
                            case LOCALE_zhCN:
                            case LOCALE_zhTW:
                            case LOCALE_ruRU:
								// Send a whisper to the player
                                ss2 << "|cFFFFA500Congrats on Level " << std::to_string(player->getLevel()) << ", " << player->GetName() << "! You've been awarded " << money / 10000  << " gold and a few treasures!|r";
                                ChatHandler(player->GetSession()).PSendSysMessage(ss2.str().c_str());
                                break;

                            case LOCALE_esES:
                            case LOCALE_esMX:
								// Send a whisper to the player
                                ss2 << "|cFFFFA500¡Felicitaciones por alcanzar el Nivel " << std::to_string(player->getLevel()) << ", " << player->GetName() << "! Se le ha concedido " << money / 10000 << " de oro y algunos tesoros!|r";
                                ChatHandler(player->GetSession()).PSendSysMessage(ss2.str().c_str());
                                break;
								
                            default:
                                break;
                        }
                    }
                    break;
            }
        }
    }
};

class ModCongratsLevelWorldScript : public WorldScript
{
public:
    ModCongratsLevelWorldScript() : WorldScript("ModCongratsLevelWorldScript") { }

    void OnBeforeConfigLoad(bool reload) override
    {
        if (!reload)
        {
            sConfigMgr->LoadModulesConfigs();
            col.acoreMessageId = sConfigMgr->GetOption<uint32>("Congrats.Acore.String.ID", 60000);
            col.congratsAnnounce = sConfigMgr->GetOption<uint32>("Congrats.Announce", 1);
            col.congratsEnable = sConfigMgr->GetOption<uint32>("Congrats.Enable", 1);
            col.CongratsPerLevelEnable = sConfigMgr->GetOption<uint32>("CongratsPerLevel.Enable", 1);
        }
    }
};

void AddCongratsOnLevelScripts()
{
    new CongratsAnnounce();
    new CongratsOnLevel();
    new ModCongratsLevelWorldScript();
}
