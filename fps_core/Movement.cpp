#include "Movement.h"
#include <limits>

// ============================================================================
// 1. ARENA İNŞASI VE DÜZLEŞTİRİLMİŞ BVH AĞACI
// ============================================================================
void CollisionWorld::AddBox(const Vec3& mins, const Vec3& maxs, const Vec3& color, MaterialType mat) {
    Brush b{};
    b.mins     = mins;
    b.maxs     = maxs;
    b.color    = color;
    b.material = mat;
    b.planes   = {
        { Vec3{ 1.0f,  0.0f,  0.0f},  maxs.x },
        { Vec3{-1.0f,  0.0f,  0.0f}, -mins.x },
        { Vec3{ 0.0f,  1.0f,  0.0f},  maxs.y },
        { Vec3{ 0.0f, -1.0f,  0.0f}, -mins.y },
        { Vec3{ 0.0f,  0.0f,  1.0f},  maxs.z },
        { Vec3{ 0.0f,  0.0f, -1.0f}, -mins.z }
    };
    brushes.push_back(b);
}

void CollisionWorld::AddSlopeRamp(const Vec3& mins, const Vec3& maxs, const Vec3& slopeNormal, float slopeDist, const Vec3& color) {
    Brush b{};
    b.mins     = mins;
    b.maxs     = maxs;
    b.color    = color;
    b.material = MaterialType::Concrete;
    b.planes   = {
        { Vec3{ 1.0f,  0.0f,  0.0f},  maxs.x },
        { Vec3{-1.0f,  0.0f,  0.0f}, -mins.x },
        { Vec3{ 0.0f,  1.0f,  0.0f},  maxs.y },
        { Vec3{ 0.0f, -1.0f,  0.0f}, -mins.y },
        { Vec3{ 0.0f,  0.0f,  1.0f},  maxs.z },
        { Vec3{ 0.0f,  0.0f, -1.0f}, -mins.z },
        { slopeNormal.Normalized(),   slopeDist }
    };
    brushes.push_back(b);
}

void CollisionWorld::BuildTestArena() {
    brushes.clear();

    // 1. Ana Beton Zemin
    AddBox(Vec3{-4096.0f, -4096.0f, -64.0f}, Vec3{4096.0f, 4096.0f, 0.0f}, Vec3{0.14f, 0.16f, 0.20f}, MaterialType::Concrete);

    // 2. Arka Sprey & Bomba Sekme Duvarı
    AddBox(Vec3{680.0f, -360.0f, 0.0f}, Vec3{720.0f, 360.0f, 260.0f}, Vec3{0.26f, 0.28f, 0.34f}, MaterialType::Concrete);
    AddBox(Vec3{676.0f, -90.0f, 45.0f}, Vec3{680.0f,  90.0f, 175.0f}, Vec3{0.36f, 0.40f, 0.48f}, MaterialType::Wood);

    // 3. Wallbang & Ses Sönümleme (Occlusion) Siperleri (Ahşap, Beton, Metal)
    AddBox(Vec3{242.0f, -120.0f, 0.0f}, Vec3{258.0f, -20.0f, 92.0f}, Vec3{0.68f, 0.46f, 0.22f}, MaterialType::Wood);
    AddBox(Vec3{240.0f, -225.0f, 0.0f}, Vec3{260.0f, -125.0f, 92.0f}, Vec3{0.45f, 0.48f, 0.52f}, MaterialType::Concrete);
    AddBox(Vec3{234.0f, -310.0f, 0.0f}, Vec3{266.0f, -230.0f, 92.0f}, Vec3{0.22f, 0.28f, 0.36f}, MaterialType::Metal);

    // 4. Farklı Ayak Sesi Materyallerini Test Etmek İçin Yerde Ahşap ve Metal Platformlar
    AddBox(Vec3{-220.0f, -320.0f, 0.0f}, Vec3{-40.0f, -140.0f, 6.0f}, Vec3{0.58f, 0.38f, 0.18f}, MaterialType::Wood);
    AddBox(Vec3{-220.0f,  140.0f, 0.0f}, Vec3{-40.0f,  320.0f, 6.0f}, Vec3{0.28f, 0.36f, 0.46f}, MaterialType::Metal);

    // 5. StepSlideMove Merdivenleri (Metal Basamaklar + Ahşap Üst Platform)
    for (int i = 0; i < 6; ++i) {
        const float stepX0 = 250.0f + static_cast<float>(i) * 48.0f;
        const float stepX1 = stepX0 + 48.0f;
        const float stepZ  = static_cast<float>(i + 1) * 16.0f;
        AddBox(Vec3{stepX0, -560.0f, 0.0f}, Vec3{stepX1, -340.0f, stepZ}, Vec3{0.25f, 0.48f, 0.72f}, MaterialType::Metal);
    }
    AddBox(Vec3{538.0f, -560.0f, 0.0f}, Vec3{800.0f, -340.0f, 96.0f}, Vec3{0.55f, 0.38f, 0.20f}, MaterialType::Wood);

    // 6. Surf Rampası
    const Vec3 surfNormal = Vec3{0.0f, -0.8660254f, 0.5f}.Normalized();
    const float surfDist  = surfNormal.y * 380.0f;
    AddSlopeRamp(
        Vec3{-900.0f, 380.0f, 0.0f},
        Vec3{ 900.0f, 580.0f, 346.4f},
        surfNormal,
        surfDist,
        Vec3{0.85f, 0.42f, 0.15f}
    );

    // 7. Taktik Sütunlar (Flashbang Anti-Flash Siperleri)
    for (int gx = -3; gx <= 3; ++gx) {
        for (int gy = -3; gy <= 3; ++gy) {
            if (gx >= -1 && gx <= 2 && gy >= -1 && gy <= 1) continue;
            const float cx = static_cast<float>(gx) * 380.0f;
            const float cy = static_cast<float>(gy) * 380.0f - 200.0f;
            if (cy > 260.0f) continue;

            AddBox(Vec3{cx - 36.0f, cy - 36.0f, 0.0f},   Vec3{cx + 36.0f, cy + 36.0f, 160.0f}, Vec3{0.30f, 0.35f, 0.42f}, MaterialType::Concrete);
            AddBox(Vec3{cx - 48.0f, cy - 48.0f, 160.0f}, Vec3{cx + 48.0f, cy + 48.0f, 184.0f}, Vec3{0.38f, 0.44f, 0.52f}, MaterialType::Concrete);
        }
    }

    BuildBVH();
}

void CollisionWorld::BuildBVH() {
    bvhNodes.clear();
    brushIndices.resize(brushes.size());
    for (uint32_t i = 0; i < static_cast<uint32_t>(brushes.size()); ++i) {
        brushIndices[i] = i;
    }

    bvhNodes.reserve(brushes.size() * 2);

    BVHNode root{};
    root.leftFirst  = 0;
    root.brushCount = static_cast<uint32_t>(brushes.size());
    bvhNodes.push_back(root);

    UpdateNodeBounds(0);
    SubdivideBVHNode(0, 0);
}

