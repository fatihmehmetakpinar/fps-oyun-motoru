#include "Engine.h"

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    using SockLenT = int;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <unistd.h>
    using SockLenT = socklen_t;
#endif

#include <SDL2/SDL_opengl.h>
#include <cstdio>
#include <cstring>
#include <cmath>

namespace GL33 {
    using PFNGLCREATESHADERPROC            = GLuint (APIENTRY*)(GLenum);
    using PFNGLSHADERSOURCEPROC            = void   (APIENTRY*)(GLuint, GLsizei, const GLchar* const*, const GLint*);
    using PFNGLCOMPILESHADERPROC           = void   (APIENTRY*)(GLuint);
    using PFNGLGETSHADERIVPROC             = void   (APIENTRY*)(GLuint, GLenum, GLint*);
    using PFNGLGETSHADERINFOLOGPROC        = void   (APIENTRY*)(GLuint, GLsizei, GLsizei*, GLchar*);
    using PFNGLCREATEPROGRAMPROC           = GLuint (APIENTRY*)();
    using PFNGLATTACHSHADERPROC            = void   (APIENTRY*)(GLuint, GLuint);
    using PFNGLLINKPROGRAMPROC             = void   (APIENTRY*)(GLuint);
    using PFNGLGETPROGRAMIVPROC            = void   (APIENTRY*)(GLuint, GLenum, GLint*);
    using PFNGLUSEPROGRAMPROC              = void   (APIENTRY*)(GLuint);
    using PFNGLDELETESHADERPROC            = void   (APIENTRY*)(GLuint);
    using PFNGLDELETEPROGRAMPROC           = void   (APIENTRY*)(GLuint);
    using PFNGLGETUNIFORMLOCATIONPROC      = GLint  (APIENTRY*)(GLuint, const GLchar*);
    using PFNGLUNIFORMMATRIX4FVPROC        = void   (APIENTRY*)(GLint, GLsizei, GLboolean, const GLfloat*);
    using PFNGLGENVERTEXARRAYSPROC         = void   (APIENTRY*)(GLsizei, GLuint*);
    using PFNGLBINDVERTEXARRAYPROC         = void   (APIENTRY*)(GLuint);
    using PFNGLDELETEVERTEXARRAYSPROC      = void   (APIENTRY*)(GLsizei, const GLuint*);
    using PFNGLGENBUFFERSPROC              = void   (APIENTRY*)(GLsizei, GLuint*);
    using PFNGLBINDBUFFERPROC              = void   (APIENTRY*)(GLenum, GLuint);
    using PFNGLBUFFERDATAPROC              = void   (APIENTRY*)(GLenum, GLsizeiptr, const void*, GLenum);
    using PFNGLDELETEBUFFERSPROC           = void   (APIENTRY*)(GLsizei, const GLuint*);
    using PFNGLENABLEVERTEXATTRIBARRAYPROC = void   (APIENTRY*)(GLuint);
    using PFNGLVERTEXATTRIBPOINTERPROC     = void   (APIENTRY*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);

    static PFNGLCREATESHADERPROC            CreateShader            = nullptr;
    static PFNGLSHADERSOURCEPROC            ShaderSource            = nullptr;
    static PFNGLCOMPILESHADERPROC           CompileShader           = nullptr;
    static PFNGLGETSHADERIVPROC             GetShaderiv             = nullptr;
    static PFNGLGETSHADERINFOLOGPROC        GetShaderInfoLog        = nullptr;
    static PFNGLCREATEPROGRAMPROC           CreateProgram           = nullptr;
    static PFNGLATTACHSHADERPROC            AttachShader            = nullptr;
    static PFNGLLINKPROGRAMPROC             LinkProgram             = nullptr;
    static PFNGLGETPROGRAMIVPROC            GetProgramiv            = nullptr;
    static PFNGLUSEPROGRAMPROC              UseProgram              = nullptr;
    static PFNGLDELETESHADERPROC            DeleteShader            = nullptr;
    static PFNGLDELETEPROGRAMPROC           DeleteProgram           = nullptr;
    static PFNGLGETUNIFORMLOCATIONPROC      GetUniformLocation      = nullptr;
    static PFNGLUNIFORMMATRIX4FVPROC        UniformMatrix4fv        = nullptr;
    static PFNGLGENVERTEXARRAYSPROC         GenVertexArrays         = nullptr;
    static PFNGLBINDVERTEXARRAYPROC         BindVertexArray         = nullptr;
    static PFNGLDELETEVERTEXARRAYSPROC      DeleteVertexArrays      = nullptr;
    static PFNGLGENBUFFERSPROC              GenBuffers              = nullptr;
    static PFNGLBINDBUFFERPROC              BindBuffer              = nullptr;
    static PFNGLBUFFERDATAPROC              BufferData              = nullptr;
    static PFNGLDELETEBUFFERSPROC           DeleteBuffers           = nullptr;
    static PFNGLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray = nullptr;
    static PFNGLVERTEXATTRIBPOINTERPROC     VertexAttribPointer     = nullptr;

    template <typename T>
    static bool LoadProc(T& outProc, const char* name) {
        outProc = reinterpret_cast<T>(SDL_GL_GetProcAddress(name));
        return outProc != nullptr;
    }

    static bool LoadAll() {
        return LoadProc(CreateShader, "glCreateShader") &&
               LoadProc(ShaderSource, "glShaderSource") &&
               LoadProc(CompileShader, "glCompileShader") &&
               LoadProc(GetShaderiv, "glGetShaderiv") &&
               LoadProc(GetShaderInfoLog, "glGetShaderInfoLog") &&
               LoadProc(CreateProgram, "glCreateProgram") &&
               LoadProc(AttachShader, "glAttachShader") &&
               LoadProc(LinkProgram, "glLinkProgram") &&
               LoadProc(GetProgramiv, "glGetProgramiv") &&
               LoadProc(UseProgram, "glUseProgram") &&
               LoadProc(DeleteShader, "glDeleteShader") &&
               LoadProc(DeleteProgram, "glDeleteProgram") &&
               LoadProc(GetUniformLocation, "glGetUniformLocation") &&
               LoadProc(UniformMatrix4fv, "glUniformMatrix4fv") &&
               LoadProc(GenVertexArrays, "glGenVertexArrays") &&
               LoadProc(BindVertexArray, "glBindVertexArray") &&
               LoadProc(DeleteVertexArrays, "glDeleteVertexArrays") &&
               LoadProc(GenBuffers, "glGenBuffers") &&
               LoadProc(BindBuffer, "glBindBuffer") &&
               LoadProc(BufferData, "glBufferData") &&
               LoadProc(DeleteBuffers, "glDeleteBuffers") &&
               LoadProc(EnableVertexAttribArray, "glEnableVertexAttribArray") &&
               LoadProc(VertexAttribPointer, "glVertexAttribPointer");
    }
}

struct Mat4 {
    float m[16]{};

    static Mat4 Identity() {
        Mat4 r{};
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }

    static Mat4 Perspective(float fovYDeg, float aspect, float zNear, float zFar) {
        const float rad = fovYDeg * (std::numbers::pi_v<float> / 180.0f);
        const float f   = 1.0f / std::tan(rad * 0.5f);
        Mat4 r{};
        r.m[0]  = f / aspect;
        r.m[5]  = f;
        r.m[10] = (zFar + zNear) / (zNear - zFar);
        r.m[11] = -1.0f;
        r.m[14] = (2.0f * zFar * zNear) / (zNear - zFar);
        return r;
    }

    static Mat4 SourceView(const Vec3& eye, const Vec3& angles) {
        Vec3 forward, right, up;
        Movement::AngleVectors(angles, &forward, &right, &up);
        Mat4 r{};
        r.m[0] = right.x;    r.m[4] = right.y;    r.m[8]  = right.z;    r.m[12] = -right.Dot(eye);
        r.m[1] = up.x;       r.m[5] = up.y;       r.m[9]  = up.z;       r.m[13] = -up.Dot(eye);
        r.m[2] = -forward.x; r.m[6] = -forward.y; r.m[10] = -forward.z; r.m[14] =  forward.Dot(eye);
        r.m[3] = 0.0f;       r.m[7] = 0.0f;       r.m[11] = 0.0f;       r.m[15] = 1.0f;
        return r;
    }

    Mat4 operator*(const Mat4& rhs) const {
        Mat4 out{};
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                out.m[col * 4 + row] =
                    m[0 * 4 + row] * rhs.m[col * 4 + 0] +
                    m[1 * 4 + row] * rhs.m[col * 4 + 1] +
                    m[2 * 4 + row] * rhs.m[col * 4 + 2] +
                    m[3 * 4 + row] * rhs.m[col * 4 + 3];
            }
        }
        return out;
    }
};

