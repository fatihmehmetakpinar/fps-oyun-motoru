#pragma once

#include <cmath>
#include <cstdint>
#include <numbers>
#include <algorithm>
#include <vector>
#include <array>

// ============================================================================
// 1. ÖNBELLEK DOSTU 3B VEKTÖR MATEMATİĞİ (Z-Up Source Standardı)
// ============================================================================
struct Vec3 {
    float x{0.0f}, y{0.0f}, z{0.0f};

    constexpr Vec3() = default;
    constexpr Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    constexpr Vec3 operator+(const Vec3& r) const { return {x + r.x, y + r.y, z + r.z}; }
    constexpr Vec3 operator-(const Vec3& r) const { return {x - r.x, y - r.y, z - r.z}; }
    constexpr Vec3 operator-() const { return {-x, -y, -z}; }
    constexpr Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    constexpr Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }

    constexpr Vec3& operator+=(const Vec3& r) { x += r.x; y += r.y; z += r.z; return *this; }
    constexpr Vec3& operator-=(const Vec3& r) { x -= r.x; y -= r.y; z -= r.z; return *this; }
    constexpr Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }

    [[nodiscard]] constexpr float Dot(const Vec3& r) const {
        return x * r.x + y * r.y + z * r.z;
    }

    [[nodiscard]] constexpr Vec3 Cross(const Vec3& r) const {
        return {
            y * r.z - z * r.y,
            z * r.x - x * r.z,
            x * r.y - y * r.x
        };
    }

    [[nodiscard]] float LengthSq() const { return x * x + y * y + z * z; }
    [[nodiscard]] float Length() const { return std::sqrt(LengthSq()); }
    [[nodiscard]] float Length2D() const { return std::sqrt(x * x + y * y); }

    float NormalizeInPlace() {
        const float len = Length();
        if (len > 1e-6f) {
            const float inv = 1.0f / len;
            x *= inv; y *= inv; z *= inv;
        }
        return len;
    }

    [[nodiscard]] Vec3 Normalized() const {
        Vec3 v = *this;
        v.NormalizeInPlace();
        return v;
    }

    [[nodiscard]] static constexpr Vec3 Lerp(const Vec3& a, const Vec3& b, float t) {
        return a + (b - a) * t;
    }

    [[nodiscard]] static Vec3 Min(const Vec3& a, const Vec3& b) {
        return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) };
    }

    [[nodiscard]] static Vec3 Max(const Vec3& a, const Vec3& b) {
        return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) };
    }
};

// ============================================================================
// 2. SİLAH VE EL BOMBASI PROFİLLERİ (1: AK-47, 2: DEAGLE, 3: AWP, 4: HE, 5: FLASH)
// ============================================================================
enum class WeaponID : uint8_t {
    AK47      = 0,
    Deagle    = 1,
    AWP       = 2,
    HEGrenade = 3,
    Flashbang = 4
};

struct WeaponSpec {
    const char* name{"AK-47"};
    float       baseDamage{36.0f};
    float       armorPenetration{0.775f};
    float       penetrationPower{2.0f};
    float       rangeModifier{0.98f};
    float       fireInterval{0.096f};
    float       maxGroundSpeed{215.0f};
    float       scopedGroundSpeed{100.0f};
    float       baseInaccStand{0.0045f};
    float       baseInaccDuck{0.0022f};
    float       unscopedPenalty{0.0f};
    float       moveInaccScale{0.048f};
    bool        isAutomatic{true};
    bool        hasScope{false};
    bool        isGrenade{false};
};

// ============================================================================
// 3. YÜZEY MATERYALLERİ, FIRÇALAR VE DÜZLEŞTİRİLMİŞ BVH AĞACI
// ============================================================================
enum class MaterialType : uint8_t {
    Concrete = 0,
    Wood     = 1,
    Metal    = 2
};

struct SurfaceProperties {
    float penetrationModifier{1.0f};
    float damageModifier{0.80f};
};