void CollisionWorld::UpdateNodeBounds(uint32_t nodeIdx) {
    BVHNode& node = bvhNodes[nodeIdx];
    node.mins = Vec3{ 1e30f,  1e30f,  1e30f};
    node.maxs = Vec3{-1e30f, -1e30f, -1e30f};

    for (uint32_t i = 0; i < node.brushCount; ++i) {
        const Brush& b = brushes[brushIndices[node.leftFirst + i]];
        node.mins = Vec3::Min(node.mins, b.mins);
        node.maxs = Vec3::Max(node.maxs, b.maxs);
    }
}

void CollisionWorld::SubdivideBVHNode(uint32_t nodeIdx, uint32_t depth) {
    if (bvhNodes[nodeIdx].brushCount <= 2 || depth >= 16) return;

    const Vec3 extent = bvhNodes[nodeIdx].maxs - bvhNodes[nodeIdx].mins;
    int axis = 0;
    if (extent.y > extent.x) axis = 1;
    if (extent.z > ((axis == 1) ? extent.y : extent.x)) axis = 2;

    auto getAxisCenter = [&](uint32_t bIdx) -> float {
        const Brush& b = brushes[bIdx];
        if (axis == 0) return (b.mins.x + b.maxs.x) * 0.5f;
        if (axis == 1) return (b.mins.y + b.maxs.y) * 0.5f;
        return (b.mins.z + b.maxs.z) * 0.5f;
    };

    float splitPos = 0.0f;
    const uint32_t first = bvhNodes[nodeIdx].leftFirst;
    const uint32_t count = bvhNodes[nodeIdx].brushCount;
    for (uint32_t i = 0; i < count; ++i) {
        splitPos += getAxisCenter(brushIndices[first + i]);
    }
    splitPos /= static_cast<float>(count);

    int i = static_cast<int>(first);
    int j = static_cast<int>(first + count - 1);
    while (i <= j) {
        if (getAxisCenter(brushIndices[i]) < splitPos) {
            ++i;
        } else {
            std::swap(brushIndices[i], brushIndices[j]);
            --j;
        }
    }

    uint32_t leftCount = static_cast<uint32_t>(i) - first;
    if (leftCount == 0 || leftCount == count) {
        leftCount = count / 2;
    }

    const uint32_t leftChildIdx  = static_cast<uint32_t>(bvhNodes.size());
    const uint32_t rightChildIdx = leftChildIdx + 1;

    BVHNode leftNode{};
    leftNode.leftFirst  = first;
    leftNode.brushCount = leftCount;

    BVHNode rightNode{};
    rightNode.leftFirst  = first + leftCount;
    rightNode.brushCount = count - leftCount;

    bvhNodes.push_back(leftNode);
    bvhNodes.push_back(rightNode);

    bvhNodes[nodeIdx].leftFirst  = leftChildIdx;
    bvhNodes[nodeIdx].brushCount = 0;

    UpdateNodeBounds(leftChildIdx);
    UpdateNodeBounds(rightChildIdx);

    SubdivideBVHNode(leftChildIdx, depth + 1);
    SubdivideBVHNode(rightChildIdx, depth + 1);
}