namespace MeshBuilder {
    static constexpr std::array<uint8_t, 7> GetGlyph5x7(char c) {
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
        switch (c) {
            case 'A': return {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
            case 'B': return {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E};
            case 'C': return {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E};
            case 'D': return {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C};
            case 'E': return {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F};
            case 'F': return {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10};
            case 'G': return {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F};
            case 'H': return {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11};
            case 'I': return {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E};
            case 'J': return {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C};
            case 'K': return {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11};
            case 'L': return {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F};
            case 'M': return {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11};
            case 'N': return {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11};
            case 'O': return {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
            case 'P': return {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10};
            case 'Q': return {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D};
            case 'R': return {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11};
            case 'S': return {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E};
            case 'T': return {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
            case 'U': return {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
            case 'V': return {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04};
            case 'W': return {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11};
            case 'X': return {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11};
            case 'Y': return {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04};
            case 'Z': return {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F};
            case '0': return {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E};
            case '1': return {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E};
            case '2': return {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F};
            case '3': return {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E};
            case '4': return {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02};
            case '5': return {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E};
            case '6': return {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E};
            case '7': return {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08};
            case '8': return {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E};
            case '9': return {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C};
            case ':': return {0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00};
            case '.': return {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C};
            case '-': return {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00};
            case '+': return {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00};
            case '/': return {0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10};
            case '|': return {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};
            case '[': return {0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E};
            case ']': return {0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E};
            case '(': return {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02};
            case ')': return {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08};
            case '%': return {0x19, 0x19, 0x02, 0x04, 0x08, 0x13, 0x13};
            case '!': return {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04};
            default:  return {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        }
    }

    static void AppendQuad(std::vector<GLVertex>& tris, const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, const Vec3& col) {
        tris.push_back({a, col}); tris.push_back({b, col}); tris.push_back({c, col});
        tris.push_back({a, col}); tris.push_back({c, col}); tris.push_back({d, col});
    }

    static void AppendText2D(
        std::vector<GLVertex>& tris,
        float startPxX,
        float startPxY,
        const char* text,
        float pixelScale,
        const Vec3& color,
        int screenW,
        int screenH
    ) {
        if (!text || screenW <= 0 || screenH <= 0) return;

        const float invW = 2.0f / static_cast<float>(screenW);
        const float invH = 2.0f / static_cast<float>(screenH);

        auto emitPass = [&](float offsetX, float offsetY, const Vec3& passCol) {
            float cursorX = startPxX + offsetX;
            const float cursorY = startPxY + offsetY;

            for (const char* p = text; *p != '\0'; ++p) {
                if (*p == ' ') {
                    cursorX += 6.0f * pixelScale;
                    continue;
                }
                const auto glyph = GetGlyph5x7(*p);
                for (int row = 0; row < 7; ++row) {
                    const uint8_t bits = glyph[row];
                    if (bits == 0) continue;
                    for (int col = 0; col < 5; ++col) {
                        if (bits & (1u << (4 - col))) {
                            const float x0 = -1.0f + (cursorX + static_cast<float>(col) * pixelScale) * invW;
                            const float x1 = -1.0f + (cursorX + static_cast<float>(col + 1) * pixelScale) * invW;
                            const float y0 =  1.0f - (cursorY + static_cast<float>(row + 1) * pixelScale) * invH;
                            const float y1 =  1.0f - (cursorY + static_cast<float>(row) * pixelScale) * invH;
                            AppendQuad(tris, {x0, y0, 0.0f}, {x1, y0, 0.0f}, {x1, y1, 0.0f}, {x0, y1, 0.0f}, passCol);
                        }
                    }
                }
                cursorX += 6.0f * pixelScale;
            }
        };

        emitPass(pixelScale * 0.85f, pixelScale * 0.85f, Vec3{0.03f, 0.04f, 0.05f});
        emitPass(0.0f, 0.0f, color);
    }

    static void AppendSolidBox(std::vector<GLVertex>& tris, const Vec3& mn, const Vec3& mx, const Vec3& baseCol) {
        AppendQuad(tris, {mn.x, mn.y, mx.z}, {mx.x, mn.y, mx.z}, {mx.x, mx.y, mx.z}, {mn.x, mx.y, mx.z}, baseCol * 1.15f);
        AppendQuad(tris, {mn.x, mn.y, mn.z}, {mn.x, mn.y, mx.z}, {mx.x, mn.y, mx.z}, {mx.x, mn.y, mn.z}, baseCol * 0.90f);
        AppendQuad(tris, {mn.x, mx.y, mn.z}, {mx.x, mx.y, mn.z}, {mx.x, mx.y, mx.z}, {mn.x, mx.y, mx.z}, baseCol * 0.80f);
        AppendQuad(tris, {mn.x, mn.y, mn.z}, {mn.x, mx.y, mn.z}, {mn.x, mx.y, mx.z}, {mn.x, mn.y, mx.z}, baseCol * 0.72f);
        AppendQuad(tris, {mx.x, mn.y, mn.z}, {mx.x, mn.y, mx.z}, {mx.x, mx.y, mx.z}, {mx.x, mx.y, mn.z}, baseCol * 0.95f);
    }

    static void AppendWireBox(std::vector<GLVertex>& lines, const Vec3& mn, const Vec3& mx, const Vec3& col) {
        const Vec3 p[8] = {
            {mn.x, mn.y, mn.z}, {mx.x, mn.y, mn.z}, {mx.x, mx.y, mn.z}, {mn.x, mx.y, mn.z},
            {mn.x, mn.y, mx.z}, {mx.x, mn.y, mx.z}, {mx.x, mx.y, mx.z}, {mn.x, mx.y, mx.z}
        };
        constexpr int edges[12][2] = {
            {0,1},{1,2},{2,3},{3,0}, {4,5},{5,6},{6,7},{7,4}, {0,4},{1,5},{2,6},{3,7}
        };
        for (const auto& e : edges) {
            lines.push_back({p[e[0]], col});
            lines.push_back({p[e[1]], col});
        }
    }

    // Patlama Şok Dalgası İçin 3 Eksenel Tel-Küre Çizer
    static void AppendWireSphere(std::vector<GLVertex>& lines, const Vec3& center, float radius, const Vec3& col) {
        constexpr int SEGMENTS = 24;
        constexpr float STEP = (2.0f * std::numbers::pi_v<float>) / static_cast<float>(SEGMENTS);
        for (int i = 0; i < SEGMENTS; ++i) {
            const float a0 = static_cast<float>(i) * STEP;
            const float a1 = static_cast<float>(i + 1) * STEP;
            const float c0 = std::cos(a0) * radius, s0 = std::sin(a0) * radius;
            const float c1 = std::cos(a1) * radius, s1 = std::sin(a1) * radius;

            lines.push_back({center + Vec3{c0, s0, 0.0f}, col}); lines.push_back({center + Vec3{c1, s1, 0.0f}, col});
            lines.push_back({center + Vec3{c0, 0.0f, s0}, col}); lines.push_back({center + Vec3{c1, 0.0f, s1}, col});
            lines.push_back({center + Vec3{0.0f, c0, s0}, col}); lines.push_back({center + Vec3{0.0f, c1, s1}, col});
        }
    }

    static void AppendHitboxSkeleton(
        std::vector<GLVertex>& tris,
        std::vector<GLVertex>& lines,
        const Vec3& origin,
        bool drawSolid,
        float colorScale = 1.0f
    ) {
        const auto rig = Movement::BuildPlayerHitboxes(origin);
        for (const HitboxVolume& hb : rig) {
            if (drawSolid) {
                AppendSolidBox(tris, hb.boxMins + Vec3{0.4f, 0.4f, 0.4f}, hb.boxMaxs - Vec3{0.4f, 0.4f, 0.4f}, hb.debugColor * (0.75f * colorScale));
            }
            AppendWireBox(lines, hb.boxMins, hb.boxMaxs, hb.debugColor * colorScale);
        }
    }
}

Engine::~Engine() {
    Shutdown();
}

// ============================================================================
// 1. SDL_AUDIOSTREAM KURULUMU VE PROSEDÜREL 3B KONUMSAL SES MOTORU
// ============================================================================
bool Engine::InitAudioStream() {
    SDL_AudioSpec want{};
    want.freq     = AUDIO_SAMPLE_RATE;
    want.format   = AUDIO_F32SYS;
    want.channels = 2;
    want.samples  = 512; // ~10.6 ms ultra düşük tampon gecikmesi
    want.callback = nullptr;

    m_audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, &m_deviceSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (m_audioDevice == 0) {
        std::fprintf(stderr, "Uyari: SDL Ses cihazi acilamadi: %s\n", SDL_GetError());
        return false;
    }

    m_audioStream = SDL_NewAudioStream(
        AUDIO_F32SYS, 2, AUDIO_SAMPLE_RATE,
        m_deviceSpec.format, m_deviceSpec.channels, m_deviceSpec.freq
    );
    if (!m_audioStream) {
        SDL_CloseAudioDevice(m_audioDevice);
        m_audioDevice = 0;
        return false;
    }

    SDL_PauseAudioDevice(m_audioDevice, 0);
    return true;
}

void Engine::CloseAudioStream() {
    if (m_audioStream) {
        SDL_FreeAudioStream(m_audioStream);
        m_audioStream = nullptr;
    }
    if (m_audioDevice != 0) {
        SDL_CloseAudioDevice(m_audioDevice);
        m_audioDevice = 0;
    }
}

void Engine::EmitSound(SoundType type, const Vec3& origin, float volume, bool is2D) {
    float duration = 0.15f;
    switch (type) {
        case SoundType::FootstepConcrete:
        case SoundType::FootstepWood:
        case SoundType::FootstepMetal: duration = 0.095f; break;
        case SoundType::FireAK47:      duration = 0.240f; break;
        case SoundType::FireDeagle:    duration = 0.340f; break;
        case SoundType::FireAWP:       duration = 0.620f; break;
        case SoundType::HeadshotDink:  duration = 0.190f; break;
        case SoundType::HitFlesh:      duration = 0.090f; break;
        case SoundType::GrenadeBounce: duration = 0.065f; break;
        case SoundType::ExplosionHE:   duration = 0.850f; break;
        case SoundType::FlashbangBang: duration = 0.550f; break;
    }

    // Boş bir ses kanalı bul veya en eski kanalı devral
    std::size_t bestIdx = 0;
    float maxProgress   = -1.0f;
    for (std::size_t i = 0; i < MAX_VOICES; ++i) {
        if (!m_voices[i].active) {
            bestIdx = i;
            break;
        }
        const float prog = m_voices[i].time / std::max(0.01f, m_voices[i].duration);
        if (prog > maxProgress) {
            maxProgress = prog;
            bestIdx     = i;
        }
    }

    // 3B ses ise BVH üzerinden Duvar Sönümlemesi (Occlusion) testi yap
    bool isOccluded = false;
    if (!is2D) {
        const Vec3 eyePos = m_currState.origin + Vec3{0.0f, 0.0f, m_currState.view_offset_z};
        Vec3 toSound = origin - eyePos;
        const float dist = toSound.NormalizeInPlace();
        if (dist > 16.0f) {
            const TraceResult tr = Movement::TraceRay(m_world, eyePos, toSound, dist);
            if (tr.fraction < 0.96f) {
                isOccluded = true; // Ses kaynağı ile oyuncunun kulağı arasında katı fırça/duvar var!
            }
        }
    }

    m_voices[bestIdx] = SoundVoice{
        .type          = type,
        .origin        = origin,
        .time          = 0.0f,
        .duration      = duration,
        .volume        = volume,
        .lowPassStateL = 0.0f,
        .lowPassStateR = 0.0f,
        .is2D          = is2D,
        .occluded      = isOccluded,
        .active        = true
    };
}

float Engine::SynthesizeVoiceSample(const SoundVoice& v, uint32_t& rngState) const {
    constexpr float TWO_PI = 2.0f * std::numbers::pi_v<float>;
    const float t = v.time;

    // Hızlı Xorshift32 Beyaz Gürültü Üreteci [-1.0, +1.0]
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    const float noise = static_cast<float>(static_cast<int32_t>(rngState)) / 2147483648.0f;

    switch (v.type) {
        case SoundType::FootstepConcrete: {
            // Tok beton adım darbesi + taban sürtünmesi
            const float env   = std::exp(-t * 48.0f);
            const float thump = std::sin(TWO_PI * 92.0f * t) * 0.65f;
            const float grit  = noise * std::exp(-t * 65.0f) * 0.55f;
            return (thump + grit) * env;
        }
        case SoundType::FootstepWood: {
            // Ahşap döşeme rezonansı (165 Hz + 310 Hz kutu tınısı)
            const float env  = std::exp(-t * 42.0f);
            const float wood = (std::sin(TWO_PI * 165.0f * t) * 0.7f + std::sin(TWO_PI * 310.0f * t) * 0.35f);
            return (wood + noise * 0.3f) * env;
        }
        case SoundType::FootstepMetal: {
            // Metal ızgara/basamak çınlaması (440 Hz + 890 Hz + 1320 Hz)
            const float env   = std::exp(-t * 36.0f);
            const float clang = std::sin(TWO_PI * 440.0f * t) * 0.5f +
                                std::sin(TWO_PI * 890.0f * t) * 0.4f +
                                std::sin(TWO_PI * 1320.0f * t) * 0.25f;
            return (clang + noise * 0.35f) * env;
        }
        case SoundType::FireAK47: {
            // AK-47: Keskin namlu patlaması + 0.024 sn'de mekanizma şakırtısı
            const float punchFreq = 65.0f + 140.0f * std::exp(-t * 50.0f);
            const float punch     = std::sin(TWO_PI * punchFreq * t) * std::exp(-t * 24.0f);
            const float crack     = noise * std::exp(-t * 19.0f);
            const float boltClick = (t > 0.022f && t < 0.038f) ? (std::sin(TWO_PI * 1950.0f * t) * 0.45f) : 0.0f;
            return std::tanh((punch * 1.4f + crack * 1.1f + boltClick) * 1.3f);
        }
        case SoundType::FireDeagle: {
            // Desert Eagle: Ağır 50 AE top gürlemesi
            const float boomFreq = 42.0f + 110.0f * std::exp(-t * 35.0f);
            const float subBoom  = std::sin(TWO_PI * boomFreq * t) * std::exp(-t * 14.0f);
            const float blast    = noise * std::exp(-t * 12.0f);
            return std::tanh((subBoom * 1.7f + blast * 1.2f) * 1.4f);
        }
        case SoundType::FireAWP: {
            // AWP: Göğüs titreten keskin nişancı gürlemesi + 0.32 sn'de sürgü çekme klik sesi
            const float subFreq  = 32.0f + 95.0f * std::exp(-t * 22.0f);
            const float body     = std::sin(TWO_PI * subFreq * t) * std::exp(-t * 6.5f);
            const float roar     = noise * std::exp(-t * 5.8f);
            const float boltPull = (t > 0.30f && t < 0.36f) ? (std::sin(TWO_PI * 1600.0f * t) * 0.35f) : 0.0f;
            return std::tanh((body * 1.9f + roar * 1.4f + boltPull) * 1.5f);
        }
        case SoundType::HeadshotDink: {
            // CS Kask "Dink" Sesi: 2840 Hz + 4120 Hz parlak metalik çan
            const float env  = std::exp(-t * 22.0f);
            const float ring = std::sin(TWO_PI * 2840.0f * t) * 0.65f +
                               std::sin(TWO_PI * 4120.0f * t) * 0.45f;
            return ring * env;
        }
        case SoundType::HitFlesh: {
            // Kevlar / Gövde İsabeti
            const float env = std::exp(-t * 45.0f);
            return (std::sin(TWO_PI * 135.0f * t) * 0.8f + noise * 0.5f) * env;
        }
        case SoundType::GrenadeBounce: {
            // El Bombası Metalik Sekme Sesi
            const float env = std::exp(-t * 55.0f);
            return (std::sin(TWO_PI * 1240.0f * t) * 0.6f + std::sin(TWO_PI * 1890.0f * t) * 0.4f) * env;
        }
        case SoundType::ExplosionHE: {
            // HE Grenade: Derin sarsıntılı patlama dalgası
            const float env    = std::exp(-t * 4.2f);
            const float rumble = std::sin(TWO_PI * (38.0f + 45.0f * std::exp(-t * 10.0f)) * t);
            return std::tanh((rumble * 1.6f + noise * 1.5f) * env * 1.6f);
        }
        case SoundType::FlashbangBang: {
            // Flashbang: Yüksek frekanslı keskin patlama
            const float env   = std::exp(-t * 9.0f);
            const float pitch = std::sin(TWO_PI * 2400.0f * t) * 0.4f;
            return std::tanh((noise * 1.6f + pitch) * env * 1.5f);
        }
    }
    return 0.0f;
}

void Engine::PumpPositionalAudio(const PlayerState& listenerState) {
    if (m_audioDevice == 0 || !m_audioStream) return;

    // Donanım kuyruğunu ~35 ms (1680 örnek) seviyesinde tut (Sıfır gecikme + kesintisiz akış)
    const uint32_t queuedBytes = SDL_GetQueuedAudioSize(m_audioDevice);
    const uint32_t targetBytes = 1680u * sizeof(float) * 2u;
    if (queuedBytes >= targetBytes) return;

    constexpr int CHUNK_FRAMES = 512;
    float mixBuffer[CHUNK_FRAMES * 2] = {0.0f};
    constexpr float INV_SR = 1.0f / static_cast<float>(AUDIO_SAMPLE_RATE);

    const Vec3 listenerEye = listenerState.origin + Vec3{0.0f, 0.0f, listenerState.view_offset_z};
    Vec3 camForward, camRight, camUp;
    Movement::AngleVectors(m_clientViewAngles, &camForward, &camRight, &camUp);

    for (SoundVoice& v : m_voices) {
        if (!v.active) continue;

        float gainL = v.volume;
        float gainR = v.volume;

        if (!v.is2D) {
            Vec3 toSrc = v.origin - listenerEye;
            const float dist = toSrc.NormalizeInPlace();

            // Ters kare / doğrusal hibrit CS mesafe sönümlemesi
            const float distAtten = 1.0f / (1.0f + (dist / 360.0f) * (dist / 360.0f));

            // Kamera Sağ (camRight) vektörü ile skaler çarpım üzerinden Stereo Panning [-0.92, +0.92]
            const float pan   = std::clamp(camRight.Dot(toSrc), -0.92f, 0.92f);
            const float angle = (pan + 1.0f) * (0.25f * std::numbers::pi_v<float>);

            const float occlusionScale = v.occluded ? 0.42f : 1.0f;
            gainL = v.volume * distAtten * occlusionScale * std::cos(angle);
            gainR = v.volume * distAtten * occlusionScale * std::sin(angle);
        } else {
            gainL = v.volume * 0.65f;
            gainR = v.volume * 0.65f;
        }

        // Duvar arkasındaki (occluded) sesler için tek kutuplu Alçak Geçiren Filtre (Low-Pass) katsayısı
        const float lpAlpha = v.occluded ? 0.14f : 1.0f;

        for (int f = 0; f < CHUNK_FRAMES; ++f) {
            if (v.time >= v.duration) {
                v.active = false;
                break;
            }
            const float raw = SynthesizeVoiceSample(v, m_audioRng);
            v.lowPassStateL += lpAlpha * (raw * gainL - v.lowPassStateL);
            v.lowPassStateR += lpAlpha * (raw * gainR - v.lowPassStateR);

            mixBuffer[f * 2 + 0] += v.lowPassStateL;
            mixBuffer[f * 2 + 1] += v.lowPassStateR;
            v.time += INV_SR;
        }
    }

    // Flashbang Körlük Çınlaması (3100 Hz Tinnitus Sine Wave)
    if (m_tinnitusTimer > 0.0f) {
        const float tinGain = std::clamp(m_tinnitusTimer / 3.0f, 0.0f, 1.0f) * 0.14f;
        constexpr float PHASE_INC = (2.0f * std::numbers::pi_v<float> * 3100.0f) / static_cast<float>(AUDIO_SAMPLE_RATE);
        for (int f = 0; f < CHUNK_FRAMES; ++f) {
            const float s = std::sin(m_tinnitusPhase) * tinGain;
            mixBuffer[f * 2 + 0] += s;
            mixBuffer[f * 2 + 1] += s;
            m_tinnitusPhase += PHASE_INC;
            if (m_tinnitusPhase > 6.283185f) m_tinnitusPhase -= 6.283185f;
        }
    }

    // Yumuşak Sınırlandırıcı (Soft Clipper)
    for (float& sample : mixBuffer) {
        sample = std::clamp(sample, -0.95f, 0.95f);
    }

    // SDL_AudioStream'e yaz ve donanım formatına dönüştürülmüş veriyi ses cihazına aktar
    SDL_AudioStreamPut(m_audioStream, mixBuffer, sizeof(mixBuffer));
    const int avail = SDL_AudioStreamAvailable(m_audioStream);
    if (avail > 0) {
        std::vector<uint8_t> hwBuf(static_cast<std::size_t>(avail));
        const int got = SDL_AudioStreamGet(m_audioStream, hwBuf.data(), avail);
        if (got > 0) {
            SDL_QueueAudio(m_audioDevice, hwBuf.data(), static_cast<uint32_t>(got));
        }
    }
}

// ============================================================================
// 2. EL BOMBASI (HE GRENADE & FLASHBANG) SİMÜLASYONU VE PATLAMA ÇÖZÜCÜSÜ
// ============================================================================
void Engine::SpawnGrenade(GrenadeType type, const Vec3& origin, const Vec3& velocity) {
    for (GrenadeProjectile& g : m_grenades) {
        if (!g.active) {
            g = GrenadeProjectile{
                .type      = type,
                .origin    = origin,
                .velocity  = velocity,
                .fuseTimer = 1.55f,
                .active    = true
            };
            return;
        }
    }
}

void Engine::DetonateGrenade(const GrenadeProjectile& g) {
    // 1. Görsel Şok Dalgası Küresi Ekle
    for (ExplosionVisual& ev : m_explosions) {
        if (!ev.active) {
            ev = ExplosionVisual{
                .origin      = g.origin,
                .type        = g.type,
                .timer       = 0.0f,
                .maxDuration = (g.type == GrenadeType::HE) ? 0.45f : 0.28f,
                .active      = true
            };
            break;
        }
    }

    const Vec3 playerEye = m_currState.origin + Vec3{0.0f, 0.0f, m_currState.view_offset_z};
    const Vec3 botCenter = m_botServerOrigin + Vec3{0.0f, 0.0f, 42.0f};
    const Vec3 botHead   = m_botServerOrigin + Vec3{0.0f, 0.0f, 62.0f};

    if (g.type == GrenadeType::HE) {
        EmitSound(SoundType::ExplosionHE, g.origin, 1.35f, false);

        // Bot üzerindeki HE Patlama Hasarı (BVH görüş hattı kontrolü ile!)
        Vec3 toBot = botCenter - g.origin;
        const float distToBot = toBot.NormalizeInPlace();
        constexpr float HE_RADIUS = 380.0f;

        if (distToBot < HE_RADIUS) {
            const TraceResult los = Movement::TraceRay(m_world, g.origin, toBot, distToBot);
            // Doğrudan görüş varsa tam hasar; ince ahşap arkasındaysa %45 hasar
            float coverScale = 1.0f;
            if (los.fraction < 0.96f) {
                coverScale = (los.hitMaterial == MaterialType::Wood) ? 0.45f : 0.0f;
            }

            const float rawDmg = 98.0f * (1.0f - (distToBot / HE_RADIUS)) * coverScale;
            const float finalDmg = std::floor(rawDmg * 0.75f); // Zırh sönümlemesi
            if (finalDmg >= 1.0f) {
                m_totalHits++;
                m_lastShotHit          = true;
                m_lastHitGroup         = HitGroup::Generic;
                m_lastDamageDealt      = finalDmg;
                m_lastPenetratedWalls  = (coverScale < 1.0f) ? 1 : 0;
                m_lastRewoundBoxOrigin = m_botServerOrigin;
                m_lastTraceStart       = g.origin;
                m_lastTraceEnd         = botCenter;
                m_tracerTimer          = 0.95f;

                m_botHealth -= finalDmg;
                if (m_botHealth <= 0.0f) {
                    m_botKills++;
                    m_botHealth = 100.0f;
                }
            }
        }
    } else {
        // FLASHBANG PATLAMASI: Görüş Hattı (TraceRay) + Bakış Açısı (Anti-Flash Turn)
        EmitSound(SoundType::FlashbangBang, g.origin, 1.25f, false);

        // A) Oyuncu Körlük Kontrolü
        Vec3 eyeToFlash = g.origin - playerEye;
        const float distPlayer = eyeToFlash.NormalizeInPlace();
        if (distPlayer < 1600.0f) {
            const TraceResult losPlayer = Movement::TraceRay(m_world, playerEye, eyeToFlash, distPlayer);
            if (losPlayer.fraction >= 0.97f) {
                Vec3 camFwd, camRight, camUp;
                Movement::AngleVectors(m_clientViewAngles, &camFwd, &camRight, &camUp);
                const float viewCos  = camFwd.Dot(eyeToFlash); // +1.0 = Tam bombaya bakıyor, -1.0 = Arkası dönük
                const float distFall = std::clamp(1.0f - (distPlayer / 1600.0f), 0.2f, 1.0f);

                if (viewCos > 0.05f) {
                    // Bombaya bakarken yakalandı: Tam Körlük + Kulak Çınlaması!
                    const float intensity = (0.45f + 0.95f * viewCos) * distFall;
                    m_flashAlpha    = std::max(m_flashAlpha, intensity);
                    m_tinnitusTimer = std::max(m_tinnitusTimer, 3.2f * intensity);
                } else {
                    // Oyuncu arkasını döndü (Anti-Flash): Çok hafif anlık parlama!
                    m_flashAlpha = std::max(m_flashAlpha, 0.22f * distFall);
                }
            }
        }

        // B) Bot Körlük Kontrolü (Duvar arkasında değilse Bot 3.5 sn kör olur ve olduğu yerde donar!)
        Vec3 flashToBot = botHead - g.origin;
        const float distBot = flashToBot.NormalizeInPlace();
        if (distBot < 1200.0f) {
            const TraceResult losBot = Movement::TraceRay(m_world, g.origin, flashToBot, distBot);
            if (losBot.fraction >= 0.97f) {
                m_botBlindTimer = 3.5f;
            }
        }
    }
}

void Engine::TickGrenades(float dt) {
    for (GrenadeProjectile& g : m_grenades) {
        if (!g.active) continue;

        Vec3  bouncePos{};
        float impactSpeed = 0.0f;
        if (Movement::SimulateGrenadeStep(g, m_world, m_moveVars.gravity, dt, &bouncePos, &impactSpeed)) {
            const float vol = std::clamp(impactSpeed / 380.0f, 0.15f, 0.95f);
            EmitSound(SoundType::GrenadeBounce, bouncePos, vol, false);
        }

        if (g.fuseTimer <= 0.0f) {
            DetonateGrenade(g);
            g.active = false;
        }
    }
}

static void SetSocketNonBlocking(uintptr_t sock) {
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(static_cast<SOCKET>(sock), FIONBIO, &mode);
#else
    const int flags = fcntl(static_cast<int>(sock), F_GETFL, 0);
    fcntl(static_cast<int>(sock), F_SETFL, flags | O_NONBLOCK);
#endif
}

bool Engine::InitUDPSockets() {
#ifdef _WIN32
    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;
#endif
    m_serverSock = static_cast<uintptr_t>(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    m_clientSock = static_cast<uintptr_t>(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    if (m_serverSock == ~uintptr_t(0) || m_clientSock == ~uintptr_t(0)) return false;

    sockaddr_in srvAddr{};
    srvAddr.sin_family      = AF_INET;
    srvAddr.sin_port        = htons(SERVER_UDP_PORT);
    srvAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    bind(static_cast<int>(m_serverSock), reinterpret_cast<const sockaddr*>(&srvAddr), sizeof(srvAddr));
    SetSocketNonBlocking(m_serverSock);
    SetSocketNonBlocking(m_clientSock);
    return true;
}

void Engine::CloseUDPSockets() {
#ifdef _WIN32
    if (m_serverSock != ~uintptr_t(0)) closesocket(static_cast<SOCKET>(m_serverSock));
    if (m_clientSock != ~uintptr_t(0)) closesocket(static_cast<SOCKET>(m_clientSock));
    WSACleanup();
#else
    if (m_serverSock != ~uintptr_t(0)) close(static_cast<int>(m_serverSock));
    if (m_clientSock != ~uintptr_t(0)) close(static_cast<int>(m_clientSock));
#endif
    m_serverSock = ~uintptr_t(0);
    m_clientSock = ~uintptr_t(0);
}

bool Engine::InitGL33Pipeline() {
    if (!GL33::LoadAll()) return false;

    const char* vsSrc = R"(#version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aColor;
        uniform mat4 uMVP;
        out vec3 vColor;
        void main() {
            vColor = aColor;
            gl_Position = uMVP * vec4(aPos, 1.0);
        }
    )";

    const char* fsSrc = R"(#version 330 core
        in vec3 vColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(vColor, 1.0);
        }
    )";

    auto compileShader = [](GLenum type, const char* src) -> GLuint {
        GLuint s = GL33::CreateShader(type);
        GL33::ShaderSource(s, 1, &src, nullptr);
        GL33::CompileShader(s);
        return s;
    };

    const GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
    const GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);

    m_shaderProgram = GL33::CreateProgram();
    GL33::AttachShader(m_shaderProgram, vs);
    GL33::AttachShader(m_shaderProgram, fs);
    GL33::LinkProgram(m_shaderProgram);
    GL33::DeleteShader(vs);
    GL33::DeleteShader(fs);

    m_uMvpLocation = GL33::GetUniformLocation(m_shaderProgram, "uMVP");

    GL33::GenVertexArrays(1, &m_dynVao);
    GL33::GenBuffers(1, &m_dynVbo);
    GL33::BindVertexArray(m_dynVao);
    GL33::BindBuffer(GL_ARRAY_BUFFER, m_dynVbo);
    GL33::EnableVertexAttribArray(0);
    GL33::VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex), reinterpret_cast<const void*>(0));
    GL33::EnableVertexAttribArray(1);
    GL33::VertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex), reinterpret_cast<const void*>(sizeof(Vec3)));
    GL33::BindVertexArray(0);

    BuildStaticWorldMesh();
    return true;
}

