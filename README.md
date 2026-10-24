# fps-oyun-motoru(optimizasyoni iyi değil açık kaynaklı bir koddur)
  ===KULANIM KULAVUZU===
  
  TR
  
          CS COMPETITIVE FPS ENGINE CORE - GELISTIRICI VE KULLANIM KLAVUZU

1. MİMARİ GENEL BAKIŞ VE DOSYA DÜZENİ

Motor; harici oyun motoru şişkinliklerinden arındırılmış, sıfır dinamik bellek 
tahsisi (zero heap allocation) ve önbellek dostu (Data-Oriented POD) prensiplerle 
C++20, SDL2 ve OpenGL 3.3 Core kullanılarak tasarlanmıştır.

fps_core/
├── CMakeLists.txt        - Derleyici, OpenGL ve SDL2 bağlayıcısı
├── Movement.h            - Vektör matematiği, Fırçalar, Hitbox'lar ve Balistik bildirimleri
├── Movement.cpp          - Kinematik hareket, BVH Trace, Duvar Delme ve Silah fiziği
├── Engine.h              - Oyun döngüsü, UDP soketleri, Ses Sentezleyici ve Viewmodel
├── Engine.cpp            - OpenGL render, 3D Ses DSP mikseri, HUD ve Olay döngüsü
└── main.cpp              - Minimal giriş noktası (Entry point)


2. TEMEL ÇEKİRDEK MODÜLLER VE ÇALIŞMA MANTIĞI


A. Sabit Zaman Döngüsü (Fixed-Timestep Tick System)
- Fizik simülasyonu render kare hızından (FPS) tamamen izole edilmiş 128 Hz 
  (TICK_INTERVAL = 1.0 / 128.0 sn) sabit adımlarla ilerler.
- İnterpolasyon (InterpolateState): Yüksek kare hızında (örn. 240 FPS) akıcı görüntü 
  elde etmek için, oyuncunun konumu önceki tick (prevState) ile mevcut tick (currState) 
  arasında doğrusal olarak enterpole edilir.

B. Quake / Source Kinematik Hareket ve Çarpışma
- Minkowski Genişletmeli TraceBox: Standart rigidbody fiziği kullanılmaz. Oyuncunun 
  eksen hizalı kutusu (AABB), yarı boyutları kadar fırçaları dışarı doğru genişletir.
- StepSlideMove & Surf: 18 birimlik stepsize sınırı ile merdivenler takılmadan çıkılır. 
  Eğimli yüzeylerde yer sürtünmesi devre dışı kalır ve gerçek Source Surf fiziği sağlanır.

C. Düzleştirilmiş BVH Uzamsal Hızlandırma Ağacı (Linearized BVH)
- Her BVH düğümü tam 32 bayt (alignas(32) BVHNode) boyutundadır.
- Uzamsal Budama (Pruning): Sahnede çok sayıda fırça/sütun olsa bile, yığın tabanlı 
  hızlı BVH gezintisi sayesinde ışın izleme ve kutu süpürme işlemleri yalnızca ilgili 
  fırçaları test eder.

D. Deterministik Silah Geri Tepmesi ve Dağılım (Recoil & Seeded Spread)
- AimPunch ve Sprey: Ateş edildiğinde AK-47 sprey tablosundan okunan açısal darbe 
  hıza eklenir. Yay-sönümleme (spring-damper) fiziği namluyu merkeze çeker.
- Tohumlanmış Dağılım (Seeded Spread): Ağa ek paket gönderilmeden, istemci ve sunucu 
  aynı command_number değerini girdi alan deterministik bit karıştırıcı (SplitMix32) 
  ile birebir aynı mermi dağılımını üretir.

E. Bölgesel Hitbox İskeleti ve Duvar Delme (Multi-Hitbox & Wallbang)
- Anatomik Kapsüller: Botun vurulma hacmi 5 parçalı silindirik kapsül/kutudan oluşur: 
  Baş (4.0x Hasar), Göğüs (1.0x), Karın (1.25x) ve Bacaklar (0.75x - Zırhsız).
- TraceToExit ve Wallbang: Mermi duvara çarptığında materyali okunur. Kalıcı enerji 
  yeterliyse mermi duvarı delerek arkadaki botu vurabilir.

F. Prosedürel 3B Konumsal Ses Sentezleyicisi (SDL_AudioStream DSP)
- Harici Dosyasız Ses: SDL_AudioStream üzerinden 48000 Hz frekansta çalışan gerçek 
  zamanlı DSP motorudur. Ayak sesleri zemin materyaline göre ayrı dalga boylarıyla sentezlenir.
- BVH Duvar Sönümlemesi (Occlusion): Kaynak ile kulak arasında katı duvar varsa ses 
  filtrelenerek (Low-Pass Filter) boğuklaştırılır.

