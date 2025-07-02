#include "TriggerBot.h"
#include <thread>
#include <chrono>
#include <random>
#include "../Helpers/Logger.h"

void TriggerBot::Run(const CEntity& LocalEntity)
{
    if (MenuConfig::ShowMenu)
        return;

    if (!CanTrigger(LocalEntity))
    {
        g_HasValidTarget = false;
        g_CanShoot = false;
        return;
    }

    auto now = std::chrono::system_clock::now();

    // Handle shot duration cooldown
    if (g_CanShoot)
    {
        auto timeSinceShot = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - g_LastShotTime).count();

        if (timeSinceShot < ShotDuration)
        {
            return; // Still in shot cooldown
        }
        else
        {
            g_CanShoot = false; // Reset shot state
        }
    }

    // Process trigger logic if we have a valid target
    if (g_HasValidTarget)
    {
        // Handle trigger delay
        if (!g_CanShoot)
        {
            auto timeSinceFound = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - g_TargetFoundTime).count();

            if (timeSinceFound >= TriggerDelay)
            {
                g_CanShoot = true;
            }
        }

        // Execute shot if ready
        if (g_CanShoot && (GetAsyncKeyState(TriggerBot::HotKey) || LegitBotConfig::TriggerAlways))
        {
            ExecuteShot();
        }
    }
}

bool TriggerBot::CheckForValidHitbox(const CEntity& LocalEntity, const CEntity& Entity, bool IsVisible)
{
    if (!IsValidTarget(LocalEntity, Entity, IsVisible))
        return false;

    const auto& BoneList = Entity.GetBone().BonePosList;

    if (BoneList.empty())
        return false;

    // Define hitboxes to check
    const std::vector<BONEINDEX> hitboxesToCheck = {
        BONEINDEX::pelvis,
        BONEINDEX::spine_2,
        BONEINDEX::spine_1,
        BONEINDEX::neck_0,
        BONEINDEX::head,
        BONEINDEX::arm_upper_L,
        BONEINDEX::arm_lower_L,
        BONEINDEX::hand_L,
        BONEINDEX::arm_upper_R,
        BONEINDEX::arm_lower_R,
        BONEINDEX::hand_R,
        BONEINDEX::leg_upper_L,
        BONEINDEX::leg_lower_L,
        BONEINDEX::ankle_L,
        BONEINDEX::leg_upper_R,
        BONEINDEX::leg_lower_R,
        BONEINDEX::ankle_R
    };



    for (int hitboxIndex : hitboxesToCheck)
    {
        const BoneJointPos& bone = BoneList[hitboxIndex];
        float radius = GetHitboxRadius(Entity, hitboxIndex);
        //Gui.Circle(bone.ScreenPos, radius, ESPConfig::HeadBoxColor, 1.2f);

        if (CastRayToHitbox(LocalEntity, Entity, hitboxIndex, bone.ScreenPos, radius))
        {
            if (!g_HasValidTarget)
            {
                g_HasValidTarget = true;
                g_TargetFoundTime = std::chrono::system_clock::now();
                g_CanShoot = false;
            }
            // If we already had a target, keep the original timer
            return true;
        }
    }
    return false;
}

float TriggerBot::GetHitboxRadius(const CEntity& Entity, int hitboxIndex)
{
    const auto& BoneList = Entity.GetBone().BonePosList;

    // Get world radius for this hitbox type
    float worldRadius;
    switch (hitboxIndex)
    {
    case BONEINDEX::head:        worldRadius = 8.0f; break;
    case BONEINDEX::neck_0:      worldRadius = 6.0f; break;
    case BONEINDEX::spine_1:
    case BONEINDEX::spine_2:     worldRadius = 12.0f; break;
    case BONEINDEX::pelvis:      worldRadius = 10.0f; break;
    case BONEINDEX::hand_L:
    case BONEINDEX::hand_R:      worldRadius = 4.0f; break;
    case BONEINDEX::arm_upper_L:
    case BONEINDEX::arm_upper_R:
    case BONEINDEX::arm_lower_L:
    case BONEINDEX::arm_lower_R: worldRadius = 5.0f; break;
    default:                     worldRadius = 6.0f; break;
    }

    // Calculate scale using head-to-neck bone distance
    if (BONEINDEX::head < static_cast<int>(BoneList.size()) &&
        BONEINDEX::neck_0 < static_cast<int>(BoneList.size()))
    {
        const BoneJointPos& Head = BoneList[BONEINDEX::head];
        const BoneJointPos& Neck = BoneList[BONEINDEX::neck_0];

        float dx = Head.ScreenPos.x - Neck.ScreenPos.x;
        float dy = Head.ScreenPos.y - Neck.ScreenPos.y;
        float screenDistance = sqrt(dx * dx + dy * dy);

        float scaleFactor = screenDistance / 8.0f; // 8.0f is world head-neck distance
        return worldRadius * scaleFactor;
    }

    return 0.f;
}