void Engine::BuildStaticWorldMesh() {
    std::vector<GLVertex> tris;
    std::vector<GLVertex> lines;

    for (std::size_t idx = 1; idx < m_world.brushes.size(); ++idx) {
        const Brush& b = m_world.brushes[idx];
        if (b.planes.size() == 6) {
            MeshBuilder::AppendSolidBox(tris, b.mins, b.maxs, b.color);
            MeshBuilder::AppendWireBox(lines, b.mins, b.maxs, b.color * 1.35f);
        } else {
            const Vec3& mn = b.mins;
            const Vec3& mx = b.maxs;
            const Vec3 A{mn.x, mn.y, mn.z}, B{mx.x, mn.y, mn.z};
            const Vec3 C{mx.x, mx.y, mx.z}, D{mn.x, mx.y, mx.z};
            const Vec3 E{mn.x, mx.y, mn.z}, F{mx.x, mx.y, mn.z};

            MeshBuilder::AppendQuad(tris, A, B, C, D, b.color * 1.1f);
            MeshBuilder::AppendQuad(tris, E, F, C, D, b.color * 0.75f);
            tris.push_back({A, b.color * 0.85f}); tris.push_back({E, b.color * 0.85f}); tris.push_back({D, b.color * 0.85f});
            tris.push_back({B, b.color * 0.85f}); tris.push_back({C, b.color * 0.85f}); tris.push_back({F, b.color * 0.85f});

            lines.push_back({A, {1.0f, 0.7f, 0.3f}}); lines.push_back({B, {1.0f, 0.7f, 0.3f}});
            lines.push_back({B, {1.0f, 0.7f, 0.3f}}); lines.push_back({C, {1.0f, 0.7f, 0.3f}});
            lines.push_back({C, {1.0f, 0.7f, 0.3f}}); lines.push_back({D, {1.0f, 0.7f, 0.3f}});
            lines.push_back({D, {1.0f, 0.7f, 0.3f}}); lines.push_back({A, {1.0f, 0.7f, 0.3f}});
        }
    }

    constexpr int GRID_EXTENT = 4096;
    constexpr int GRID_STEP   = 128;
    for (int i = -GRID_EXTENT; i <= GRID_EXTENT; i += GRID_STEP) {
        const Vec3 col = (i == 0) ? Vec3{0.45f, 0.45f, 0.55f} :
                         (i % 512 == 0) ? Vec3{0.22f, 0.26f, 0.32f} : Vec3{0.12f, 0.14f, 0.18f};
        lines.push_back({{static_cast<float>(i), static_cast<float>(-GRID_EXTENT), 0.05f}, col});
        lines.push_back({{static_cast<float>(i), static_cast<float>( GRID_EXTENT), 0.05f}, col});
        lines.push_back({{static_cast<float>(-GRID_EXTENT), static_cast<float>(i), 0.05f}, col});
        lines.push_back({{static_cast<float>( GRID_EXTENT), static_cast<float>(i), 0.05f}, col});
    }

    m_worldTriCount  = static_cast<uint32_t>(tris.size());
    m_worldLineCount = static_cast<uint32_t>(lines.size());

    std::vector<GLVertex> combined;
    combined.reserve(tris.size() + lines.size());
    combined.insert(combined.end(), tris.begin(), tris.end());
    combined.insert(combined.end(), lines.begin(), lines.end());

    GL33::GenVertexArrays(1, &m_worldVao);
    GL33::GenBuffers(1, &m_worldVbo);
    GL33::BindVertexArray(m_worldVao);
    GL33::BindBuffer(GL_ARRAY_BUFFER, m_worldVbo);
    GL33::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(combined.size() * sizeof(GLVertex)), combined.data(), GL_STATIC_DRAW);
    GL33::EnableVertexAttribArray(0);
    GL33::VertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex), reinterpret_cast<const void*>(0));
    GL33::EnableVertexAttribArray(1);
    GL33::VertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex), reinterpret_cast<const void*>(sizeof(Vec3)));
    GL33::BindVertexArray(0);
}