G. El Bombası Fiziği ve Canlı Yörünge Önizlemesi (HE & Flashbang)
- Trajectory Preview: Elde bomba varken (4 veya 5) sahneye fırlatılacak bombanın sekme 
  noktalarını gösteren canlı 3B yeşil yörünge çizgisi çizilir.
- Anti-Flash Körlük: Flashbang patladığında kameranın bakış açısı test edilir; patlamaya 
  doğrudan bakıyorsan beyaz ekran ve kulak çınlaması (Tinnitus) yaşarsın.


3. OYUN İÇİ KONTROLLER VE TUŞ KISAYOLARI

- W, A, S, D          : Quake / Source kinematik koşu ve hava kontrolü (Air-Strafe)
- Space / MWheel      : Bunny Hop (Zıplama)
- Sol Ctrl            : Çömelme (Duck - Inaccuracy değerini azaltır)
- Sol Tık (LMB)       : Ateş etme / El bombası uzun mesafe fırlatma (720 u/s)
- Sağ Tık (RMB)       : AWP dürbün açma (2.5x / 6.0x) / El bombası kısa bırakma (280 u/s)
- Sol + Sağ Tık       : El bombası orta mesafe fırlatma (490 u/s)
- 1 / 2 / 3           : Silah seçimi (1: AK-47, 2: Desert Eagle, 3: AWP Sniper)
- 4 / 5               : Bomba seçimi (4: HE Grenade, 5: Flashbang)
- G / C               : Hızlı el bombası fırlatma (G: HE, C: Flashbang)
- R                   : Duvardaki mermi ve wallbang çıkartmalarını temizleme
- F1                  : AutoBhop aç/kapat
- F2                  : Simüle edilen UDP ağ gecikmesini değiştir (0 ms -> 100 ms -> 200 ms)
- F3                  : Düzleştirilmiş BVH uzamsal ağaç düğümlerini göster/gizle
- F4                  : Duvar arkasındaki hitbox iskeletini (X-Ray) aç/kapat
- ESC                 : Fare imleci serbest bırakma ve güvenli çıkış


EN



         CS COMPETITIVE FPS ENGINE CORE - DEVELOPER & USER MANUAL


1. ARCHITECTURAL OVERVIEW & FILE STRUCTURE

The engine is designed without external game engine bloat, using zero heap 
allocation and cache-friendly (Data-Oriented POD) principles with C++20, SDL2, 
and OpenGL 3.3 Core.

fps_core/
├── CMakeLists.txt        - Compiler, OpenGL, and SDL2 linker
├── Movement.h            - Vector math, Brushes, Hitboxes, and Ballistics declarations
├── Movement.cpp          - Kinematic movement, BVH Trace, Wallbang, and Weapon physics
├── Engine.h              - Game loop, UDP sockets, Sound Synthesizer, and Viewmodel
├── Engine.cpp            - OpenGL render, 3D Audio DSP mixer, HUD, and Event loop
└── main.cpp              - Minimal entry point


2. CORE MODULES & WORKING MECHANICS


A. Fixed-Timestep Tick System
- Physics simulation runs at a fixed 128 Hz (TICK_INTERVAL = 1.0 / 128.0 sec), 
  completely isolated from render framerate (FPS).
- Interpolation (InterpolateState): For smooth rendering at high framerates (e.g., 240 FPS), 
  player position is linearly interpolated between the previous tick (prevState) and current tick (currState).

B. Quake / Source Kinematic Movement & Collision
- Minkowski Expanded TraceBox: Standard rigidbody physics is not used. The player's axis-aligned 
  bounding box (AABB) expands brushes outward by its half-extents.
- StepSlideMove & Surf: 18-unit stepsize allows climbing stairs without sticking. 
  Surface friction is disabled on slopes below the walkable threshold, producing authentic Source Surf physics.

C. Linearized BVH Spatial Acceleration Tree
- Each BVH node is exactly 32 bytes (alignas(32) BVHNode).
- Spatial Pruning: Even with many brushes/pillars in the scene, stack-based fast BVH traversal 
  ensures raycasting and box sweeping only test relevant brushes.

D. Deterministic Weapon Recoil & Seeded Spread
- AimPunch & Spray: Firing adds angular kick to aim_punch_angle_vel from the AK-47 spray table. 
  Spring-damper physics pull the barrel back to center.
- Seeded Spread: Without extra network packets, client and server produce bit-identical spread 
  using a deterministic bit mixer (SplitMix32) keyed on command_number.

E. Multi-Hitbox Skeleton & Wallbang Penetration
- Anatomical Capsules: Bot collision volume consists of 5 cylindrical capsules/boxes: 
  Head (4.0x Damage), Chest (1.0x), Stomach (1.25x), and Legs (0.75x - Unarmored).
- TraceToExit & Wallbang: When a bullet hits a brush, its material is read. If remaining energy 
  is sufficient, the bullet penetrates the wall to hit targets behind it.

