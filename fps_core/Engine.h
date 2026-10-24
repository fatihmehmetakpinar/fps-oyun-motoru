#pragma once

#include "Movement.h"
#include <SDL2/SDL.h>
#include <array>
#include <deque>
#include <vector>

#pragma pack(push, 1)
enum class PacketType : uint8_t {
    ClientUserCmd  = 1,
    ServerSnapshot = 2
};

struct NetCmdPacket {
    PacketType type{PacketType::ClientUserCmd};
    uint32_t   command_number{0};
    uint32_t   client_tick{0};
    float      subtick_fraction{0.0f};
    float      client_interp_time{0.0f};
    Vec3       viewangles{};
    float      forwardmove{0.0f};
    float      sidemove{0.0f};
    uint32_t   buttons{0};
    uint8_t    weapon_select{0};
};

struct NetSnapshotPacket {
    PacketType  type{PacketType::ServerSnapshot};
    uint32_t    ack_command_number{0};
    uint32_t    server_tick{0};
    PlayerState authoritative_state{};
    Vec3        bot_server_origin{};
    bool        shot_fired{false};
    bool        hit_confirmed{false};
    HitGroup    hit_group{HitGroup::Generic};
    float       damage_dealt{0.0f};
    uint8_t     penetrated_walls{0};
    Vec3        rewound_bot_origin{};
    Vec3        final_end_pos{};
    uint8_t     impact_count{0};
    std::array<PenetrationImpact, 8> impacts{};
};
#pragma pack(pop)

struct GLVertex {
    Vec3 pos{};
    Vec3 color{};
};

struct BulletDecal {
    Vec3 pos{};
    Vec3 normal{};
    bool hitBot{false};
    bool isExitPoint{false};
    bool active{false};
};

// ============================================================================
// PROSEDÜREL 3B KONUMSAL SES SENTEZLEYİCİ VERİ YAPILARI (SDL_AudioStream)
// ============================================================================
enum class SoundType : uint8_t {
    FootstepConcrete = 0,
    FootstepWood     = 1,
    FootstepMetal    = 2,
    FireAK47         = 3,
    FireDeagle       = 4,
    FireAWP          = 5,
    HeadshotDink     = 6, // 2840 Hz + 4120 Hz Kask Çınlaması
    HitFlesh         = 7, // Kevlar / Gövde Tok Vuruş Sesi
    GrenadeBounce    = 8, // Metalik Sekme Sesi
    ExplosionHE      = 9, // Derin Bas HE Patlama Sesi
    FlashbangBang    = 10 // Keskin Flashbang Patlaması
};

struct SoundVoice {
    SoundType type{SoundType::FootstepConcrete};
    Vec3      origin{};
    float     time{0.0f};
    float     duration{0.2f};
    float     volume{1.0f};
    float     lowPassStateL{0.0f}; // BVH Duvar Sönümlemesi (Occlusion) Filtre Durumu
    float     lowPassStateR{0.0f};
    bool      is2D{false};         // true = Kendi silahımız/ayak sesimiz, false = 3B Uzamsal
    bool      occluded{false};     // Kaynak ile kulak arasında duvar var mı?
    bool      active{false};
};

struct ExplosionVisual {
    Vec3        origin{};
    GrenadeType type{GrenadeType::HE};
    float       timer{0.0f};
    float       maxDuration{0.45f};
    bool        active{false};
};

class Engine {
public:
    static constexpr double      TICK_RATE           = 128.0;
    static constexpr double      TICK_INTERVAL       = 1.0 / TICK_RATE;
    static constexpr uint32_t    HISTORY_BUFFER_SIZE = 256;
    static constexpr uint32_t    HISTORY_MASK        = HISTORY_BUFFER_SIZE - 1;
    static constexpr uint16_t    SERVER_UDP_PORT     = 27015;
    static constexpr std::size_t MAX_DECALS          = 96;
    static constexpr std::size_t MAX_VOICES          = 32;
    static constexpr std::size_t MAX_GRENADES        = 16;
    static constexpr int         AUDIO_SAMPLE_RATE   = 48000;

    Engine() = default;
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    bool Init(const char* title, int width, int height);
    void Run();
    void Shutdown();

    void ReconcileClientPrediction(const PlayerState& authoritative_server_state);
    [[nodiscard]] bool RewindBotLagRecord(float target_simulation_time, Vec3& out_rewound_origin) const;

private:
    bool InitUDPSockets();
    void CloseUDPSockets();
    bool InitAudioStream();
    void CloseAudioStream();
    bool InitGL33Pipeline();
    void BuildStaticWorldMesh();

    // 3B Konumsal Ses Sentezleyici & Mikser
    void EmitSound(SoundType type, const Vec3& origin, float volume, bool is2D = false);
    float SynthesizeVoiceSample(const SoundVoice& v, uint32_t& rngState) const;
    void PumpPositionalAudio(const PlayerState& listenerState);