bool Engine::Init(const char* title, int width, int height) {
    m_width  = width;
    m_height = height;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) return false;

    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "0");
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SCALING, "0");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!m_window) return false;

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) return false;

    SDL_GL_SetSwapInterval(0);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.06f, 0.07f, 0.09f, 1.0f);

    m_world.BuildTestArena();
    if (!InitGL33Pipeline()) return false;
    InitUDPSockets();
    InitAudioStream();

    m_prevState       = PlayerState{};
    m_currState       = PlayerState{};
    m_serverAuthState = PlayerState{};
    m_running         = true;
    return true;
}

void Engine::Shutdown() {
    CloseAudioStream();
    CloseUDPSockets();
    if (m_worldVao) { GL33::DeleteVertexArrays(1, &m_worldVao); m_worldVao = 0; }
    if (m_worldVbo) { GL33::DeleteBuffers(1, &m_worldVbo);      m_worldVbo = 0; }
    if (m_dynVao)   { GL33::DeleteVertexArrays(1, &m_dynVao);   m_dynVao   = 0; }
    if (m_dynVbo)   { GL33::DeleteBuffers(1, &m_dynVbo);        m_dynVbo   = 0; }
    if (m_shaderProgram) { GL33::DeleteProgram(m_shaderProgram); m_shaderProgram = 0; }

    if (m_glContext) { SDL_GL_DeleteContext(m_glContext); m_glContext = nullptr; }
    if (m_window)    { SDL_DestroyWindow(m_window);       m_window    = nullptr; }
    SDL_Quit();
}

void Engine::AddBulletDecal(const Vec3& pos, const Vec3& normal, bool hitBot, bool isExitPoint) {
    m_decals[m_decalCursor] = BulletDecal{pos, normal, hitBot, isExitPoint, true};
    m_decalCursor = (m_decalCursor + 1) % MAX_DECALS;
}

void Engine::PollEvents(double current_subtick_fraction) {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
            case SDL_QUIT:
                m_running = false;
                break;

            case SDL_WINDOWEVENT:
                if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
                    m_width  = ev.window.data1;
                    m_height = ev.window.data2;
                    glViewport(0, 0, m_width, m_height);
                }
                break;

            case SDL_KEYDOWN:
                if (ev.key.keysym.sym == SDLK_ESCAPE) {
                    m_running = false;
                } else if (ev.key.keysym.sym == SDLK_SPACE && ev.key.repeat == 0) {
                    m_jumpPressedThisTick = true;
                    m_pendingJumpSubtick  = static_cast<float>(current_subtick_fraction);
                } else if (ev.key.keysym.sym == SDLK_1 && ev.key.repeat == 0) {
                    m_pendingWeaponSelect = 1; // AK-47
                } else if (ev.key.keysym.sym == SDLK_2 && ev.key.repeat == 0) {
                    m_pendingWeaponSelect = 2; // Desert Eagle
                } else if (ev.key.keysym.sym == SDLK_3 && ev.key.repeat == 0) {
                    m_pendingWeaponSelect = 3; // AWP
                } else if (ev.key.keysym.sym == SDLK_4 && ev.key.repeat == 0) {
                    m_pendingWeaponSelect = 4; // HE Grenade Yuvası
                } else if (ev.key.keysym.sym == SDLK_5 && ev.key.repeat == 0) {
                    m_pendingWeaponSelect = 5; // Flashbang Yuvası
                } else if (ev.key.keysym.sym == SDLK_g && ev.key.repeat == 0) {
                    m_quickHePressed = true;   // G: Hızlı HE Grenade Atışı
                } else if (ev.key.keysym.sym == SDLK_c && ev.key.repeat == 0) {
                    m_quickFlashPressed = true;// C: Hızlı Flashbang Atışı
                } else if (ev.key.keysym.sym == SDLK_F1 && ev.key.repeat == 0) {
                    m_moveVars.autobhop = !m_moveVars.autobhop;
                } else if (ev.key.keysym.sym == SDLK_F2 && ev.key.repeat == 0) {
                    m_simulatedRttMs += 100.0f;
                    if (m_simulatedRttMs > 200.0f) m_simulatedRttMs = 0.0f;
                } else if (ev.key.keysym.sym == SDLK_F3 && ev.key.repeat == 0) {
                    m_showBvhDebug = !m_showBvhDebug;
                } else if (ev.key.keysym.sym == SDLK_F4 && ev.key.repeat == 0) {
                    m_xrayHitboxes = !m_xrayHitboxes;
                } else if (ev.key.keysym.sym == SDLK_r && ev.key.repeat == 0) {
                    for (auto& d : m_decals) d.active = false;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (ev.button.button == SDL_BUTTON_LEFT) {
                    m_attackPressedThisTick = true;
                    m_pendingAttackSubtick  = static_cast<float>(current_subtick_fraction);
                } else if (ev.button.button == SDL_BUTTON_RIGHT) {
                    m_attack2PressedThisTick = true;
                }
                break;

            case SDL_MOUSEWHEEL:
                if (ev.wheel.y != 0) {
                    m_jumpPressedThisTick = true;
                    m_pendingJumpSubtick  = static_cast<float>(current_subtick_fraction);
                }
                break;

            case SDL_MOUSEMOTION: {
                float zoomSensScale = 1.0f;
                if (m_currState.scope_level == 1) zoomSensScale = 0.45f;
                if (m_currState.scope_level == 2) zoomSensScale = 0.18f;

                const float dx = static_cast<float>(ev.motion.xrel);
                const float dy = static_cast<float>(ev.motion.yrel);
                m_clientViewAngles.y -= dx * m_sensitivity * zoomSensScale * m_mYaw;
                m_clientViewAngles.x += dy * m_sensitivity * zoomSensScale * m_mPitch;
                m_clientViewAngles.x  = std::clamp(m_clientViewAngles.x, -89.0f, 89.0f);
                m_clientViewAngles.y  = std::remainder(m_clientViewAngles.y, 360.0f);
                break;
            }
        }
    }
}

