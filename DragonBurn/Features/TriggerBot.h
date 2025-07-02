#pragma once
#define _USE_MATH_DEFINES
#include <chrono>
#include <vector>
#include <math.h>

#include "../Game/Game.h"
#include "../Game/Entity.h"
#include "../Core/Config.h"

#undef min()
#undef max()

namespace TriggerBot
{

    // Configuration
    inline int TriggerDelay = 30; // ms
    inline int ShotDuration = 80; // ms
    inline bool ScopeOnly = true;
    inline bool IgnoreFlash = false;
    inline bool WorkWithAimbot = false;
    inline bool VisibleCheck = true;

    // Input configuration
    inline int HotKey = VK_XBUTTON2;

    // Timing variables
    inline std::chrono::time_point<std::chrono::system_clock> g_LastShotTime;
    inline std::chrono::time_point<std::chrono::system_clock> g_TargetFoundTime;
    inline bool g_HasValidTarget = false;
    inline bool g_CanShoot = false;

    // Main functions
    void Run(const CEntity& LocalEntity);

    void DrawHitboxes(c_game_scene_node* playerGameSceneNode);

    // Core detection function
    bool CheckForValidHitbox(const CEntity& LocalEntity, const CEntity& Entity, bool IsVisible);

    // Hitbox radius calculation based on bone positions
    float GetHitboxRadius(const CEntity& Entity, int hitboxIndex);

    // Ray casting from eye position to hitbox radius
    bool CastRayToHitbox(const CEntity& LocalEntity, const CEntity& Entity, int hitboxIndex, const Vec2& hitboxScreenPos, float radius);

    // Validation functions
    bool IsValidTarget(const CEntity& LocalEntity, const CEntity& TargetEntity, bool Spotted);
    bool CanTrigger(const CEntity& LocalEntity);

    // Execution functions
    void ExecuteShot();

    // Utility functions
    std::string GetWeapon(const CEntity& LocalEntity);
    bool CheckScopeWeapon(const std::string& WeaponName);
    bool CheckWeapon(const std::string& WeaponName);
}