struct Plane {
    Vec3  normal{0.0f, 0.0f, 1.0f};
    float dist{0.0f};
};

struct Brush {
    Vec3               mins{};
    Vec3               maxs{};
    Vec3               color{0.25f, 0.30f, 0.38f};
    MaterialType       material{MaterialType::Concrete};
    std::vector<Plane> planes{};
};

struct alignas(32) BVHNode {
    Vec3     mins{};
    uint32_t leftFirst{0};
    Vec3     maxs{};
    uint32_t brushCount{0};

    [[nodiscard]] constexpr bool IsLeaf() const { return brushCount > 0; }
};

struct CollisionWorld {
    std::vector<Brush>    brushes{};
    std::vector<BVHNode>  bvhNodes{};
    std::vector<uint32_t> brushIndices{};
    mutable uint32_t      lastTraceBrushTests{0};

    void BuildTestArena();
    void AddBox(const Vec3& mins, const Vec3& maxs, const Vec3& color, MaterialType mat = MaterialType::Concrete);
    void AddSlopeRamp(const Vec3& mins, const Vec3& maxs, const Vec3& slopeNormal, float slopeDist, const Vec3& color);
    void BuildBVH();

    [[nodiscard]] static constexpr SurfaceProperties GetSurfaceProps(MaterialType mat) {
        switch (mat) {
            case MaterialType::Wood:     return {2.60f, 0.90f};
            case MaterialType::Concrete: return {0.95f, 0.72f};
            case MaterialType::Metal:    return {0.35f, 0.50f};
        }
        return {1.0f, 0.75f};
    }

private:
    void SubdivideBVHNode(uint32_t nodeIdx, uint32_t depth);
    void UpdateNodeBounds(uint32_t nodeIdx);
};

struct TraceResult {
    float        fraction{1.0f};
    Vec3         endPos{};
    Vec3         planeNormal{0.0f, 0.0f, 1.0f};
    bool         startSolid{false};
    bool         allSolid{false};
    int          hitBrushIndex{-1};
    MaterialType hitMaterial{MaterialType::Concrete};
};

// ============================================================================
// 4. BÖLGESEL HITBOX, WALLBANG VE EL BOMBASI PROJEKTİL YAPILARI
// ============================================================================
enum class HitGroup : uint8_t {
    Generic  = 0,
    Head     = 1,
    Chest    = 2,
    Stomach  = 3,
    LeftLeg  = 4,
    RightLeg = 5
};

struct HitboxVolume {
    HitGroup group{HitGroup::Generic};
    bool     isCapsule{true};
    Vec3     capA{};
    Vec3     capB{};
    float    radius{6.0f};
    Vec3     boxMins{};
    Vec3     boxMaxs{};
    Vec3     debugColor{0.2f, 0.9f, 1.0f};
};

struct PenetrationImpact {
    Vec3 pos{};
    Vec3 normal{};
    bool isExit{false};
};

struct FireBulletResult {
    bool      hitPlayer{false};
    HitGroup  hitGroup{HitGroup::Generic};
    float     finalDamage{0.0f};
    uint8_t   penetratedCount{0};
    Vec3      finalEndPos{};
    uint8_t   impactCount{0};
    std::array<PenetrationImpact, 8> impacts{};
};

enum class GrenadeType : uint8_t {
    HE        = 0,
    Flashbang = 1
};

struct GrenadeProjectile {
    GrenadeType type{GrenadeType::HE};
    Vec3        origin{};
    Vec3        velocity{};
    float       fuseTimer{1.60f}; // 1.60 saniye sonra patlar
    bool        active{false};
};