UserCmd Engine::BuildUserCmd(double subtick_fraction) {
    const Uint8* keys      = SDL_GetKeyboardState(nullptr);
    const Uint32 mouseMask = SDL_GetMouseState(nullptr, nullptr);

    UserCmd cmd{};
    cmd.command_number   = ++m_commandNumber;
    cmd.tick_count       = ++m_currentTick;
    cmd.subtick_fraction = m_attackPressedThisTick ? m_pendingAttackSubtick :
                           (m_jumpPressedThisTick  ? m_pendingJumpSubtick   : static_cast<float>(subtick_fraction));
    cmd.viewangles       = m_clientViewAngles;
    cmd.weapon_select    = m_pendingWeaponSelect;

    if (keys[SDL_SCANCODE_W]) { cmd.forwardmove += m_moveVars.maxspeed; cmd.buttons |= Buttons::FORWARD; }
    if (keys[SDL_SCANCODE_S]) { cmd.forwardmove -= m_moveVars.maxspeed; cmd.buttons |= Buttons::BACK; }
    if (keys[SDL_SCANCODE_D]) { cmd.sidemove    += m_moveVars.maxspeed; cmd.buttons |= Buttons::MOVERIGHT; }
    if (keys[SDL_SCANCODE_A]) { cmd.sidemove    -= m_moveVars.maxspeed; cmd.buttons |= Buttons::MOVELEFT; }

    if (keys[SDL_SCANCODE_SPACE] || m_jumpPressedThisTick) {
        cmd.buttons |= Buttons::JUMP;
    }
    if (keys[SDL_SCANCODE_LCTRL]) {
        cmd.buttons |= Buttons::DUCK;
    }
    if ((mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) || m_attackPressedThisTick) {
        cmd.buttons |= Buttons::ATTACK;
    }
    if ((mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) || m_attack2PressedThisTick) {
        cmd.buttons |= Buttons::ATTACK2;
    }
    if (m_quickHePressed) {
        cmd.buttons |= Buttons::QUICK_HE;
    }
    if (m_quickFlashPressed) {
        cmd.buttons |= Buttons::QUICK_FLASH;
    }

    m_jumpPressedThisTick    = false;
    m_attackPressedThisTick  = false;
    m_attack2PressedThisTick = false;
    m_quickHePressed         = false;
    m_quickFlashPressed      = false;
    m_pendingWeaponSelect    = 0;
    return cmd;
}

bool Engine::RewindBotLagRecord(float target_simulation_time, Vec3& out_rewound_origin) const {
    if (target_simulation_time <= 0.0f) return false;

    const double exact_tick = static_cast<double>(target_simulation_time) / TICK_INTERVAL;
    const auto tick0 = static_cast<uint32_t>(std::floor(exact_tick));
    const auto tick1 = tick0 + 1;

    if (tick1 > m_currentTick || (m_currentTick - tick0) >= HISTORY_BUFFER_SIZE) {
        return false;
    }

    const LagRecord& rec0 = m_botLagRing[tick0 & HISTORY_MASK];
    const LagRecord& rec1 = m_botLagRing[tick1 & HISTORY_MASK];
    if (rec0.tick_count != tick0 || rec1.tick_count != tick1) {
        return false;
    }

    const float frac = static_cast<float>(exact_tick - static_cast<double>(tick0));
    out_rewound_origin = Vec3::Lerp(rec0.origin, rec1.origin, frac);
    return true;
}

void Engine::ReconcileClientPrediction(const PlayerState& authoritative_server_state) {
    const uint32_t acked_tick = authoritative_server_state.tick_count;
    if (acked_tick == 0 || acked_tick > m_currentTick || (m_currentTick - acked_tick) >= HISTORY_BUFFER_SIZE) {
        return;
    }

    const PlayerState& predicted = m_predictionRing[acked_tick & HISTORY_MASK].predicted_state;
    const float pos_err_sq   = (predicted.origin - authoritative_server_state.origin).LengthSq();
    const float punch_err_sq = (predicted.aim_punch_angle - authoritative_server_state.aim_punch_angle).LengthSq();

    if (pos_err_sq > 0.001f || punch_err_sq > 0.0001f || predicted.active_weapon != authoritative_server_state.active_weapon) {
        PlayerState replayed_state = authoritative_server_state;
        for (uint32_t tick = acked_tick + 1; tick <= m_currentTick; ++tick) {
            PredictionSlot& slot = m_predictionRing[tick & HISTORY_MASK];
            replayed_state = Movement::SimulateTick(
                replayed_state,
                slot.cmd,
                m_world,
                m_moveVars,
                static_cast<float>(TICK_INTERVAL)
            );
            slot.predicted_state = replayed_state;
        }
        m_currState = replayed_state;
    }
}

void Engine::ServerProcessCmdPacket(const NetCmdPacket& pkt) {
    UserCmd srvCmd{};
    srvCmd.command_number   = pkt.command_number;
    srvCmd.tick_count       = pkt.client_tick;
    srvCmd.subtick_fraction = pkt.subtick_fraction;
    srvCmd.viewangles       = pkt.viewangles;
    srvCmd.forwardmove      = pkt.forwardmove;
    srvCmd.sidemove         = pkt.sidemove;
    srvCmd.buttons          = pkt.buttons;
    srvCmd.weapon_select    = pkt.weapon_select;

    const PlayerState preFireState = m_serverAuthState;

    m_serverAuthState = Movement::SimulateTick(
        m_serverAuthState,
        srvCmd,
        m_world,
        m_moveVars,
        static_cast<float>(TICK_INTERVAL)
    );

    NetSnapshotPacket snap{};
    snap.ack_command_number  = pkt.command_number;
    snap.server_tick         = pkt.client_tick;
    snap.authoritative_state = m_serverAuthState;
    snap.bot_server_origin   = m_botServerOrigin;

    if (m_serverAuthState.fired_this_tick) {
        snap.shot_fired = true;

        const float subtickTimeOffset = pkt.subtick_fraction * static_cast<float>(TICK_INTERVAL);
        const float targetRewindTime  = pkt.client_interp_time + subtickTimeOffset;

        Vec3 rewoundBotOrigin = m_botServerOrigin;
        if (!RewindBotLagRecord(targetRewindTime, rewoundBotOrigin)) {
            rewoundBotOrigin = m_botServerOrigin;
        }
        snap.rewound_bot_origin = rewoundBotOrigin;

        const Vec3 eyePos = m_serverAuthState.origin + Vec3{0.0f, 0.0f, m_serverAuthState.view_offset_z};
        const Vec3 shotDir = Movement::ComputeShotDirection(
            pkt.viewangles,
            preFireState.aim_punch_angle,
            m_serverAuthState.inaccuracy,
            pkt.command_number,
            m_moveVars.recoil_scale
        );

        const FireBulletResult bulletRes = Movement::FireBulletPenetrating(
            m_world,
            m_serverAuthState.active_weapon,
            eyePos,
            shotDir,
            rewoundBotOrigin
        );

        snap.hit_confirmed    = bulletRes.hitPlayer;
        snap.hit_group        = bulletRes.hitGroup;
        snap.damage_dealt     = bulletRes.finalDamage;
        snap.penetrated_walls = bulletRes.penetratedCount;
        snap.final_end_pos    = bulletRes.finalEndPos;
        snap.impact_count     = bulletRes.impactCount;
        snap.impacts          = bulletRes.impacts;
    }

    sockaddr_in srvAddr{};
    srvAddr.sin_family      = AF_INET;
    srvAddr.sin_port        = htons(SERVER_UDP_PORT);
    srvAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    sendto(
        static_cast<int>(m_clientSock),
        reinterpret_cast<const char*>(&snap),
        sizeof(snap),
        0,
        reinterpret_cast<const sockaddr*>(&srvAddr),
        sizeof(srvAddr)
    );
}

void Engine::ProcessNetworkQueues(double current_time) {
    sockaddr_in fromAddr{};
    SockLenT fromLen = sizeof(fromAddr);
    uint8_t buf[1024];

    while (true) {
        const int bytes = recvfrom(
            static_cast<int>(m_serverSock),
            reinterpret_cast<char*>(buf),
            sizeof(buf),
            0,
            reinterpret_cast<sockaddr*>(&fromAddr),
            &fromLen
        );
        if (bytes <= 0) break;

        if (buf[0] == static_cast<uint8_t>(PacketType::ClientUserCmd) && bytes == sizeof(NetCmdPacket)) {
            NetCmdPacket pkt{};
            std::memcpy(&pkt, buf, sizeof(NetCmdPacket));
            const double deliverTime = current_time + static_cast<double>(m_simulatedRttMs) * 0.001;
            m_inFlightToServer.push_back({deliverTime, pkt});
        } else if (buf[0] == static_cast<uint8_t>(PacketType::ServerSnapshot) && bytes == sizeof(NetSnapshotPacket)) {
            NetSnapshotPacket snap{};
            std::memcpy(&snap, buf, sizeof(NetSnapshotPacket));

            ReconcileClientPrediction(snap.authoritative_state);

            if (snap.shot_fired) {
                m_totalShots++;
                m_lastShotHit          = snap.hit_confirmed;
                m_lastRewoundBoxOrigin = snap.rewound_bot_origin;
                m_lastTraceStart       = m_currState.origin + Vec3{0.0f, 0.0f, m_currState.view_offset_z - 5.0f};
                m_lastTraceEnd         = snap.final_end_pos;
                m_tracerTimer          = 0.95f;

                if (snap.hit_confirmed) {
                    m_totalHits++;
                    m_lastHitGroup        = snap.hit_group;
                    m_lastDamageDealt     = snap.damage_dealt;
                    m_lastPenetratedWalls = snap.penetrated_walls;

                    // 3B Konumsal İsabet Sesi: Kafa vuruşunda "Headshot Dink", gövdede "Kevlar Flesh"
                    const SoundType hitSnd = (snap.hit_group == HitGroup::Head) ? SoundType::HeadshotDink : SoundType::HitFlesh;
                    EmitSound(hitSnd, snap.final_end_pos, 1.15f, false);

                    m_botHealth -= snap.damage_dealt;
                    if (m_botHealth <= 0.0f) {
                        m_botKills++;
                        m_botHealth = 100.0f;
                    }
                }

                for (uint8_t i = 0; i < snap.impact_count; ++i) {
                    const bool isFinalPlayerHit = snap.hit_confirmed && (i + 1 == snap.impact_count);
                    AddBulletDecal(snap.impacts[i].pos, snap.impacts[i].normal, isFinalPlayerHit, snap.impacts[i].isExit);
                }
            }
        }
    }

    while (!m_inFlightToServer.empty() && m_inFlightToServer.front().deliver_time <= current_time) {
        ServerProcessCmdPacket(m_inFlightToServer.front().pkt);
        m_inFlightToServer.pop_front();
    }
}