// ============================================================================
// 2. SİLAH & BOMBA TABLOLARI, BALİSTİK VE HAREKET ÇÖZÜCÜ
// ============================================================================
namespace Movement {

constexpr float DIST_EPSILON = 0.03125f;

constexpr std::array<WeaponSpec, 5> WEAPON_SPECS = {{
    // [0] AK-47
    WeaponSpec{
        .name              = "AK-47",
        .baseDamage        = 36.0f,
        .armorPenetration  = 0.775f,
        .penetrationPower  = 2.0f,
        .rangeModifier     = 0.98f,
        .fireInterval      = 0.096f,
        .maxGroundSpeed    = 215.0f,
        .scopedGroundSpeed = 215.0f,
        .baseInaccStand    = 0.0045f,
        .baseInaccDuck     = 0.0022f,
        .unscopedPenalty   = 0.0f,
        .moveInaccScale    = 0.048f,
        .isAutomatic       = true,
        .hasScope          = false,
        .isGrenade         = false
    },
    // [1] Desert Eagle
    WeaponSpec{
        .name              = "DESERT EAGLE",
        .baseDamage        = 53.0f,
        .armorPenetration  = 0.932f,
        .penetrationPower  = 2.0f,
        .rangeModifier     = 0.81f,
        .fireInterval      = 0.225f,
        .maxGroundSpeed    = 230.0f,
        .scopedGroundSpeed = 230.0f,
        .baseInaccStand    = 0.0035f,
        .baseInaccDuck     = 0.0018f,
        .unscopedPenalty   = 0.0f,
        .moveInaccScale    = 0.055f,
        .isAutomatic       = false,
        .hasScope          = false,
        .isGrenade         = false
    },
    // [2] AWP Sniper Rifle
    WeaponSpec{
        .name              = "AWP SNIPER",
        .baseDamage        = 115.0f,
        .armorPenetration  = 0.975f,
        .penetrationPower  = 2.5f,
        .rangeModifier     = 0.99f,
        .fireInterval      = 1.455f,
        .maxGroundSpeed    = 200.0f,
        .scopedGroundSpeed = 100.0f,
        .baseInaccStand    = 0.0004f,
        .baseInaccDuck     = 0.0002f,
        .unscopedPenalty   = 0.068f,
        .moveInaccScale    = 0.095f,
        .isAutomatic       = false,
        .hasScope          = true,
        .isGrenade         = false
    },
    // [3] HE Grenade
    WeaponSpec{
        .name              = "HE GRENADE",
        .baseDamage        = 98.0f,
        .armorPenetration  = 0.60f,
        .penetrationPower  = 0.0f,
        .rangeModifier     = 1.0f,
        .fireInterval      = 0.65f,
        .maxGroundSpeed    = 245.0f,
        .scopedGroundSpeed = 245.0f,
        .baseInaccStand    = 0.002f,
        .baseInaccDuck     = 0.002f,
        .unscopedPenalty   = 0.0f,
        .moveInaccScale    = 0.0f,
        .isAutomatic       = false,
        .hasScope          = false,
        .isGrenade         = true
    },
    // [4] Flashbang
    WeaponSpec{
        .name              = "FLASHBANG",
        .baseDamage        = 1.0f,
        .armorPenetration  = 1.0f,
        .penetrationPower  = 0.0f,
        .rangeModifier     = 1.0f,
        .fireInterval      = 0.65f,
        .maxGroundSpeed    = 245.0f,
        .scopedGroundSpeed = 245.0f,
        .baseInaccStand    = 0.002f,
        .baseInaccDuck     = 0.002f,
        .unscopedPenalty   = 0.0f,
        .moveInaccScale    = 0.0f,
        .isAutomatic       = false,
        .hasScope          = false,
        .isGrenade         = true
    }
}};

const WeaponSpec& GetWeaponSpec(WeaponID id) {
    const auto idx = std::min<std::size_t>(static_cast<std::size_t>(id), WEAPON_SPECS.size() - 1);
    return WEAPON_SPECS[idx];
}

struct RecoilSample { float pitchKick; float yawKick; };
constexpr std::array<RecoilSample, 30> AK47_RECOIL_TABLE = {{
    {-16.0f,   0.0f}, {-24.0f,  -1.5f}, {-31.0f,   2.0f}, {-34.0f,  -2.5f}, {-35.0f,   3.5f},
    {-28.0f,  14.0f}, {-22.0f,  21.0f}, {-16.0f,  24.0f}, {-10.0f,  22.0f}, { -6.0f,  18.0f},
    { -4.0f,  10.0f}, { -5.0f,  -8.0f}, { -6.0f, -22.0f}, { -5.0f, -28.0f}, { -4.0f, -30.0f},
    { -3.0f, -26.0f}, { -2.0f, -20.0f}, { -4.0f, -12.0f}, { -5.0f,   6.0f}, { -4.0f,  18.0f},
    { -3.0f,  24.0f}, { -2.0f,  22.0f}, { -3.0f,  12.0f}, { -4.0f,  -8.0f}, { -3.0f, -20.0f},
    { -2.0f, -24.0f}, { -2.0f, -18.0f}, { -3.0f,  -6.0f}, { -2.0f,  12.0f}, { -2.0f,  18.0f}
}};

static constexpr uint32_t DeterministicHash32(uint32_t x) {
    x += 0x9e3779b9u;
    x = (x ^ (x >> 16)) * 0x85ebca6bu;
    x = (x ^ (x >> 13)) * 0xc2b2ae35u;
    return x ^ (x >> 16);
}

static constexpr float DeterministicFloat01(uint32_t command_number, uint32_t salt) {
    const uint32_t h = DeterministicHash32(command_number * 747796405u + salt * 2891336453u);
    return static_cast<float>(h & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
}

void AngleVectors(const Vec3& angles, Vec3* forward, Vec3* right, Vec3* up) {
    constexpr float DEG2RAD = std::numbers::pi_v<float> / 180.0f;
    const float pitch = angles.x * DEG2RAD;
    const float yaw   = angles.y * DEG2RAD;
    const float roll  = angles.z * DEG2RAD;

    const float sp = std::sin(pitch), cp = std::cos(pitch);
    const float sy = std::sin(yaw),   cy = std::cos(yaw);
    const float sr = std::sin(roll),  cr = std::cos(roll);

    if (forward) {
        forward->x = cp * cy;
        forward->y = cp * sy;
        forward->z = -sp;
    }
    if (right) {
        right->x = (-1.0f * sr * sp * cy + -1.0f * cr * -sy);
        right->y = (-1.0f * sr * sp * sy + -1.0f * cr * cy);
        right->z = -1.0f * sr * cp;
    }
    if (up) {
        up->x = (cr * sp * cy + -sr * -sy);
        up->y = (cr * sp * sy + -sr * cy);
        up->z = cr * cp;
    }
}

// ============================================================================
// EL BOMBASI FIRLATMA VE BVH SEKME SİMÜLASYONU (CS2 GRENADE PHYSICS)
// ============================================================================
Vec3 ComputeGrenadeThrowVelocity(
    const Vec3& viewangles,
    const Vec3& playerVelocity,
    float throwSpeed
) {
    // CS standardı: Bomba bakış açısından hafif yukarı (+10 derece eğim) fırlatılır
    Vec3 throwAngles = viewangles;
    throwAngles.x = std::clamp(throwAngles.x - 8.0f, -89.0f, 89.0f);

    Vec3 forward, right, up;
    AngleVectors(throwAngles, &forward, &right, &up);

    // Oyuncunun anlık hareket hızının %50'si bombaya aktarılır (Koşarak/Zıplayarak fırlatma)
    return forward * throwSpeed + playerVelocity * 0.5f;
}

bool SimulateGrenadeStep(
    GrenadeProjectile& proj,
    const CollisionWorld& world,
    float gravity,
    float dt,
    Vec3* outBouncePos,
    float* outImpactSpeed
) {
    if (!proj.active) return false;

    proj.fuseTimer -= dt;
    // Source el bombası yerçekimi çarpanı (0.55 * 800 = 440 u/s^2)
    constexpr float GRENADE_GRAVITY_SCALE = 0.55f;
    proj.velocity.z -= gravity * GRENADE_GRAVITY_SCALE * dt;

    const Vec3 nextPos = proj.origin + proj.velocity * dt;
    const TraceResult tr = TraceBox(world, proj.origin, nextPos, GRENADE_MINS, GRENADE_MAXS);

    if (tr.fraction < 1.0f) {
        proj.origin = tr.endPos;

        const float normalSpeed = -proj.velocity.Dot(tr.planeNormal);
        if (outBouncePos)   *outBouncePos   = tr.endPos;
        if (outImpactSpeed) *outImpactSpeed = normalSpeed;

        // CS Esnek Yansıma: overbounce = 1.45 (yüzeye dik hızın %45'i geri seker) + teğetsel sürtünme
        Vec3 reflected{};
        ClipVelocity(proj.velocity, tr.planeNormal, reflected, 1.45f);
        proj.velocity = reflected * 0.72f;

        // Zeminde çok yavaşladıysa yuvarlanmayı durdur
        if (tr.planeNormal.z > 0.7f && proj.velocity.LengthSq() < 400.0f) {
            proj.velocity = {0.0f, 0.0f, 0.0f};
        }
        return normalSpeed > 45.0f;
    }

    proj.origin = nextPos;
    return false;
}

static bool RayIntersectSphere(const Vec3& orig, const Vec3& dir, const Vec3& center, float radius, float& outT) {
    const Vec3 oc = orig - center;
    const float b = oc.Dot(dir);
    const float c = oc.Dot(oc) - radius * radius;
    const float disc = b * b - c;
    if (disc < 0.0f) return false;

    const float s = std::sqrt(disc);
    float t = -b - s;
    if (t < 0.0f) t = -b + s;
    if (t < 0.0f) return false;

    outT = t;
    return true;
}

bool RayIntersectCapsule(
    const Vec3& rayOrigin,
    const Vec3& rayDir,
    const Vec3& capA,
    const Vec3& capB,
    float radius,
    float& outHitDist
) {
    const Vec3  ba   = capB - capA;
    const Vec3  oa   = rayOrigin - capA;
    const float baba = ba.Dot(ba);
    const float bard = ba.Dot(rayDir);
    const float baoa = ba.Dot(oa);
    const float rdoa = rayDir.Dot(oa);
    const float oaoa = oa.Dot(oa);

    float bestT = std::numeric_limits<float>::max();
    bool  hit   = false;

    const float a = baba - bard * bard;
    const float b = baba * rdoa - baoa * bard;
    const float c = baba * oaoa - baoa * baoa - radius * radius * baba;

    if (std::abs(a) > 1e-6f) {
        const float h = b * b - a * c;
        if (h >= 0.0f) {
            const float t = (-b - std::sqrt(h)) / a;
            if (t >= 0.0f) {
                const float y = baoa + t * bard;
                if (y >= 0.0f && y <= baba) {
                    bestT = t;
                    hit   = true;
                }
            }
        }
    }

    float tSphere = 0.0f;
    if (RayIntersectSphere(rayOrigin, rayDir, capA, radius, tSphere) && tSphere < bestT) {
        bestT = tSphere;
        hit   = true;
    }
    if (RayIntersectSphere(rayOrigin, rayDir, capB, radius, tSphere) && tSphere < bestT) {
        bestT = tSphere;
        hit   = true;
    }

    if (hit) {
        outHitDist = bestT;
    }
    return hit;
}

std::array<HitboxVolume, 5> BuildPlayerHitboxes(const Vec3& playerOrigin) {
    std::array<HitboxVolume, 5> rig{};

    rig[0] = HitboxVolume{
        .group      = HitGroup::Head,
        .isCapsule  = true,
        .capA       = playerOrigin + Vec3{0.0f, 0.0f, 58.0f},
        .capB       = playerOrigin + Vec3{0.0f, 0.0f, 64.5f},
        .radius     = 6.2f,
        .boxMins    = playerOrigin + Vec3{-6.5f, -6.5f, 54.0f},
        .boxMaxs    = playerOrigin + Vec3{ 6.5f,  6.5f, 70.5f},
        .debugColor = Vec3{1.0f, 0.85f, 0.15f}
    };

    rig[1] = HitboxVolume{
        .group      = HitGroup::Chest,
        .isCapsule  = false,
        .capA       = playerOrigin + Vec3{0.0f, 0.0f, 40.0f},
        .capB       = playerOrigin + Vec3{0.0f, 0.0f, 52.0f},
        .radius     = 11.5f,
        .boxMins    = playerOrigin + Vec3{-11.0f, -12.5f, 38.0f},
        .boxMaxs    = playerOrigin + Vec3{ 11.0f,  12.5f, 54.0f},
        .debugColor = Vec3{0.15f, 0.85f, 1.0f}
    };

    rig[2] = HitboxVolume{
        .group      = HitGroup::Stomach,
        .isCapsule  = false,
        .capA       = playerOrigin + Vec3{0.0f, 0.0f, 26.0f},
        .capB       = playerOrigin + Vec3{0.0f, 0.0f, 36.0f},
        .radius     = 10.0f,
        .boxMins    = playerOrigin + Vec3{-9.5f, -10.5f, 24.0f},
        .boxMaxs    = playerOrigin + Vec3{ 9.5f,  10.5f, 38.0f},
        .debugColor = Vec3{0.20f, 0.95f, 0.55f}
    };

    rig[3] = HitboxVolume{
        .group      = HitGroup::LeftLeg,
        .isCapsule  = true,
        .capA       = playerOrigin + Vec3{0.0f, -6.5f,  4.5f},
        .capB       = playerOrigin + Vec3{0.0f, -6.5f, 21.5f},
        .radius     = 5.0f,
        .boxMins    = playerOrigin + Vec3{-5.0f, -11.5f,  0.0f},
        .boxMaxs    = playerOrigin + Vec3{ 5.0f,  -1.5f, 24.0f},
        .debugColor = Vec3{0.35f, 0.50f, 1.0f}
    };

    rig[4] = HitboxVolume{
        .group      = HitGroup::RightLeg,
        .isCapsule  = true,
        .capA       = playerOrigin + Vec3{0.0f,  6.5f,  4.5f},
        .capB       = playerOrigin + Vec3{0.0f,  6.5f, 21.5f},
        .radius     = 5.0f,
        .boxMins    = playerOrigin + Vec3{-5.0f,  1.5f,  0.0f},
        .boxMaxs    = playerOrigin + Vec3{ 5.0f, 11.5f, 24.0f},
        .debugColor = Vec3{0.35f, 0.50f, 1.0f}
    };

    return rig;
}

float ScaleDamageByHitGroup(float rawDamage, HitGroup group, float armorPenetration, bool targetHasArmor) {
    float dmg = rawDamage;
    switch (group) {
        case HitGroup::Head:
            dmg *= 4.0f;
            if (targetHasArmor) dmg *= armorPenetration;
            break;
        case HitGroup::Chest:
        case HitGroup::Generic:
            dmg *= 1.0f;
            if (targetHasArmor) dmg *= armorPenetration;
            break;
        case HitGroup::Stomach:
            dmg *= 1.25f;
            if (targetHasArmor) dmg *= armorPenetration;
            break;
        case HitGroup::LeftLeg:
        case HitGroup::RightLeg:
            dmg *= 0.75f;
            break;
    }
    return std::floor(dmg);
}

const char* GetHitGroupName(HitGroup group) {
    switch (group) {
        case HitGroup::Head:     return "HEADSHOT";
        case HitGroup::Chest:    return "CHEST";
        case HitGroup::Stomach:  return "STOMACH";
        case HitGroup::LeftLeg:  return "LEFT LEG";
        case HitGroup::RightLeg: return "RIGHT LEG";
        default:                 return "HE BLAST";
    }
}

bool TraceToExit(
    const CollisionWorld& world,
    const Vec3& enterPos,
    const Vec3& rayDir,
    Vec3& outExitPos,
    Vec3& outExitNormal
) {
    constexpr float STEP_SIZE = 4.0f;
    constexpr float MAX_PENETRATION_DIST = 90.0f;
    constexpr Vec3  ZERO_EXT{0.0f, 0.0f, 0.0f};

    float dist = 0.0f;
    while (dist <= MAX_PENETRATION_DIST) {
        dist += STEP_SIZE;
        const Vec3 testPos = enterPos + rayDir * dist;

        const TraceResult pointCheck = TraceBox(world, testPos, testPos, ZERO_EXT, ZERO_EXT);
        if (!pointCheck.startSolid) {
            const Vec3 prevPos = testPos - rayDir * STEP_SIZE;
            const TraceResult backTrace = TraceBox(world, testPos, prevPos, ZERO_EXT, ZERO_EXT);
            if (backTrace.fraction < 1.0f) {
                outExitPos    = backTrace.endPos;
                outExitNormal = backTrace.planeNormal;
                return true;
            }
            outExitPos    = prevPos;
            outExitNormal = -rayDir;
            return true;
        }
    }
    return false;
}

FireBulletResult FireBulletPenetrating(
    const CollisionWorld& world,
    WeaponID weapon,
    const Vec3& startPos,
    const Vec3& shotDir,
    const Vec3& rewoundTargetOrigin
) {
    FireBulletResult result{};
    result.finalEndPos = startPos;

    const WeaponSpec& spec = GetWeaponSpec(weapon);
    float currentDamage       = spec.baseDamage;
    float totalTraveledDist   = 0.0f;
    constexpr float MAX_RANGE = 4096.0f;
    int remainingPenetrations = 4;

    Vec3 currentRayStart = startPos;
    const auto hitboxes  = BuildPlayerHitboxes(rewoundTargetOrigin);

    while (remainingPenetrations >= 0 && currentDamage >= 1.0f && totalTraveledDist < MAX_RANGE) {
        const float remainingRange = MAX_RANGE - totalTraveledDist;

        const TraceResult worldTrace = TraceRay(world, currentRayStart, shotDir, remainingRange);
        const float worldHitDist = (worldTrace.fraction < 1.0f) ? (worldTrace.fraction * remainingRange) : remainingRange;

        float    bestBotDist  = worldHitDist;
        HitGroup bestHitGroup = HitGroup::Generic;
        bool     hitBotOnSeg  = false;

        for (const HitboxVolume& hb : hitboxes) {
            float d = 0.0f;
            bool intersected = false;
            if (hb.isCapsule) {
                intersected = RayIntersectCapsule(currentRayStart, shotDir, hb.capA, hb.capB, hb.radius, d);
            } else {
                intersected = RayIntersectAABB(currentRayStart, shotDir, hb.boxMins, hb.boxMaxs, d);
            }

            if (intersected && d >= 0.0f && d < bestBotDist) {
                bestBotDist  = d;
                bestHitGroup = hb.group;
                hitBotOnSeg  = true;
            }
        }

        if (hitBotOnSeg) {
            totalTraveledDist += bestBotDist;
            const float distAtten = std::pow(spec.rangeModifier, totalTraveledDist / 500.0f);
            const float rawDmg    = currentDamage * distAtten;

            result.hitPlayer   = true;
            result.hitGroup    = bestHitGroup;
            result.finalDamage = ScaleDamageByHitGroup(rawDmg, bestHitGroup, spec.armorPenetration, true);
            result.finalEndPos = currentRayStart + shotDir * bestBotDist;

            if (result.impactCount < result.impacts.size()) {
                result.impacts[result.impactCount++] = {result.finalEndPos, -shotDir, false};
            }
            return result;
        }

        if (worldTrace.fraction >= 1.0f) {
            result.finalEndPos = currentRayStart + shotDir * remainingRange;
            break;
        }

        totalTraveledDist += worldHitDist;
        result.finalEndPos = worldTrace.endPos;
        if (result.impactCount < result.impacts.size()) {
            result.impacts[result.impactCount++] = {worldTrace.endPos, worldTrace.planeNormal, false};
        }

        if (remainingPenetrations == 0) break;

        Vec3 exitPos{}, exitNormal{};
        if (!TraceToExit(world, worldTrace.endPos, shotDir, exitPos, exitNormal)) {
            break;
        }

        const float thickness = (exitPos - worldTrace.endPos).Length();
        const SurfaceProperties surfProps = CollisionWorld::GetSurfaceProps(worldTrace.hitMaterial);

        const float effectivePenMod  = surfProps.penetrationModifier * (spec.penetrationPower * 0.55f);
        const float thicknessPenalty = (thickness * thickness) / (24.0f * effectivePenMod);
        const float materialPenalty  = currentDamage * (1.0f - surfProps.damageModifier);
        const float lostDamage       = thicknessPenalty + materialPenalty;

        currentDamage -= lostDamage;
        if (currentDamage < 1.0f) {
            break;
        }

        if (result.impactCount < result.impacts.size()) {
            result.impacts[result.impactCount++] = {exitPos, exitNormal, true};
        }

        result.penetratedCount++;
        remainingPenetrations--;
        totalTraveledDist += thickness;
        currentRayStart    = exitPos + shotDir * 0.5f;
    }

    return result;
}

void DecayAimPunchAngle(PlayerState& state, float dt) {
    if (state.aim_punch_angle.LengthSq() > 1e-6f || state.aim_punch_angle_vel.LengthSq() > 1e-6f) {
        state.aim_punch_angle += state.aim_punch_angle_vel * dt;
        const float damping = std::max(0.0f, 1.0f - (9.5f * dt));
        state.aim_punch_angle_vel *= damping;
        state.aim_punch_angle_vel -= state.aim_punch_angle * (68.0f * dt);
        state.aim_punch_angle *= std::exp(-4.2f * dt);
    } else {
        state.aim_punch_angle     = {0.0f, 0.0f, 0.0f};
        state.aim_punch_angle_vel = {0.0f, 0.0f, 0.0f};
    }
}

void UpdateWeaponAndRecoil(PlayerState& state, const UserCmd& cmd, [[maybe_unused]] const MoveVars& vars, float dt) {
    state.fired_this_tick         = false;
    state.threw_grenade_this_tick = false;

    // 1. Silah ve Bomba Yuvası Seçimi (1: AK-47, 2: Deagle, 3: AWP, 4: HE Grenade, 5: Flashbang)
    if (cmd.weapon_select >= 1 && cmd.weapon_select <= 5) {
        const auto requested = static_cast<WeaponID>(cmd.weapon_select - 1);
        if (requested != state.active_weapon) {
            state.active_weapon       = requested;
            state.scope_level         = 0;
            state.recoil_index        = 0.0f;
            state.shots_fired         = 0;
            state.aim_punch_angle     = {0.0f, 0.0f, 0.0f};
            state.aim_punch_angle_vel = {0.0f, 0.0f, 0.0f};
            state.switch_cooldown     = 0.25f;
            state.weapon_cooldown     = 0.25f;
        }
    }

    const WeaponSpec& spec = GetWeaponSpec(state.active_weapon);

    DecayAimPunchAngle(state, dt);

    if (state.switch_cooldown > 0.0f) {
        state.switch_cooldown = std::max(0.0f, state.switch_cooldown - dt);
    }
    if (state.weapon_cooldown > 0.0f) {
        state.weapon_cooldown = std::max(0.0f, state.weapon_cooldown - dt);
    }

    // 2. Hızlı Bomba Tuşları (G = HE Grenade, C = Flashbang)
    const bool quickHePress    = (cmd.buttons & Buttons::QUICK_HE)    && !(state.old_buttons & Buttons::QUICK_HE);
    const bool quickFlashPress = (cmd.buttons & Buttons::QUICK_FLASH) && !(state.old_buttons & Buttons::QUICK_FLASH);

    if ((quickHePress || quickFlashPress) && state.weapon_cooldown <= 0.0f) {
        state.threw_grenade_this_tick = true;
        state.thrown_grenade_type     = quickHePress ? GrenadeType::HE : GrenadeType::Flashbang;
        state.thrown_grenade_speed    = 700.0f;
        state.weapon_cooldown         = 0.55f;
    }

    // 3. Elde Bomba Varken Sol Tık (Uzun: 720), Sağ Tık (Kısa: 280), Sol+Sağ (Orta: 490) Fırlatma
    const bool attackHeld        = (cmd.buttons & Buttons::ATTACK) != 0;
    const bool attack2Held       = (cmd.buttons & Buttons::ATTACK2) != 0;
    const bool attackFreshPress  = attackHeld  && !(state.old_buttons & Buttons::ATTACK);
    const bool attack2FreshPress = attack2Held && !(state.old_buttons & Buttons::ATTACK2);

    if (spec.isGrenade) {
        if ((attackFreshPress || attack2FreshPress) && state.weapon_cooldown <= 0.0f && state.switch_cooldown <= 0.0f) {
            state.threw_grenade_this_tick = true;
            state.thrown_grenade_type     = (state.active_weapon == WeaponID::HEGrenade) ? GrenadeType::HE : GrenadeType::Flashbang;
            if (attackHeld && attack2Held) {
                state.thrown_grenade_speed = 490.0f;
            } else if (attack2Held) {
                state.thrown_grenade_speed = 280.0f;
            } else {
                state.thrown_grenade_speed = 720.0f;
            }
            state.weapon_cooldown = spec.fireInterval;
        }
    } else {
        // AWP Dürbün Kademesi
        if (spec.hasScope && attack2FreshPress && state.switch_cooldown <= 0.0f) {
            state.scope_level = (state.scope_level + 1) % 3;
        }

        // Dinamik Dağılım
        const float speed2D   = state.velocity.Length2D();
        float baseInacc       = (cmd.buttons & Buttons::DUCK) ? spec.baseInaccDuck : spec.baseInaccStand;
        if (spec.hasScope && state.scope_level == 0) {
            baseInacc += spec.unscopedPenalty;
        }
        const float moveInacc = std::clamp((speed2D - 34.0f) / 180.0f, 0.0f, 1.6f) * spec.moveInaccScale;
        const float airInacc  = state.on_ground ? 0.0f : 0.095f;
        const float fireInacc = (state.active_weapon == WeaponID::Deagle) ? (state.recoil_index * 0.018f) : (state.recoil_index * 0.0012f);

        state.inaccuracy = baseInacc + moveInacc + airInacc + fireInacc;

        // Ateşleme
        const bool canPullTrigger = spec.isAutomatic ? attackHeld : attackFreshPress;
        if (canPullTrigger && state.weapon_cooldown <= 0.0f && state.switch_cooldown <= 0.0f) {
            state.fired_this_tick = true;
            state.weapon_cooldown = spec.fireInterval;

            if (state.active_weapon == WeaponID::AK47) {
                const std::size_t patIdx = std::min<std::size_t>(
                    static_cast<std::size_t>(state.recoil_index),
                    AK47_RECOIL_TABLE.size() - 1
                );
                state.aim_punch_angle_vel.x += AK47_RECOIL_TABLE[patIdx].pitchKick;
                state.aim_punch_angle_vel.y += AK47_RECOIL_TABLE[patIdx].yawKick;
                state.recoil_index = std::min(30.0f, state.recoil_index + 1.0f);
            } else if (state.active_weapon == WeaponID::Deagle) {
                state.aim_punch_angle_vel.x -= 48.0f;
                state.aim_punch_angle_vel.y += (state.shots_fired % 2 == 0) ? 8.5f : -8.5f;
                state.recoil_index = std::min(6.0f, state.recoil_index + 1.8f);
            } else if (state.active_weapon == WeaponID::AWP) {
                state.aim_punch_angle_vel.x -= 62.0f;
                state.recoil_index = 2.0f;
            }
            state.shots_fired++;
        } else if (!attackHeld && state.weapon_cooldown <= 0.0f) {
            const float recoveryRate = (state.active_weapon == WeaponID::Deagle) ? 8.5f : 55.0f;
            state.recoil_index = std::max(0.0f, state.recoil_index - (dt * recoveryRate));
            if (state.recoil_index == 0.0f) {
                state.shots_fired = 0;
            }
        }
    }

    constexpr uint32_t WEAPON_BTN_MASK = Buttons::ATTACK | Buttons::ATTACK2 | Buttons::QUICK_HE | Buttons::QUICK_FLASH;
    state.old_buttons = (state.old_buttons & ~WEAPON_BTN_MASK) | (cmd.buttons & WEAPON_BTN_MASK);
}

Vec3 ComputeShotDirection(
    const Vec3& viewangles,
    const Vec3& aim_punch_angle,
    float inaccuracy,
    uint32_t command_number,
    float recoil_scale
) {
    Vec3 trueShotAngles = viewangles + aim_punch_angle * recoil_scale;
    trueShotAngles.x = std::clamp(trueShotAngles.x, -89.0f, 89.0f);

    Vec3 forward, right, up;
    AngleVectors(trueShotAngles, &forward, &right, &up);

    constexpr float TWO_PI = 2.0f * std::numbers::pi_v<float>;
    const float r1     = DeterministicFloat01(command_number, 1u);
    const float theta1 = DeterministicFloat01(command_number, 2u) * TWO_PI;
    const float r2     = DeterministicFloat01(command_number, 3u);
    const float theta2 = DeterministicFloat01(command_number, 4u) * TWO_PI;

    const float spreadRadius = (r1 + r2) * 0.5f * inaccuracy;
    const float spreadX      = (std::cos(theta1) + std::cos(theta2)) * 0.5f * spreadRadius;
    const float spreadY      = (std::sin(theta1) + std::sin(theta2)) * 0.5f * spreadRadius;

    Vec3 finalDir = forward + (right * spreadX) + (up * spreadY);
    finalDir.NormalizeInPlace();
    return finalDir;
}

static void ClipBoxToBrush(
    const Brush& brush,
    int brushIndex,
    const Vec3& startCenter,
    const Vec3& endCenter,
    const Vec3& extents,
    TraceResult& ioTrace
) {
    if (brush.planes.empty()) return;

    float enterFrac = -1.0f;
    float leaveFrac = 1.0f;
    Vec3  bestNormal{0.0f, 0.0f, 1.0f};
    bool  startOut = false;
    bool  getOut   = false;

    for (const Plane& plane : brush.planes) {
        const float offset =
            std::abs(plane.normal.x) * extents.x +
            std::abs(plane.normal.y) * extents.y +
            std::abs(plane.normal.z) * extents.z;

        const float dist = plane.dist + offset;
        const float d0   = startCenter.Dot(plane.normal) - dist;
        const float d1   = endCenter.Dot(plane.normal)   - dist;

        if (d0 > 0.0f) startOut = true;
        if (d1 > 0.0f) getOut   = true;

        if (d0 > 0.0f && d1 >= d0) return;
        if (d0 <= 0.0f && d1 <= 0.0f) continue;

        if (d0 > d1) {
            float f = (d0 - DIST_EPSILON) / (d0 - d1);
            if (f < 0.0f) f = 0.0f;
            if (f > enterFrac) {
                enterFrac  = f;
                bestNormal = plane.normal;
            }
        } else {
            float f = (d0 + DIST_EPSILON) / (d0 - d1);
            if (f > 1.0f) f = 1.0f;
            if (f < leaveFrac) {
                leaveFrac = f;
            }
        }
    }

    if (!startOut) {
        ioTrace.startSolid  = true;
        ioTrace.hitMaterial = brush.material;
        if (!getOut) {
            ioTrace.allSolid      = true;
            ioTrace.fraction      = 0.0f;
            ioTrace.hitBrushIndex = brushIndex;
        }
        return;
    }

    if (enterFrac < leaveFrac && enterFrac > -1.0f && enterFrac < ioTrace.fraction) {
        if (enterFrac < 0.0f) enterFrac = 0.0f;
        ioTrace.fraction      = enterFrac;
        ioTrace.planeNormal   = bestNormal;
        ioTrace.hitBrushIndex = brushIndex;
        ioTrace.hitMaterial   = brush.material;
    }
}

static bool AABBOverlaps(const Vec3& minA, const Vec3& maxA, const Vec3& minB, const Vec3& maxB) {
    return (minA.x <= maxB.x && maxA.x >= minB.x) &&
           (minA.y <= maxB.y && maxA.y >= minB.y) &&
           (minA.z <= maxB.z && maxA.z >= minB.z);
}

TraceResult TraceBox(
    const CollisionWorld& world,
    const Vec3& start,
    const Vec3& end,
    const Vec3& mins,
    const Vec3& maxs
) {
    TraceResult trace{};
    trace.fraction = 1.0f;
    trace.endPos   = end;

    const Vec3 centerOffset = (mins + maxs) * 0.5f;
    const Vec3 extents      = (maxs - mins) * 0.5f;
    const Vec3 startCenter  = start + centerOffset;
    const Vec3 endCenter    = end   + centerOffset;

    const Vec3 sweepMins = Vec3::Min(startCenter, endCenter) - extents - Vec3{1.0f, 1.0f, 1.0f};
    const Vec3 sweepMaxs = Vec3::Max(startCenter, endCenter) + extents + Vec3{1.0f, 1.0f, 1.0f};

    world.lastTraceBrushTests = 0;

    if (world.bvhNodes.empty()) {
        for (std::size_t i = 0; i < world.brushes.size(); ++i) {
            world.lastTraceBrushTests++;
            ClipBoxToBrush(world.brushes[i], static_cast<int>(i), startCenter, endCenter, extents, trace);
        }
    } else {
        uint32_t stack[64];
        uint32_t stackPtr = 0;
        stack[stackPtr++] = 0;

        while (stackPtr > 0) {
            const uint32_t nodeIdx = stack[--stackPtr];
            const BVHNode& node    = world.bvhNodes[nodeIdx];

            if (!AABBOverlaps(sweepMins, sweepMaxs, node.mins, node.maxs)) {
                continue;
            }

            if (node.IsLeaf()) {
                for (uint32_t i = 0; i < node.brushCount; ++i) {
                    const uint32_t bIdx = world.brushIndices[node.leftFirst + i];
                    const Brush& b      = world.brushes[bIdx];
                    if (!AABBOverlaps(sweepMins, sweepMaxs, b.mins, b.maxs)) continue;

                    world.lastTraceBrushTests++;
                    ClipBoxToBrush(b, static_cast<int>(bIdx), startCenter, endCenter, extents, trace);
                    if (trace.allSolid) {
                        trace.fraction = 0.0f;
                        trace.endPos   = start;
                        return trace;
                    }
                }
            } else {
                stack[stackPtr++] = node.leftFirst;
                stack[stackPtr++] = node.leftFirst + 1;
            }
        }
    }

    trace.endPos = Vec3::Lerp(start, end, trace.fraction);
    return trace;
}

TraceResult TraceRay(
    const CollisionWorld& world,
    const Vec3& start,
    const Vec3& dir,
    float maxDist
) {
    const Vec3 end = start + dir * maxDist;
    return TraceBox(world, start, end, Vec3{0.0f, 0.0f, 0.0f}, Vec3{0.0f, 0.0f, 0.0f});
}

bool RayIntersectAABB(
    const Vec3& rayOrigin,
    const Vec3& rayDir,
    const Vec3& boxMins,
    const Vec3& boxMaxs,
    float& outHitDist
) {
    float tmin = 0.0f;
    float tmax = std::numeric_limits<float>::max();

    const float orig[3] = { rayOrigin.x, rayOrigin.y, rayOrigin.z };
    const float dir[3]  = { rayDir.x,    rayDir.y,    rayDir.z    };
    const float bmin[3] = { boxMins.x,   boxMins.y,   boxMins.z   };
    const float bmax[3] = { boxMaxs.x,   boxMaxs.y,   boxMaxs.z   };

    for (int i = 0; i < 3; ++i) {
        if (std::abs(dir[i]) < 1e-6f) {
            if (orig[i] < bmin[i] || orig[i] > bmax[i]) return false;
        } else {
            const float invD = 1.0f / dir[i];
            float t1 = (bmin[i] - orig[i]) * invD;
            float t2 = (bmax[i] - orig[i]) * invD;
            if (t1 > t2) std::swap(t1, t2);
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            if (tmin > tmax) return false;
        }
    }

    outHitDist = tmin;
    return true;
}

void ClipVelocity(const Vec3& in, const Vec3& normal, Vec3& out, float overbounce) {
    const float backoff = in.Dot(normal) * overbounce;
    out = in - normal * backoff;

    const float adjust = out.Dot(normal);
    if (adjust < 0.0f) {
        out -= normal * adjust;
    }
}

void TryPlayerMove(PlayerState& state, const CollisionWorld& world, float dt) {
    constexpr int MAX_BUMPS       = 4;
    constexpr int MAX_CLIP_PLANES = 5;

    float time_left = dt;
    const Vec3 primal_velocity = state.velocity;
    Vec3 planes[MAX_CLIP_PLANES];
    int numplanes = 0;

    for (int bumpcount = 0; bumpcount < MAX_BUMPS; ++bumpcount) {
        if (state.velocity.LengthSq() < 1e-6f) break;

        const Vec3 end = state.origin + state.velocity * time_left;
        const TraceResult trace = TraceBox(world, state.origin, end, PLAYER_MINS, PLAYER_MAXS);

        if (trace.allSolid) {
            state.velocity = {0.0f, 0.0f, 0.0f};
            return;
        }

        if (trace.fraction > 0.0f) {
            state.origin = trace.endPos;
        }

        if (trace.fraction == 1.0f) break;

        time_left -= time_left * trace.fraction;

        if (numplanes >= MAX_CLIP_PLANES) {
            state.velocity = {0.0f, 0.0f, 0.0f};
            break;
        }

        planes[numplanes++] = trace.planeNormal;

        Vec3 new_velocity{};
        int i = 0;
        for (; i < numplanes; ++i) {
            ClipVelocity(state.velocity, planes[i], new_velocity, 1.0f);
            int j = 0;
            for (; j < numplanes; ++j) {
                if (j != i && new_velocity.Dot(planes[j]) < 0.0f) break;
            }
            if (j == numplanes) break;
        }

        if (i != numplanes) {
            state.velocity = new_velocity;
        } else {
            if (numplanes != 2) {
                state.velocity = {0.0f, 0.0f, 0.0f};
                break;
            }
            Vec3 dir = planes[0].Cross(planes[1]);
            dir.NormalizeInPlace();
            const float d = dir.Dot(state.velocity);
            state.velocity = dir * d;
        }

        if (state.velocity.Dot(primal_velocity) <= 0.0f) {
            state.velocity = {0.0f, 0.0f, 0.0f};
            break;
        }
    }
}

void StepSlideMove(PlayerState& state, const CollisionWorld& world, const MoveVars& vars, float dt) {
    const Vec3 start_o = state.origin;
    const Vec3 start_v = state.velocity;

    TryPlayerMove(state, world, dt);

    const Vec3 down_o = state.origin;
    const Vec3 down_v = state.velocity;

    state.origin   = start_o;
    state.velocity = start_v;

    const Vec3 stepUpTarget = start_o + Vec3{0.0f, 0.0f, vars.stepsize};
    const TraceResult traceUp = TraceBox(world, start_o, stepUpTarget, PLAYER_MINS, PLAYER_MAXS);
    if (!traceUp.allSolid) {
        state.origin = traceUp.endPos;
    }

    TryPlayerMove(state, world, dt);

    const Vec3 stepDownTarget = state.origin - Vec3{0.0f, 0.0f, vars.stepsize};
    const TraceResult traceDown = TraceBox(world, state.origin, stepDownTarget, PLAYER_MINS, PLAYER_MAXS);
    if (!traceDown.allSolid) {
        state.origin = traceDown.endPos;
    }

    if (traceDown.fraction < 1.0f && traceDown.planeNormal.z < vars.walkable_normal_z) {
        state.origin   = down_o;
        state.velocity = down_v;
        return;
    }

    const float downDistSq = (down_o.x - start_o.x) * (down_o.x - start_o.x) + (down_o.y - start_o.y) * (down_o.y - start_o.y);
    const float upDistSq   = (state.origin.x - start_o.x) * (state.origin.x - start_o.x) + (state.origin.y - start_o.y) * (state.origin.y - start_o.y);

    if (downDistSq >= upDistSq) {
        state.origin   = down_o;
        state.velocity = down_v;
    } else {
        state.velocity.z = down_v.z;
    }
}

void CategorizePosition(PlayerState& state, const CollisionWorld& world, const MoveVars& vars) {
    if (state.velocity.z > 140.0f) {
        state.on_ground = false;
        return;
    }

    const Vec3 point = state.origin - Vec3{0.0f, 0.0f, 2.0f};
    const TraceResult tr = TraceBox(world, state.origin, point, PLAYER_MINS, PLAYER_MAXS);

    if (tr.fraction < 1.0f && tr.planeNormal.z >= vars.walkable_normal_z) {
        state.origin          = tr.endPos;
        state.on_ground       = true;
        state.ground_normal   = tr.planeNormal;
        state.ground_material = tr.hitMaterial; // Ayak sesi sentezleyicisi için zemin materyalini kaydet!
    } else {
        state.on_ground = false;
    }
}

void CheckJumpButton(PlayerState& state, const UserCmd& cmd, const MoveVars& vars) {
    if (!state.on_ground) {
        state.old_buttons |= (cmd.buttons & Buttons::JUMP);
        return;
    }
    if (!(cmd.buttons & Buttons::JUMP)) {
        state.old_buttons &= ~Buttons::JUMP;
        return;
    }
    if (!vars.autobhop && (state.old_buttons & Buttons::JUMP)) {
        return;
    }

    state.on_ground  = false;
    state.velocity.z = vars.jump_impulse;
    state.old_buttons |= Buttons::JUMP;
}

void ApplyFriction(PlayerState& state, const MoveVars& vars, float dt) {
    const float speed = state.velocity.Length2D();
    if (speed < 0.1f) {
        state.velocity.x = 0.0f;
        state.velocity.y = 0.0f;
        return;
    }

    const float control = (speed < vars.stopspeed) ? vars.stopspeed : speed;
    const float drop    = control * vars.friction * dt;

    float newspeed = speed - drop;
    if (newspeed < 0.0f) newspeed = 0.0f;

    if (newspeed != speed) {
        newspeed /= speed;
        state.velocity.x *= newspeed;
        state.velocity.y *= newspeed;
    }
}

void Accelerate(PlayerState& state, const Vec3& wishdir, float wishspeed, float accel, float dt) {
    const float currentspeed = state.velocity.Dot(wishdir);
    const float addspeed     = wishspeed - currentspeed;
    if (addspeed <= 0.0f) return;

    float accelspeed = accel * dt * wishspeed;
    if (accelspeed > addspeed) accelspeed = addspeed;

    state.velocity += wishdir * accelspeed;
}

void AirAccelerate(PlayerState& state, const Vec3& wishdir, float wishspeed, float accel, float air_cap, float dt) {
    float wishspd = wishspeed;
    if (wishspd > air_cap) wishspd = air_cap;

    const float currentspeed = state.velocity.Dot(wishdir);
    const float addspeed     = wishspd - currentspeed;
    if (addspeed <= 0.0f) return;

    float accelspeed = accel * wishspeed * dt;
    if (accelspeed > addspeed) accelspeed = addspeed;

    state.velocity += wishdir * accelspeed;
}

static void ComputeWishMove(const PlayerState& state, const UserCmd& cmd, float maxAllowedSpeed, Vec3& out_wishdir, float& out_wishspeed) {
    Vec3 forward, right, up;
    AngleVectors(state.viewangles, &forward, &right, &up);

    forward.z = 0.0f;
    right.z   = 0.0f;
    forward.NormalizeInPlace();
    right.NormalizeInPlace();

    Vec3 wishvel = forward * cmd.forwardmove + right * cmd.sidemove;
    wishvel.z = 0.0f;

    out_wishdir   = wishvel;
    out_wishspeed = out_wishdir.NormalizeInPlace();

    if (out_wishspeed > maxAllowedSpeed) {
        out_wishspeed = maxAllowedSpeed;
    }
}

void WalkMove(PlayerState& state, const UserCmd& cmd, const CollisionWorld& world, const MoveVars& vars, float dt) {
    const WeaponSpec& spec = GetWeaponSpec(state.active_weapon);
    float weaponMaxSpeed = (state.scope_level > 0) ? spec.scopedGroundSpeed : spec.maxGroundSpeed;
    if (cmd.buttons & Buttons::DUCK) {
        weaponMaxSpeed *= 0.34f;
    }

    Vec3 wishdir;
    float wishspeed = 0.0f;
    ComputeWishMove(state, cmd, weaponMaxSpeed, wishdir, wishspeed);

    state.velocity.z = 0.0f;
    Accelerate(state, wishdir, wishspeed, vars.accelerate, dt);
    state.velocity.z = 0.0f;

    if (state.velocity.LengthSq() > 1e-4f) {
        StepSlideMove(state, world, vars, dt);
    }
}

void AirMove(PlayerState& state, const UserCmd& cmd, const CollisionWorld& world, const MoveVars& vars, float dt) {
    Vec3 wishdir;
    float wishspeed = 0.0f;
    ComputeWishMove(state, cmd, vars.maxspeed, wishdir, wishspeed);

    AirAccelerate(state, wishdir, wishspeed, vars.airaccelerate, vars.air_wishspeed_cap, dt);
    TryPlayerMove(state, world, dt);
}

PlayerState SimulateTick(
    const PlayerState& current_state,
    const UserCmd& cmd,
    const CollisionWorld& world,
    const MoveVars& vars,
    float dt
) {
    PlayerState next = current_state;
    next.tick_count    = cmd.tick_count;
    next.viewangles    = cmd.viewangles;
    next.view_offset_z = (cmd.buttons & Buttons::DUCK) ? 46.0f : 64.0f;

    UpdateWeaponAndRecoil(next, cmd, vars, dt);

    CategorizePosition(next, world, vars);
    CheckJumpButton(next, cmd, vars);

    if (!next.on_ground) {
        next.velocity.z -= 0.5f * vars.gravity * dt;
    }

    if (next.on_ground) {
        ApplyFriction(next, vars, dt);
        WalkMove(next, cmd, world, vars, dt);
    } else {
        AirMove(next, cmd, world, vars, dt);
    }

    CategorizePosition(next, world, vars);

    if (!next.on_ground) {
        next.velocity.z -= 0.5f * vars.gravity * dt;
    } else {
        next.velocity.z = 0.0f;
    }

    return next;
}

PlayerState InterpolateState(const PlayerState& prev, const PlayerState& curr, float alpha) {
    alpha = std::clamp(alpha, 0.0f, 1.0f);
    PlayerState interp = curr;
    interp.origin          = Vec3::Lerp(prev.origin, curr.origin, alpha);
    interp.velocity        = Vec3::Lerp(prev.velocity, curr.velocity, alpha);
    interp.view_offset_z   = prev.view_offset_z + (curr.view_offset_z - prev.view_offset_z) * alpha;
    interp.aim_punch_angle = Vec3::Lerp(prev.aim_punch_angle, curr.aim_punch_angle, alpha);
    interp.inaccuracy      = prev.inaccuracy + (curr.inaccuracy - prev.inaccuracy) * alpha;
    interp.weapon_cooldown = prev.weapon_cooldown + (curr.weapon_cooldown - prev.weapon_cooldown) * alpha;
    interp.switch_cooldown = prev.switch_cooldown + (curr.switch_cooldown - prev.switch_cooldown) * alpha;
    return interp;
}

} // namespace Movement