// ============================================================================
// 5. USERCMD VE OYUNCU DURUMU
// ============================================================================
namespace Buttons {
    constexpr uint32_t NONE        = 0;
    constexpr uint32_t ATTACK      = (1 << 0);
    constexpr uint32_t JUMP        = (1 << 1);
    constexpr uint32_t DUCK        = (1 << 2);
    constexpr uint32_t FORWARD     = (1 << 3);
    constexpr uint32_t BACK        = (1 << 4);
    constexpr uint32_t MOVELEFT    = (1 << 5);
    constexpr uint32_t MOVERIGHT   = (1 << 6);
    constexpr uint32_t ATTACK2     = (1 << 7);
    constexpr uint32_t QUICK_HE    = (1 << 8); // G Tuşu: Hızlı HE Grenade Fırlat
    constexpr uint32_t QUICK_FLASH = (1 << 9); // C Tuşu: Hızlı Flashbang Fırlat
}

struct UserCmd {
    uint32_t command_number{0};
    uint32_t tick_count{0};
    float    subtick_fraction{0.0f};
    Vec3     viewangles{};
    float    forwardmove{0.0f};
    float    sidemove{0.0f};
    uint32_t buttons{0};
    uint8_t  weapon_select{0}; // 1..5
};

struct PlayerState {
    uint32_t     tick_count{0};
    Vec3         origin{0.0f, 0.0f, 0.0f};
    Vec3         velocity{0.0f, 0.0f, 0.0f};
    Vec3         viewangles{0.0f, 0.0f, 0.0f};
    float        view_offset_z{64.0f};
    bool         on_ground{false};
    Vec3         ground_normal{0.0f, 0.0f, 1.0f};
    MaterialType ground_material{MaterialType::Concrete};
    uint32_t     old_buttons{0};

    WeaponID active_weapon{WeaponID::AK47};
    uint8_t  scope_level{0};
    float    switch_cooldown{0.0f};

    Vec3     aim_punch_angle{0.0f, 0.0f, 0.0f};
    Vec3     aim_punch_angle_vel{0.0f, 0.0f, 0.0f};
    float    recoil_index{0.0f};
    float    weapon_cooldown{0.0f};
    float    inaccuracy{0.004f};
    uint32_t shots_fired{0};
    bool     fired_this_tick{false};

    // Bu tick içinde el bombası fırlatıldı mı?
    bool        threw_grenade_this_tick{false};
    GrenadeType thrown_grenade_type{GrenadeType::HE};
    float       thrown_grenade_speed{700.0f};
};

struct LagRecord {
    uint32_t tick_count{0};
    float    simulation_time{0.0f};
    Vec3     origin{};
};

struct MoveVars {
    float gravity           = 800.0f;
    float stopspeed         = 80.0f;
    float maxspeed          = 250.0f;
    float accelerate        = 5.5f;
    float airaccelerate     = 12.0f;
    float friction          = 5.2f;
    float jump_impulse      = 301.993377f;
    float air_wishspeed_cap = 30.0f;
    float stepsize          = 18.0f;
    float walkable_normal_z = 0.7f;
    float recoil_scale      = 2.0f;
    bool  autobhop          = true;
};

// ============================================================================
// 6. HAREKET, BALİSTİK VE BOMBA FİZİĞİ FONKSİYONLARI
// ============================================================================
namespace Movement {
    constexpr Vec3 PLAYER_MINS{-16.0f, -16.0f, 0.0f};
    constexpr Vec3 PLAYER_MAXS{ 16.0f,  16.0f, 72.0f};
    constexpr Vec3 GRENADE_MINS{-4.0f, -4.0f, -4.0f};
    constexpr Vec3 GRENADE_MAXS{ 4.0f,  4.0f,  4.0f};

    [[nodiscard]] const WeaponSpec& GetWeaponSpec(WeaponID id);

    void AngleVectors(const Vec3& angles, Vec3* forward, Vec3* right, Vec3* up);

    [[nodiscard]] TraceResult TraceBox(
        const CollisionWorld& world,
        const Vec3& start,
        const Vec3& end,
        const Vec3& mins,
        const Vec3& maxs
    );