void Engine::TickUpdate(double subtick_fraction) {
    constexpr float DT = static_cast<float>(TICK_INTERVAL);
    m_prevState = m_currState;

    const UserCmd cmd = BuildUserCmd(subtick_fraction);

    m_currState = Movement::SimulateTick(
        m_currState,
        cmd,
        m_world,
        m_moveVars,
        DT
    );

    // 1. İstemci Anlık Silah Sesi (Sıfır Gecikmeli 2D/Yakın Alan Sentezi)
    if (m_currState.fired_this_tick) {
        SoundType fireSnd = SoundType::FireAK47;
        if (m_currState.active_weapon == WeaponID::Deagle) fireSnd = SoundType::FireDeagle;
        if (m_currState.active_weapon == WeaponID::AWP)    fireSnd = SoundType::FireAWP;
        EmitSound(fireSnd, m_currState.origin, 0.90f, true);
    }

    // 2. El Bombası Fırlatıldıysa Sahneye Ekle
    if (m_currState.threw_grenade_this_tick) {
        Vec3 fwd, right, up;
        Movement::AngleVectors(m_currState.viewangles, &fwd, &right, &up);
        const Vec3 spawnPos = m_currState.origin + Vec3{0.0f, 0.0f, m_currState.view_offset_z - 4.0f} + fwd * 18.0f;
        const Vec3 throwVel = Movement::ComputeGrenadeThrowVelocity(
            m_currState.viewangles,
            m_currState.velocity,
            m_currState.thrown_grenade_speed
        );
        SpawnGrenade(m_currState.thrown_grenade_type, spawnPos, throwVel);
    }

    // 3. Oyuncu Ayak Sesi (Hız > 135 u/s iken yürünen fırçanın materyaline göre!)
    const float playerSpeed2D = m_currState.velocity.Length2D();
    if (m_currState.on_ground && playerSpeed2D > 135.0f) {
        m_playerStepDistAccum += playerSpeed2D * DT;
        if (m_playerStepDistAccum >= 92.0f) {
            m_playerStepDistAccum = 0.0f;
            SoundType stepType = SoundType::FootstepConcrete;
            if (m_currState.ground_material == MaterialType::Wood)  stepType = SoundType::FootstepWood;
            if (m_currState.ground_material == MaterialType::Metal) stepType = SoundType::FootstepMetal;
            EmitSound(stepType, m_currState.origin, 0.48f, true);
        }
    } else if (!m_currState.on_ground) {
        m_playerStepDistAccum = 65.0f; // Yere indiği an adım sesi vermesi için
    }

    // 4. Aktif El Bombalarını İlerlet
    TickGrenades(DT);

    const uint32_t slot = cmd.tick_count & HISTORY_MASK;
    m_predictionRing[slot] = PredictionSlot{cmd, m_currState};

    // 5. Bot Güncellemesi (Flashbang ile kör edildiyse hareket etmez!)
    const Vec3 prevBotOrigin = m_botServerOrigin;
    if (m_botBlindTimer > 0.0f) {
        m_botBlindTimer = std::max(0.0f, m_botBlindTimer - DT);
    } else {
        m_botPhaseTime += DT;
        m_botServerOrigin = Vec3{420.0f, std::sin(m_botPhaseTime * 1.65f) * 265.0f, 0.0f};
    }

    // Botun 3B Uzamsal Ayak Sesi (Kulaklıkla sol/sağ konumunu ve duvar arkası boğukluğunu duyabilirsin!)
    const float botStepDelta = (m_botServerOrigin - prevBotOrigin).Length2D();
    m_botStepDistAccum += botStepDelta;
    if (m_botStepDistAccum >= 88.0f) {
        m_botStepDistAccum = 0.0f;
        EmitSound(SoundType::FootstepConcrete, m_botServerOrigin + Vec3{0.0f, 0.0f, 4.0f}, 0.85f, false);
    }

    const float simTime = static_cast<float>(cmd.tick_count * TICK_INTERVAL);
    m_botLagRing[slot] = LagRecord{
        .tick_count      = cmd.tick_count,
        .simulation_time = simTime,
        .origin          = m_botServerOrigin
    };

    const float clientVisibleTime = std::max(0.0f, simTime - (m_simulatedRttMs * 0.001f));

    NetCmdPacket netPkt{};
    netPkt.command_number     = cmd.command_number;
    netPkt.client_tick        = cmd.tick_count;
    netPkt.subtick_fraction   = cmd.subtick_fraction;
    netPkt.client_interp_time = clientVisibleTime;
    netPkt.viewangles         = cmd.viewangles;
    netPkt.forwardmove        = cmd.forwardmove;
    netPkt.sidemove           = cmd.sidemove;
    netPkt.buttons            = cmd.buttons;
    netPkt.weapon_select      = cmd.weapon_select;

    sockaddr_in srvAddr{};
    srvAddr.sin_family      = AF_INET;
    srvAddr.sin_port        = htons(SERVER_UDP_PORT);
    srvAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    sendto(
        static_cast<int>(m_clientSock),
        reinterpret_cast<const char*>(&netPkt),
        sizeof(netPkt),
        0,
        reinterpret_cast<const sockaddr*>(&srvAddr),
        sizeof(srvAddr)
    );
}

void Engine::RenderViewmodel(const PlayerState& renderState, double frame_dt) {
    if (renderState.scope_level > 0) return;

    const float speed2D = renderState.velocity.Length2D();
    if (renderState.on_ground && speed2D > 5.0f) {
        m_bobTime += frame_dt * static_cast<double>(std::min(speed2D, 320.0f)) * 0.038;
    } else {
        m_bobTime += frame_dt * 1.5;
    }

    const float bobScale = renderState.on_ground ? std::clamp(speed2D / 250.0f, 0.05f, 1.15f) : 0.15f;
    const float bobRight = static_cast<float>(std::sin(m_bobTime)) * 0.45f * bobScale;
    const float bobUp    = static_cast<float>(std::abs(std::cos(m_bobTime))) * -0.35f * bobScale;

    const WeaponSpec& spec = Movement::GetWeaponSpec(renderState.active_weapon);
    const float fireFrac   = std::clamp(renderState.weapon_cooldown / spec.fireInterval, 0.0f, 1.0f);
    const float kickBack   = std::pow(fireFrac, 2.0f) * ((renderState.active_weapon == WeaponID::AWP) ? 3.8f : 1.9f);
    const float deployDrop = (renderState.switch_cooldown / 0.25f) * -8.5f;

    const Vec3 vmOffset{
        12.5f - kickBack,
        -5.8f + bobRight,
        -5.2f + bobUp + deployDrop - (renderState.aim_punch_angle.x * 0.12f)
    };

    std::vector<GLVertex> vmTris;
    std::vector<GLVertex> vmLines;

    auto addPart = [&](const Vec3& localMins, const Vec3& localMaxs, const Vec3& col) {
        MeshBuilder::AppendSolidBox(vmTris, vmOffset + localMins, vmOffset + localMaxs, col);
        MeshBuilder::AppendWireBox(vmLines, vmOffset + localMins, vmOffset + localMaxs, col * 1.35f);
    };

    if (renderState.active_weapon == WeaponID::AK47) {
        addPart({-5.0f, -1.1f, -4.2f}, { 1.5f, 1.1f, -1.2f}, {0.58f, 0.32f, 0.14f});
        addPart({ 1.5f, -1.2f, -2.2f}, {10.5f, 1.2f,  0.6f}, {0.22f, 0.24f, 0.28f});
        addPart({ 3.5f, -0.9f, -6.5f}, { 6.5f, 0.9f, -2.2f}, {0.72f, 0.36f, 0.12f});
        addPart({10.5f, -1.1f, -1.8f}, {17.0f, 1.1f,  0.4f}, {0.65f, 0.36f, 0.15f});
        addPart({17.0f, -0.35f,-1.0f}, {26.5f, 0.35f,-0.2f}, {0.15f, 0.17f, 0.20f});
        addPart({25.0f, -0.25f,-0.2f}, {26.2f, 0.25f, 1.2f}, {0.15f, 0.17f, 0.20f});
    } else if (renderState.active_weapon == WeaponID::Deagle) {
        const float slideKick = fireFrac * 2.4f;
        addPart({ 2.5f, -0.95f,-5.6f}, { 6.2f, 0.95f,-1.2f}, {0.14f, 0.15f, 0.18f});
        addPart({ 2.0f, -1.15f,-1.2f}, {14.5f, 1.15f, 0.2f}, {0.55f, 0.58f, 0.64f});
        addPart({ 1.8f - slideKick, -1.25f, 0.2f}, {14.8f - slideKick, 1.25f, 2.1f}, {0.78f, 0.82f, 0.88f});
    } else if (renderState.active_weapon == WeaponID::AWP) {
        addPart({-6.5f, -1.2f, -4.0f}, {14.0f, 1.2f, -0.2f}, {0.22f, 0.38f, 0.20f});
        addPart({14.0f, -0.45f,-1.1f}, {33.0f, 0.45f,-0.1f}, {0.16f, 0.18f, 0.22f});
        addPart({ 2.5f, -0.75f, 0.5f}, {15.5f, 0.75f, 2.3f}, {0.12f, 0.14f, 0.18f});
        addPart({ 4.0f, -0.35f,-0.2f}, { 5.2f, 0.35f, 0.5f}, {0.20f, 0.22f, 0.25f});
        addPart({12.0f, -0.35f,-0.2f}, {13.2f, 0.35f, 0.5f}, {0.20f, 0.22f, 0.25f});
    } else if (renderState.active_weapon == WeaponID::HEGrenade) {
        // HE Grenade Viewmodel: Zeytin yeşili gövde + Kırmızı uyarı kuşağı + Metal mandal/pim
        addPart({ 4.5f, -1.8f, -3.5f}, { 8.5f, 1.8f,  0.8f}, {0.24f, 0.36f, 0.22f});
        addPart({ 4.4f, -1.85f,-1.2f}, { 8.6f, 1.85f,-0.2f}, {0.88f, 0.18f, 0.15f}); // Kırmızı Kuşak
        addPart({ 5.6f, -0.6f,  0.8f}, { 7.4f, 0.6f,  2.2f}, {0.65f, 0.68f, 0.72f}); // Fünye & Pim
    } else if (renderState.active_weapon == WeaponID::Flashbang) {
        // Flashbang Viewmodel: Çelik gri silindir + Parlak mavi kuşak + Delikli üst kapak
        addPart({ 4.8f, -1.5f, -3.8f}, { 8.2f, 1.5f,  1.0f}, {0.50f, 0.54f, 0.60f});
        addPart({ 4.7f, -1.55f,-1.5f}, { 8.3f, 1.55f,-0.4f}, {0.15f, 0.65f, 1.00f}); // Mavi Flashbang Kuşağı
        addPart({ 5.6f, -0.6f,  1.0f}, { 7.4f, 0.6f,  2.3f}, {0.75f, 0.78f, 0.82f});
    }

    glClear(GL_DEPTH_BUFFER_BIT);
    const float aspect = (m_height > 0) ? (static_cast<float>(m_width) / static_cast<float>(m_height)) : 1.333f;
    const Mat4 vmProj = Mat4::Perspective(68.0f, aspect, 0.5f, 200.0f);
    const Mat4 vmView = Mat4::SourceView(Vec3{0.0f, 0.0f, 0.0f}, Vec3{0.0f, 0.0f, 0.0f});
    const Mat4 vmMvp  = vmProj * vmView;

    GL33::UniformMatrix4fv(m_uMvpLocation, 1, GL_FALSE, vmMvp.m);
    GL33::BindVertexArray(m_dynVao);
    GL33::BindBuffer(GL_ARRAY_BUFFER, m_dynVbo);

    GL33::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vmTris.size() * sizeof(GLVertex)), vmTris.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vmTris.size()));

    GL33::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vmLines.size() * sizeof(GLVertex)), vmLines.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vmLines.size()));
}