bool TriggerBot::CastRayToHitbox(const CEntity& LocalEntity, const CEntity& Entity, int hitboxIndex, const Vec2& hitboxScreenPos, float radius)
{
    // Get crosshair center
    Vec2 crosshairCenter = { Gui.Window.Size.x / 2.0f, Gui.Window.Size.y / 2.0f };

    // Calculate distance from crosshair to hitbox center
    float distanceToHitbox = crosshairCenter.DistanceTo(hitboxScreenPos);

    // is the hitbox center within the radius on screen?
    return distanceToHitbox <= radius;
}

bool TriggerBot::IsValidTarget(const CEntity& LocalEntity, const CEntity& TargetEntity, bool Spotted)
{
    // Basic health check
    if (TargetEntity.Pawn.Health <= 0)
        return false;

    // Team check
    if (MenuConfig::TeamCheck && TargetEntity.Controller.TeamID == LocalEntity.Controller.TeamID)
        return false;

    // Check if target is in a valid state
    if (TargetEntity.Pawn.Address == 0)
        return false;

    // Visibility check
    if (VisibleCheck && !Spotted)
        return false;

    return true;
}

bool TriggerBot::CanTrigger(const CEntity& LocalEntity)
{
    // Check if player is alive
    if (LocalEntity.Controller.AliveStatus == 0)
        return false;

    // Check if weapon is ready
    bool waitForNoAttack = false;
    if (!memoryManager.ReadMemory<bool>(LocalEntity.Pawn.Address + Offset.Pawn.m_bWaitForNoAttack, waitForNoAttack))
        return false;

    if (waitForNoAttack)
        return false;

    // Check weapon type
    std::string currentWeapon = GetWeapon(LocalEntity);
    if (!CheckWeapon(currentWeapon))
        return false;

    // Check flash duration
    if (!IgnoreFlash && LocalEntity.Pawn.FlashDuration > 0.0f)
        return false;

    // Check scope requirement
    if (ScopeOnly && CheckScopeWeapon(currentWeapon))
    {
        bool isScoped = false;
        memoryManager.ReadMemory<bool>(LocalEntity.Pawn.Address + Offset.Pawn.isScoped, isScoped);
        if (!isScoped)
            return false;
    }

    return true;
}

void TriggerBot::ExecuteShot()
{
    // Check if already shooting to avoid double-click
    if (GetAsyncKeyState(VK_LBUTTON) < 0)
        return;

    // Update timing
    g_LastShotTime = std::chrono::system_clock::now();

    // Execute shot with random timing
    std::random_device RandomDevice;
    std::mt19937 RandomNumber(RandomDevice());
    std::uniform_int_distribution<> Range(1, 5);
    auto rand = std::chrono::microseconds(Range(RandomNumber));

    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    std::this_thread::sleep_for(std::chrono::microseconds(Range(RandomNumber)));
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

std::string TriggerBot::GetWeapon(const CEntity& LocalEntity)
{
    // Single memory read to get the weapon pointer
    DWORD64 CurrentWeapon;
    if (!memoryManager.ReadMemory(LocalEntity.Pawn.Address + Offset.Pawn.pClippingWeapon, CurrentWeapon) || CurrentWeapon == 0)
        return "";

    // Calculate the final address for weapon index directly
    DWORD64 weaponIndexAddress = CurrentWeapon + Offset.EconEntity.AttributeManager +
        Offset.WeaponBaseData.Item + Offset.WeaponBaseData.ItemDefinitionIndex;

    // Single memory read to get weapon index
    short weaponIndex;
    if (!memoryManager.ReadMemory(weaponIndexAddress, weaponIndex) || weaponIndex == -1)
        return "";

    // Inline weapon name lookup
    static const std::string defaultWeapon = "";
    auto it = CEntity::weaponNames.find(weaponIndex);
    return (it != CEntity::weaponNames.end()) ? it->second : defaultWeapon;
}

bool TriggerBot::CheckScopeWeapon(const std::string& WeaponName)
{
    return (WeaponName == "awp" || WeaponName == "g3Sg1" || WeaponName == "ssg08" || WeaponName == "scar20");
}

bool TriggerBot::CheckWeapon(const std::string& WeaponName)
{
    return !(WeaponName == "smokegrenade" || WeaponName == "flashbang" || WeaponName == "hegrenade" ||
        WeaponName == "molotov" || WeaponName == "decoy" || WeaponName == "incgrenade" ||
        WeaponName == "t_knife" || WeaponName == "ct_knife" || WeaponName == "c4");
}