F. Procedural 3D Spatial Audio Synthesizer (SDL_AudioStream DSP)
- Asset-Free Audio: Real-time DSP engine running via SDL_AudioStream at 48000 Hz. 
  Footsteps are synthesized with distinct waveforms depending on floor material.
- BVH Wall Occlusion: Low-pass filtering muffles sound and attenuates volume if a solid wall 
  exists between the audio source and the player's ear.

G. Grenade Physics & Live Trajectory Preview
- Trajectory Preview: Holding a grenade (4 or 5) draws a live 3D green trajectory line showing bounce points.
- Anti-Flash Blindness: Flashbang detonation tests camera view angles; looking directly at the flash 
  causes full white-out and tinnitus, whereas turning away results in a minor glare. Bot blinds for 3.5 seconds.


3. IN-GAME CONTROLS & SHORTCUTS

- W, A, S, D          : Quake / Source kinematic movement and Air-Strafe
- Space / MWheel      : Bunny Hop (Jumping)
- Left Ctrl           : Duck (Crouching - reduces inaccuracy)
- Left Click (LMB)    : Shoot / Throw grenade long-range (720 u/s)
- Right Click (RMB)   : AWP scope toggle (2.5x / 6.0x) / Throw grenade short-range (280 u/s)
- Left + Right Click  : Throw grenade mid-range (490 u/s)
- 1 / 2 / 3           : Weapon selection (1: AK-47, 2: Desert Eagle, 3: AWP Sniper)
- 4 / 5               : Grenade selection (4: HE Grenade, 5: Flashbang)
- G / C               : Quick grenade throw (G: HE, C: Flashbang)
- R                   : Clear bullet and wallbang impacts (BulletDecal)
- F1                  : AutoBhop toggle
- F2                  : Change simulated UDP network ping (0 ms -> 100 ms -> 200 ms)
- F3                  : Show/hide linearized BVH spatial tree nodes
- F4                  : Toggle hitbox skeleton X-Ray behind walls
- ESC                 : Release mouse cursor and safely exit

================================================================================
                        KURULUM KLAVUZU / INSTALLATION GUIDE
================================================================================

/*/*/ Bu bir deneme sürümü oyun motorudur; isteyen herkes kendi FPS oyununu 
tasarlamak için kullanabilir. Neredeyse her şey mevcuttur ve Yapay Zekâ (YZ) 
tarafından anlaşılıp okunabilir hale getirilmiştir. /*/*/


[TR] TÜRKÇE
--------------------------------------------------------------------------------
Oyun programıymış gibi incelemek ve çalıştırmak için aşağıdaki adımları 
takip edebilirsiniz:

1) https://www.msys2.org/ adresinden yükleyiciyi (.exe) indirin ve kurun.
2) UCRT terminalini açın ve SDL2 paketini kurmak için aşağıdaki kodu yazın 
   (Eğer sisteminizde zaten kuruluysa bu aşamayı es geçebilirsiniz):
   
   pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-SDL2

3) Komut satırında 'cd' komutu ile proje klasörünün konumuna gidin. Örnek:
   
   cd /c/Users/Can/Desktop/fps_core

4) Doğru yerde olup olmadığınızı kontrol etmek için şunu yazın:
   
   ls

5) Eğer doğru 'fps_core' dizinindeyseniz, projeyi derlemek için sırasıyla 
   şu kodları girin:
   
   mkdir -p build && cd build
   cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
   cmake --build .

6) Bu işlem derlemeyi tamamlayacak ve çalıştırılabilir (.exe) dosyayı 
   oluşturacaktır. Oyunu fırlatmak için son olarak şunu yazın:
   
   ./fps_core.exe


*** Daha Sonra Tekrar Açmak İçin:
1) 'cd' komutu ile dosyanın bulunduğu konuma tekrar gidin:
   
   cd /c/Users/Can/Desktop/fps_core

2) Doğrudan çalıştırın:
   
   ./fps_core.exe



[EN] ENGLISH
--------------------------------------------------------------------------------
This is a trial version of a game engine; anyone who wants can use it to design 
their own FPS game. It features almost everything and has been made readable and 
understandable by AI.

To run it and check everything out as if it were a finished game program, 
follow these steps:

1) Download and install the .exe installer from https://www.msys2.org/
2) Open UCRT and install SDL2 by typing the command below. 
   (If it is already installed on your system, skip this step):
   
   pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-SDL2

3) Navigate to the project folder location using the 'cd' command. Example:
   
   cd /c/Users/Can/Desktop/fps_core

4) Type 'ls' to check if you are in the right directory.

5) If you are in the 'fps_core' directory, enter the following commands to build:
   
   mkdir -p build && cd build
   cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
   cmake --build .

6) This will complete the build and generate the executable (.exe) file. 
   To launch the game, finally type:
   
   ./fps_core.exe


*** To Open It Again Later:
1) Navigate back to the folder directory using 'cd':
   
   cd /c/Users/Can/Desktop/fps_core

2) Launch directly:
   
   ./fps_core.exe