void Engine::RenderHUD(const PlayerState& renderState) {
    glDisable(GL_DEPTH_TEST);
    const Mat4 identity = Mat4::Identity();
    GL33::UniformMatrix4fv(m_uMvpLocation, 1, GL_FALSE, identity.m);

    std::vector<GLVertex> hudLines;
    std::vector<GLVertex> hudTris;

    const float pxX = 1.0f / static_cast<float>(m_width);
    const float pxY = 1.0f / static_cast<float>(m_height);

    if (renderState.scope_level > 0) {
        const Vec3 blk{0.02f, 0.02f, 0.03f};
        MeshBuilder::AppendQuad(hudTris, {-1.0f, -1.0f, 0.0f}, {-0.58f, -1.0f, 0.0f}, {-0.58f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}, blk);
        MeshBuilder::AppendQuad(hudTris, { 0.58f, -1.0f, 0.0f}, { 1.0f, -1.0f, 0.0f}, { 1.0f,  1.0f, 0.0f}, { 0.58f, 1.0f, 0.0f}, blk);
        MeshBuilder::AppendQuad(hudTris, {-0.58f,  0.72f, 0.0f}, {0.58f,  0.72f, 0.0f}, {0.58f,  1.0f, 0.0f}, {-0.58f,  1.0f, 0.0f}, blk);
        MeshBuilder::AppendQuad(hudTris, {-0.58f, -1.0f, 0.0f}, {0.58f, -1.0f, 0.0f}, {0.58f, -0.72f, 0.0f}, {-0.58f, -0.72f, 0.0f}, blk);

        const float scopeBlurGap = (renderState.inaccuracy > 0.005f) ? (renderState.inaccuracy * 220.0f * pxX) : 0.0f;
        const Vec3 scopeLineCol  = (scopeBlurGap > 0.002f) ? Vec3{0.9f, 0.35f, 0.15f} : Vec3{0.0f, 0.0f, 0.0f};
        hudLines.push_back({{-1.0f, 0.0f, 0.0f}, scopeLineCol}); hudLines.push_back({{-scopeBlurGap, 0.0f, 0.0f}, scopeLineCol});
        hudLines.push_back({{ scopeBlurGap, 0.0f, 0.0f}, scopeLineCol}); hudLines.push_back({{ 1.0f, 0.0f, 0.0f}, scopeLineCol});
        hudLines.push_back({{0.0f, -1.0f, 0.0f}, scopeLineCol}); hudLines.push_back({{0.0f, -scopeBlurGap, 0.0f}, scopeLineCol});
        hudLines.push_back({{0.0f,  scopeBlurGap, 0.0f}, scopeLineCol}); hudLines.push_back({{0.0f,  1.0f, 0.0f}, scopeLineCol});

        MeshBuilder::AppendQuad(hudTris, {-2.5f*pxX, -2.5f*pxY, 0.0f}, {2.5f*pxX, -2.5f*pxY, 0.0f}, {2.5f*pxX, 2.5f*pxY, 0.0f}, {-2.5f*pxX, 2.5f*pxY, 0.0f}, {1.0f, 0.1f, 0.1f});
    } else {
        const float gapPx = 6.0f + renderState.inaccuracy * 450.0f;
        const float lenPx = 10.0f;

        const Vec3 chCol = (m_tracerTimer > 0.70f && m_lastShotHit) ?
            (m_lastHitGroup == HitGroup::Head ? Vec3{1.0f, 0.85f, 0.0f} : Vec3{1.0f, 0.2f, 0.2f}) :
            Vec3{0.0f, 1.0f, 0.35f};

        hudLines.push_back({{-(gapPx + lenPx) * pxX, 0.0f, 0.0f}, chCol}); hudLines.push_back({{-gapPx * pxX, 0.0f, 0.0f}, chCol});
        hudLines.push_back({{ gapPx * pxX, 0.0f, 0.0f}, chCol});           hudLines.push_back({{(gapPx + lenPx) * pxX, 0.0f, 0.0f}, chCol});
        hudLines.push_back({{0.0f, -(gapPx + lenPx) * pxY, 0.0f}, chCol}); hudLines.push_back({{0.0f, -gapPx * pxY, 0.0f}, chCol});
        hudLines.push_back({{0.0f,  gapPx * pxY, 0.0f}, chCol});           hudLines.push_back({{0.0f, (gapPx + lenPx) * pxY, 0.0f}, chCol});

        const float recoilOffsetX = -renderState.aim_punch_angle.y * 0.025f;
        const float recoilOffsetY = -renderState.aim_punch_angle.x * 0.025f;
        const Vec3  rcCol{1.0f, 0.85f, 0.15f};
        hudLines.push_back({{recoilOffsetX - 6.0f * pxX, recoilOffsetY, 0.0f}, rcCol});
        hudLines.push_back({{recoilOffsetX + 6.0f * pxX, recoilOffsetY, 0.0f}, rcCol});
        hudLines.push_back({{recoilOffsetX, recoilOffsetY - 6.0f * pxY, 0.0f}, rcCol});
        hudLines.push_back({{recoilOffsetX, recoilOffsetY + 6.0f * pxY, 0.0f}, rcCol});
    }

    const float hpRatio = std::clamp(m_botHealth / 100.0f, 0.0f, 1.0f);
    const Vec3  hpCol   = {1.0f - hpRatio * 0.8f, 0.15f + hpRatio * 0.75f, 0.2f};
    MeshBuilder::AppendQuad(hudTris, {-0.30f, 0.91f, 0.0f}, {0.30f, 0.91f, 0.0f}, {0.30f, 0.95f, 0.0f}, {-0.30f, 0.95f, 0.0f}, {0.15f, 0.16f, 0.20f});
    MeshBuilder::AppendQuad(hudTris, {-0.30f, 0.91f, 0.0f}, {-0.30f + 0.60f * hpRatio, 0.91f, 0.0f}, {-0.30f + 0.60f * hpRatio, 0.95f, 0.0f}, {-0.30f, 0.95f, 0.0f}, hpCol);

    const float speed2D   = renderState.velocity.Length2D();
    const float normSpeed = std::clamp(speed2D / 700.0f, 0.0f, 1.0f);
    const Vec3  barCol    = (speed2D > 255.0f) ? Vec3{1.0f, 0.45f, 0.1f} : Vec3{0.2f, 0.75f, 1.0f};
    MeshBuilder::AppendQuad(hudTris, {-0.4f, -0.94f, 0.0f}, {-0.4f + 0.8f * normSpeed, -0.94f, 0.0f}, {-0.4f + 0.8f * normSpeed, -0.90f, 0.0f}, {-0.4f, -0.90f, 0.0f}, barCol);

    char buf[180];
    std::snprintf(buf, sizeof(buf), "FPS: %.0f | 3D AUDIO: 48KHZ STREAM | PING(F2): %.0f MS", m_currentFps, m_simulatedRttMs);
    MeshBuilder::AppendText2D(hudTris, 20.0f, 18.0f, buf, 2.2f, {0.75f, 0.88f, 1.0f}, m_width, m_height);

    std::snprintf(buf, sizeof(buf), "BVH(F3): %zu NODES | XRAY(F4): %s | STEP MAT: %s",
                  m_world.bvhNodes.size(),
                  m_xrayHitboxes ? "ON" : "OFF",
                  (renderState.ground_material == MaterialType::Wood) ? "WOOD" :
                  ((renderState.ground_material == MaterialType::Metal) ? "METAL" : "CONCRETE"));
    MeshBuilder::AppendText2D(hudTris, 20.0f, 42.0f, buf, 2.0f, {0.55f, 0.72f, 0.85f}, m_width, m_height);

    std::snprintf(buf, sizeof(buf), "BOT HP: %.0f / 100 | KILLS: %u %s",
                  m_botHealth, m_botKills, (m_botBlindTimer > 0.0f) ? "[BLINDED!]" : "");
    const Vec3 botStatCol = (m_botBlindTimer > 0.0f) ? Vec3{0.3f, 0.95f, 1.0f} : Vec3{1.0f, 0.95f, 0.85f};
    MeshBuilder::AppendText2D(hudTris, static_cast<float>(m_width) * 0.5f - 210.0f, 56.0f, buf, 2.4f, botStatCol, m_width, m_height);

    if (m_totalHits > 0) {
        std::snprintf(buf, sizeof(buf), "LAST HIT: %s [%.0f DMG | WALLBANG: %u]",
                      Movement::GetHitGroupName(m_lastHitGroup), m_lastDamageDealt, m_lastPenetratedWalls);
        const Vec3 hitTextCol = (m_lastHitGroup == HitGroup::Head) ? Vec3{1.0f, 0.85f, 0.15f} : Vec3{0.25f, 1.0f, 0.45f};
        MeshBuilder::AppendText2D(hudTris, static_cast<float>(m_width) * 0.5f - 235.0f, 84.0f, buf, 2.1f, hitTextCol, m_width, m_height);
    }

    std::snprintf(buf, sizeof(buf), "SPEED: %.0f U/S %s", speed2D, (speed2D > 255.0f) ? "[BHOP/SURF!]" : "");
    MeshBuilder::AppendText2D(hudTris, 24.0f, static_cast<float>(m_height) - 82.0f, buf, 2.6f, barCol, m_width, m_height);

    const WeaponSpec& activeSpec = Movement::GetWeaponSpec(renderState.active_weapon);
    std::snprintf(buf, sizeof(buf), "EQUIPPED: %s", activeSpec.name);
    MeshBuilder::AppendText2D(hudTris, static_cast<float>(m_width) - 520.0f, static_cast<float>(m_height) - 82.0f, buf, 2.5f, {1.0f, 0.85f, 0.30f}, m_width, m_height);

    MeshBuilder::AppendText2D(hudTris, static_cast<float>(m_width) - 520.0f, static_cast<float>(m_height) - 52.0f,
                              "[1..3] GUNS  [4/G] HE GRENADE  [5/C] FLASHBANG", 1.85f, {0.65f, 0.75f, 0.85f}, m_width, m_height);

    GL33::BindVertexArray(m_dynVao);
    GL33::BindBuffer(GL_ARRAY_BUFFER, m_dynVbo);

    if (!hudTris.empty()) {
        GL33::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(hudTris.size() * sizeof(GLVertex)), hudTris.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(hudTris.size()));
    }
    if (!hudLines.empty()) {
        GL33::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(hudLines.size() * sizeof(GLVertex)), hudLines.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(hudLines.size()));
    }

    // Flashbang Beyaz Körlük Katmanı (GL_BLEND ile ekranı beyazlatır ve yavaşça açılır)
    if (m_flashAlpha > 0.01f) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        const float w = std::clamp(m_flashAlpha, 0.0f, 1.0f);
        std::vector<GLVertex> flashQuad;
        MeshBuilder::AppendQuad(flashQuad, {-1.0f, -1.0f, 0.0f}, {1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f}, {w, w, w});
        GL33::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(flashQuad.size() * sizeof(GLVertex)), flashQuad.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(flashQuad.size()));
        glDisable(GL_BLEND);
    }
}