    [[nodiscard]] TraceResult TraceRay(
        const CollisionWorld& world,
        const Vec3& start,
        const Vec3& dir,
        float maxDist
    );

    [[nodiscard]] bool RayIntersectAABB(
        const Vec3& rayOrigin,
        const Vec3& rayDir,
        const Vec3& boxMins,
        const Vec3& boxMaxs,
        float& outHitDist
    );

    [[nodiscard]] bool RayIntersectCapsule(
        const Vec3& rayOrigin,
        const Vec3& rayDir,
        const Vec3& capA,
        const Vec3& capB,
        float radius,
        float& outHitDist
    );

    [[nodiscard]] std::array<HitboxVolume, 5> BuildPlayerHitboxes(const Vec3& playerOrigin);
    [[nodiscard]] float ScaleDamageByHitGroup(float rawDamage, HitGroup group, float armorPenetration, bool targetHasArmor = true);
    [[nodiscard]] const char* GetHitGroupName(HitGroup group);

    [[nodiscard]] bool TraceToExit(
        const CollisionWorld& world,
        const Vec3& enterPos,
        const Vec3& rayDir,
        Vec3& outExitPos,
        Vec3& outExitNormal
    );

    [[nodiscard]] FireBulletResult FireBulletPenetrating(
        const CollisionWorld& world,
        WeaponID weapon,
        const Vec3& startPos,
        const Vec3& shotDir,
        const Vec3& rewoundTargetOrigin
    );

    // El Bombası Başlangıç Hızı Hesabı ve Sekme Adımı Çözücüsü (BVH Sweep + Esnek Yansıma)
    [[nodiscard]] Vec3 ComputeGrenadeThrowVelocity(
        const Vec3& viewangles,
        const Vec3& playerVelocity,
        float throwSpeed
    );

    // Tek bir zaman adımında bombayı ilerletir; duvara/zemine çarptıysa true döner
    bool SimulateGrenadeStep(
        GrenadeProjectile& proj,
        const CollisionWorld& world,
        float gravity,
        float dt,
        Vec3* outBouncePos = nullptr,
        float* outImpactSpeed = nullptr
    );

    void DecayAimPunchAngle(PlayerState& state, float dt);
    void UpdateWeaponAndRecoil(PlayerState& state, const UserCmd& cmd, const MoveVars& vars, float dt);

    [[nodiscard]] Vec3 ComputeShotDirection(
        const Vec3& viewangles,
        const Vec3& aim_punch_angle,
        float inaccuracy,
        uint32_t command_number,
        float recoil_scale
    );

    void ClipVelocity(const Vec3& in, const Vec3& normal, Vec3& out, float overbounce);
    void TryPlayerMove(PlayerState& state, const CollisionWorld& world, float dt);
    void StepSlideMove(PlayerState& state, const CollisionWorld& world, const MoveVars& vars, float dt);

    void CategorizePosition(PlayerState& state, const CollisionWorld& world, const MoveVars& vars);
    void CheckJumpButton(PlayerState& state, const UserCmd& cmd, const MoveVars& vars);
    void ApplyFriction(PlayerState& state, const MoveVars& vars, float dt);
    void Accelerate(PlayerState& state, const Vec3& wishdir, float wishspeed, float accel, float dt);
    void AirAccelerate(PlayerState& state, const Vec3& wishdir, float wishspeed, float accel, float air_cap, float dt);
    void WalkMove(PlayerState& state, const UserCmd& cmd, const CollisionWorld& world, const MoveVars& vars, float dt);
    void AirMove(PlayerState& state, const UserCmd& cmd, const CollisionWorld& world, const MoveVars& vars, float dt);

    [[nodiscard]] PlayerState SimulateTick(
        const PlayerState& current_state,
        const UserCmd& cmd,
        const CollisionWorld& world,
        const MoveVars& vars,
        float dt
    );

    [[nodiscard]] PlayerState InterpolateState(const PlayerState& prev, const PlayerState& curr, float alpha);
}