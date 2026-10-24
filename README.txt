IIIIIIII  IIIII
   II     II  II
   II     IIIII
   II     II II
   II     II  II
   II     II   II


/*/*/Bir deneme sürümüdür oyum motorudur isteyen kulanip kendi fps oynunu tasarlayabilir çoğunlukla her şey var ve YZ tarafından anlaşılıp okunabilir hale getirilmiştir./*/*/
***Bnu oyun programıymış gibi ne var ne yok bakıp çalıştırmak için indirmeniz gereken bazı şeyler vardır:
1}https://www.msys2.org/ exeyi yükleyin ve kurun
2}UCRT yi açın ve aşağıdaki kodu yazarak SDL2 yi kurun kuruluysa bu aşamayı es geçin.
	pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-SDL2
3}Dosyanın yüklü olduğu konumu cd ile yönlendirin örneğin:
	cd /c/Users/Can/Desktop/fps_core
4}Kontrol için ls yazın
5}Eğer dosya fps_core yönlendiriyorsa ardından şu kodu girin:
	mkdir -p build && cd build
	cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
	cmake --build .
6}Bu buildi ve exeyi kurucak sadece exeyi fırlatmak kaldı son olarak şunu yazın:
	./fps_core.exe
***Eğer tekrar açmak istiyorsanız şunu izleyin:
1}Dosyanın yüklü olduğu konumu cd ile yönlendirin örneğin:
	cd /c/Users/Can/Desktop/fps_core
2}Sonra:
	./fps_core.exe


IIIIIIII   II      II
II         IIII    II
II         II II   II
IIIIIIII   II  II  II
II         II   II II
II	   II    IIII
IIIIIIII   II      II


/*/*/ This is a trial version of a game engine; anyone who wants can use it to design their own FPS game. It features almost everything and has been made readable and understandable by AI. /*/*/
*** To run it and check everything out as if it were a finished game program, there are a few things you need to download and set up:
1} Download and install the .exe from https://www.msys2.org/
2} Open UCRT and install SDL2 by typing the code below. If it is already installed, skip this step.
	pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-SDL2
3} Navigate to the folder where the files are located using 'cd', for example:
	cd /c/Users/Can/Desktop/fps_core
4} Type 'ls' to check.
5} If you are in the fps_core directory, enter the following code:
	mkdir -p build && cd build
	cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
	cmake --build .
6} This will handle the build and create the executable (.exe). All that's left is to launch it; finally, type:
	./fps_core.exe
*** If you want to open it again later, follow these steps:
1} Navigate to the directory where the files are located using 'cd', for example:
	cd /c/Users/Can/Desktop/fps_core
2} Then:
	./fps_core.exe