void Engine::Render(float interp_alpha, double frame_dt) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const PlayerState renderState = Movement::InterpolateState(m_prevState, m_currState, interp_alpha);
    const Vec3 eyePos = renderState.origin + Vec3{0.0f, 0.0f, renderState.view_offset_z};

    // Prosedürel 3B Konumsal Ses Akışını Besle
    PumpPositionalAudio(renderState);

    Vec3 renderAngles = m_clientViewAngles + renderState.aim_punch_angle;
    renderAngles.x    = std::clamp(renderAngles.x, -89.0f, 89.0f);

    float verticalFov = 73.74f;
    if (renderState.scope_level == 1) verticalFov = 30.0f;
    if (renderState.scope_level == 2) verticalFov = 12.5f;

    const float aspect = (m_height > 0) ? (static_cast<float>(m_width) / static_cast<float>(m_height)) : 1.333f;
    const Mat4 proj = Mat4::Perspective(verticalFov, aspect, 1.0f, 10000.0f);
    const Mat4 view = Mat4::SourceView(eyePos, renderAngles);
    const Mat4 mvp  = proj * view;

    GL33::UseProgram(m_shaderProgram);
    GL33::UniformMatrix4fv(m_uMvpLocation, 1, GL_FALSE, mvp.m);

    // 1. Statik Dünya Fırçalarını Çiz
    GL33::BindVertexArray(m_worldVao);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_worldTriCount));
    glDrawArrays(GL_LINES, static_cast<GLsizei>(m_worldTriCount), static_cast<GLsizei>(m_worldLineCount));

    std::vector<GLVertex> dynLines;
    std::vector<GLVertex> dynTris;

    if (m_showBvhDebug) {
        for (const BVHNode& node : m_world.bvhNodes) {
            const Vec3 col = node.IsLeaf() ? Vec3{0.2f, 0.9f, 0.35f} : Vec3{0.55f, 0.25f, 0.85f};
            MeshBuilder::AppendWireBox(dynLines, node.mins, node.maxs, col);
        }
    }

    // 2. Bot Hitbox İskeleti
    MeshBuilder::AppendWireBox(
        dynLines,
        m_botServerOrigin + Movement::PLAYER_MINS,
        m_botServerOrigin + Movement::PLAYER_MAXS,
        Vec3{0.95f, 0.20f, 0.20f}
    );

    const float currentSimTime = static_cast<float>((m_currentTick + interp_alpha) * TICK_INTERVAL);
    const float clientVisTime  = std::max(0.0f, currentSimTime - (m_simulatedRttMs * 0.001f));
    Vec3 clientVisibleBotOrigin = m_botServerOrigin;
    if (!RewindBotLagRecord(clientVisTime, clientVisibleBotOrigin)) {
        clientVisibleBotOrigin = m_botServerOrigin;
    }
    MeshBuilder::AppendHitboxSkeleton(dynTris, dynLines, clientVisibleBotOrigin, true, (m_botBlindTimer > 0.0f) ? 1.35f : 1.0f);

    // 3. Havadaki Aktif El Bombalarını ve Patlama Şok Kürelerini Çiz
    for (const GrenadeProjectile& g : m_grenades) {
        if (!g.active) continue;
        const Vec3 gCol = (g.type == GrenadeType::HE) ? Vec3{1.0f, 0.25f, 0.15f} : Vec3{0.20f, 0.80f, 1.0f};
        MeshBuilder::AppendSolidBox(dynTris, g.origin + Movement::GRENADE_MINS, g.origin + Movement::GRENADE_MAXS, gCol);
        MeshBuilder::AppendWireBox(dynLines, g.origin + Movement::GRENADE_MINS, g.origin + Movement::GRENADE_MAXS, Vec3{1.0f, 1.0f, 1.0f});
    }

    for (const ExplosionVisual& ev : m_explosions) {
        if (!ev.active) continue;
        const float prog   = ev.timer / ev.maxDuration;
        const float radius = (ev.type == GrenadeType::HE) ? (prog * 340.0f) : (prog * 220.0f);
        const Vec3  col    = (ev.type == GrenadeType::HE) ?
                             Vec3{1.0f, 0.55f * (1.0f - prog), 0.1f} :
                             Vec3{0.8f, 0.95f, 1.0f} * (1.0f - prog);
        MeshBuilder::AppendWireSphere(dynLines, ev.origin, radius, col);
    }

    // 4. Elde Bomba Varken ([4] HE veya [5] Flashbang) CS2 Tarzı Canlı Sekme Yörüngesi Önizlemesi!
    const WeaponSpec& heldSpec = Movement::GetWeaponSpec(renderState.active_weapon);
    if (heldSpec.isGrenade) {
        const Uint32 mouseMask = SDL_GetMouseState(nullptr, nullptr);
        float previewSpeed = 720.0f;
        if ((mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) && (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT))) {
            previewSpeed = 490.0f;
        } else if (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            previewSpeed = 280.0f;
        }

        Vec3 fwd, right, up;
        Movement::AngleVectors(m_clientViewAngles, &fwd, &right, &up);

        GrenadeProjectile simProj{};
        simProj.active    = true;
        simProj.origin    = eyePos - Vec3{0.0f, 0.0f, 4.0f} + fwd * 18.0f;
        simProj.velocity  = Movement::ComputeGrenadeThrowVelocity(m_clientViewAngles, renderState.velocity, previewSpeed);
        simProj.fuseTimer = 1.55f;

        Vec3 prevPt = simProj.origin;
        for (int step = 0; step < 95 && simProj.fuseTimer > 0.0f; ++step) {
            Vec3 bouncePt{};
            const bool bounced = Movement::SimulateGrenadeStep(simProj, m_world, m_moveVars.gravity, 0.016f, &bouncePt, nullptr);
            dynLines.push_back({prevPt,         Vec3{0.25f, 1.0f, 0.55f}});
            dynLines.push_back({simProj.origin, Vec3{0.25f, 1.0f, 0.55f}});
            if (bounced) {
                MeshBuilder::AppendWireBox(dynLines, bouncePt - Vec3{3.5f, 3.5f, 3.5f}, bouncePt + Vec3{3.5f, 3.5f, 3.5f}, Vec3{1.0f, 0.9f, 0.2f});
            }
            prevPt = simProj.origin;
        }
        // Patlama noktası işareti
        MeshBuilder::AppendWireBox(dynLines, prevPt - Vec3{6.0f, 6.0f, 6.0f}, prevPt + Vec3{6.0f, 6.0f, 6.0f}, Vec3{1.0f, 0.25f, 0.2f});
    }

    // 5. Mermi Çıkartmaları ve Hitscan İzi
    for (const BulletDecal& d : m_decals) {
        if (!d.active) continue;
        const Vec3 col = d.hitBot ? Vec3{0.1f, 1.0f, 0.3f} :
                         (d.isExitPoint ? Vec3{0.75f, 0.25f, 1.0f} : Vec3{1.0f, 0.40f, 0.1f});
        const Vec3 p   = d.pos + d.normal * 0.5f;
        constexpr float s = 3.2f;
        dynLines.push_back({p - Vec3{s, 0.0f, 0.0f}, col}); dynLines.push_back({p + Vec3{s, 0.0f, 0.0f}, col});
        dynLines.push_back({p - Vec3{0.0f, s, 0.0f}, col}); dynLines.push_back({p + Vec3{0.0f, s, 0.0f}, col});
        dynLines.push_back({p - Vec3{0.0f, 0.0f, s}, col}); dynLines.push_back({p + Vec3{0.0f, 0.0f, s}, col});
    }

    if (m_tracerTimer > 0.0f) {
        std::vector<GLVertex> dummyTris;
        MeshBuilder::AppendHitboxSkeleton(dummyTris, dynLines, m_lastRewoundBoxOrigin, false, 1.25f);

        const Vec3 rayCol = m_lastShotHit ?
            (m_lastHitGroup == HitGroup::Head ? Vec3{1.0f, 0.9f, 0.1f} : Vec3{0.1f, 1.0f, 0.3f}) :
            Vec3{1.0f, 0.55f, 0.15f};
        dynLines.push_back({m_lastTraceStart, Vec3{1.0f, 0.95f, 0.2f}});
        dynLines.push_back({m_lastTraceEnd,   rayCol});
    }

    GL33::BindVertexArray(m_dynVao);
    GL33::BindBuffer(GL_ARRAY_BUFFER, m_dynVbo);

    if (!dynTris.empty()) {
        GL33::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(dynTris.size() * sizeof(GLVertex)), dynTris.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(dynTris.size()));
    }
    if (!dynLines.empty()) {
        GL33::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(dynLines.size() * sizeof(GLVertex)), dynLines.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(dynLines.size()));
    }

    if (m_xrayHitboxes) {
        glDisable(GL_DEPTH_TEST);
        std::vector<GLVertex> xrayTris;
        std::vector<GLVertex> xrayLines;
        MeshBuilder::AppendHitboxSkeleton(xrayTris, xrayLines, clientVisibleBotOrigin, false, 0.65f);
        GL33::BufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(xrayLines.size() * sizeof(GLVertex)), xrayLines.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(xrayLines.size()));
        glEnable(GL_DEPTH_TEST);
    }

    RenderViewmodel(renderState, frame_dt);
    RenderHUD(renderState);

    GL33::BindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    SDL_GL_SwapWindow(m_window);
}

void Engine::UpdateWindowTelemetry(double frame_dt) {
    const auto fdt = static_cast<float>(frame_dt);
    if (m_tracerTimer > 0.0f) {
        m_tracerTimer = std::max(0.0f, m_tracerTimer - fdt);
    }
    if (m_flashAlpha > 0.0f) {
        m_flashAlpha = std::max(0.0f, m_flashAlpha - fdt * 0.42f);
    }
    if (m_tinnitusTimer > 0.0f) {
        m_tinnitusTimer = std::max(0.0f, m_tinnitusTimer - fdt);
    }
    for (ExplosionVisual& ev : m_explosions) {
        if (!ev.active) continue;
        ev.timer += fdt;
        if (ev.timer >= ev.maxDuration) ev.active = false;
    }

    static double timer = 0.0;
    static int frames = 0;
    timer += frame_dt;
    frames++;

    if (timer >= 0.1) {
        m_currentFps = frames / timer;
        const WeaponSpec& spec = Movement::GetWeaponSpec(m_currState.active_weapon);
        char titleBuf[320];
        std::snprintf(
            titleBuf, sizeof(titleBuf),
            "CS Core [%s] | FPS:%.0f | BotHP:%.0f (Kills:%u) | [G] HE [C] Flash [4/5] Trajectory Preview",
            spec.name,
            m_currentFps,
            m_botHealth,
            m_botKills,
            m_totalHits
        );
        SDL_SetWindowTitle(m_window, titleBuf);
        timer  = 0.0;
        frames = 0;
    }
}

void Engine::Run() {
    uint64_t prevCounter = SDL_GetPerformanceCounter();
    const double perfFreq = static_cast<double>(SDL_GetPerformanceFrequency());
    double accumulator = 0.0;
    double totalTime   = 0.0;

    while (m_running) {
        const uint64_t nowCounter = SDL_GetPerformanceCounter();
        double frameTime = static_cast<double>(nowCounter - prevCounter) / perfFreq;
        prevCounter = nowCounter;

        if (frameTime > 0.25) frameTime = 0.25;

        accumulator += frameTime;
        totalTime   += frameTime;

        const double currentSubtick = std::clamp(accumulator / TICK_INTERVAL, 0.0, 0.9999);
        PollEvents(currentSubtick);

        while (accumulator >= TICK_INTERVAL) {
            TickUpdate(currentSubtick);
            accumulator -= TICK_INTERVAL;
        }

        ProcessNetworkQueues(totalTime);

        const float alpha = static_cast<float>(accumulator / TICK_INTERVAL);
        Render(alpha, frameTime);
        UpdateWindowTelemetry(frameTime);
    }
}