    // El Bombası Fırlatma, Sekme ve Patlama (HE & Flashbang) Çözücüsü
    void SpawnGrenade(GrenadeType type, const Vec3& origin, const Vec3& velocity);
    void TickGrenades(float dt);
    void DetonateGrenade(const GrenadeProjectile& g);

    void PollEvents(double current_subtick_fraction);
    UserCmd BuildUserCmd(double subtick_fraction);
    void TickUpdate(double subtick_fraction);

    void ProcessNetworkQueues(double current_time);
    void ServerProcessCmdPacket(const NetCmdPacket& pkt);
    void AddBulletDecal(const Vec3& pos, const Vec3& normal, bool hitBot, bool isExitPoint);

    void Render(float interp_alpha, double frame_dt);
    void RenderViewmodel(const PlayerState& renderState, double frame_dt);
    void RenderHUD(const PlayerState& renderState);
    void UpdateWindowTelemetry(double frame_dt);

private:
    SDL_Window*   m_window{nullptr};
    SDL_GLContext m_glContext{nullptr};
    int           m_width{1280};
    int           m_height{960};
    bool          m_running{false};

    // SDL_AudioStream & Ses Cihazı Durumu
    SDL_AudioDeviceID m_audioDevice{0};
    SDL_AudioStream*  m_audioStream{nullptr};
    SDL_AudioSpec     m_deviceSpec{};
    std::array<SoundVoice, MAX_VOICES> m_voices{};
    uint32_t          m_audioRng{0x12345678u};
    float             m_tinnitusPhase{0.0f};
    float             m_playerStepDistAccum{0.0f};
    float             m_botStepDistAccum{0.0f};

    uint32_t m_shaderProgram{0};
    int32_t  m_uMvpLocation{-1};
    uint32_t m_worldVao{0}, m_worldVbo{0};
    uint32_t m_worldTriCount{0}, m_worldLineCount{0};
    uint32_t m_dynVao{0}, m_dynVbo{0};

    Vec3  m_clientViewAngles{0.0f, 0.0f, 0.0f};
    float m_sensitivity{1.25f};
    float m_mYaw{0.022f};
    float m_mPitch{0.022f};

    float   m_pendingJumpSubtick{0.0f};
    bool    m_jumpPressedThisTick{false};
    float   m_pendingAttackSubtick{0.0f};
    bool    m_attackPressedThisTick{false};
    bool    m_attack2PressedThisTick{false};
    bool    m_quickHePressed{false};
    bool    m_quickFlashPressed{false};
    uint8_t m_pendingWeaponSelect{0};
    bool    m_showBvhDebug{false};
    bool    m_xrayHitboxes{true};

    double m_bobTime{0.0};
    double m_currentFps{0.0};

    uint32_t       m_currentTick{0};
    uint32_t       m_commandNumber{0};
    CollisionWorld m_world{};
    MoveVars       m_moveVars{};
    PlayerState    m_prevState{};
    PlayerState    m_currState{};
    PlayerState    m_serverAuthState{};

    // Bot Hareket, Körlük (Flashbang) ve Lag Compensation Durumu
    Vec3  m_botServerOrigin{420.0f, 0.0f, 0.0f};
    float m_botPhaseTime{0.0f};
    float m_botBlindTimer{0.0f};
    std::array<LagRecord, HISTORY_BUFFER_SIZE> m_botLagRing{};

    // Aktif El Bombaları, Patlama Efektleri ve Oyuncu Flashbang Körlüğü
    std::array<GrenadeProjectile, MAX_GRENADES> m_grenades{};
    std::array<ExplosionVisual, 8>              m_explosions{};
    float m_flashAlpha{0.0f};        // [0.0, 1.5] Beyaz ekran körlük yoğunluğu
    float m_tinnitusTimer{0.0f};     // Flashbang kulak çınlama süresi

    struct PredictionSlot {
        UserCmd     cmd{};
        PlayerState predicted_state{};
    };
    std::array<PredictionSlot, HISTORY_BUFFER_SIZE> m_predictionRing{};

    uintptr_t m_serverSock{~uintptr_t(0)};
    uintptr_t m_clientSock{~uintptr_t(0)};
    float     m_simulatedRttMs{100.0f};

    float    m_botHealth{100.0f};
    uint32_t m_botKills{0};
    uint32_t m_totalShots{0};
    uint32_t m_totalHits{0};
    HitGroup m_lastHitGroup{HitGroup::Generic};
    float    m_lastDamageDealt{0.0f};
    uint8_t  m_lastPenetratedWalls{0};

    struct DelayedCmd {
        double       deliver_time{0.0};
        NetCmdPacket pkt{};
    };
    std::deque<DelayedCmd> m_inFlightToServer{};

    float m_tracerTimer{0.0f};
    Vec3  m_lastTraceStart{};
    Vec3  m_lastTraceEnd{};
    Vec3  m_lastRewoundBoxOrigin{};
    bool  m_lastShotHit{false};

    std::array<BulletDecal, MAX_DECALS> m_decals{};
    std::size_t                         m_decalCursor{0};
};