APPNAME = game

CC = g++
CC_C = gcc

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
src := $(call rwildcard, src/, *.cpp)
obj = $(src:.cpp=.o)

# Used on Windows to build physfs.dll straight from the vendored source, instead of linking a
# prebuilt one (see the Windows branch below) — same source the CMake build already compiles
# from. Uses CC_C (gcc), not CC (g++) — g++ compiles .c files as C++ regardless of extension,
# which isn't what we want for third-party C source.
physfs_src := $(call rwildcard, deps/include/PhysFS/, *.c)
physfs_obj = $(physfs_src:.c=.o)

DYNAMIC = true
DEBUG = true

BUILDDIR=./build
MACDIR=./deps/MacOS
LNXDIR=./deps/Linux
MINGWBINDIR=C:/msys64/mingw64/bin

INCDIR = -I./deps/include
LIBDIR =

CXXFLAGS = -Wall
LIBGL =
# development (3), and production (0)
ifeq ($(DEBUG), true)
    DEBUGFLAGS := -g3
endif
# Optimizations
OPTFLAGS := -O3

CXXFLAGS += $(DEBUGFLAGS) $(OPTFLAGS)

uname_S =

ifeq ($(OS),Windows_NT)
    uname_S := Windows
else
    uname_S := $(shell uname -s)
endif


ifeq ($(uname_S), Windows)
    #LIBGL = -lGL -lglut
    build_target := all_windows
    STD := -std=c17
    # SDL3/SDL3_image come from MSYS2's mingw-w64 packages (pacman -S mingw-w64-x86_64-sdl3
    # mingw-w64-x86_64-sdl3-image) — g++ already searches its own mingw64/include by default,
    # so no -I is needed for them. PhysFS has no MSYS2 package, so instead of a prebuilt DLL
    # it's built straight into $(BUILDDIR) from the vendored source (see
    # $(BUILDDIR)/libphysfs.dll below) and linked from there. Nothing under deps/Windows/ is
    # vendored anymore — the `windows:` target below pulls SDL3/SDL3_image and the MinGW
    # runtime DLLs (libgcc_s_seh-1/libstdc++-6/libwinpthread-1) straight from $(MINGWBINDIR).
    LIBDIR += -L$(BUILDDIR)
	ifeq ($(DYNAMIC), true)
        PHYSFS_LIB_DEP := $(BUILDDIR)/libphysfs.dll
	else
        PHYSFS_LIB_DEP := $(BUILDDIR)/libphysfs.a
	endif
	ifneq ($(DEBUG), true)
        CXXFLAGS+=-w -Wl,-subsystem,windows
	endif
    LDFLAGS := -lmingw32 -lSDL3 -lSDL3_image -lphysfs
	ifeq ($(DYNAMIC), true)
        #LDFLAGS += -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic
        # Above links statically to only the necessary things
        # Uncomment it if your compiler isn't incompetent and makes the file size 13mb larger.
	else
        LDFLAGS += -static -lkernel32 -ladvapi32 -lgdi32 -limm32 -lmsvcrt -lole32 -loleaut32 -lsetupapi -lshell32 -luser32 -lversion -lwinmm -luuid
         # Above links everything statically
	endif
else ifeq ($(uname_S), Darwin)
    #LIBGL = -framework OpenGL -framework GLUT
    build_target := all_mac
    STD := -std=gnu17
    INCDIR += -I$(MACDIR)/include
    LIBDIR += -L$(MACDIR)/lib
    LDFLAGS := -F$(MACDIR)/Frameworks -Wl,-rpath,@executable_path/../Frameworks -framework SDL3
    LDFLAGS += ./deps/MacOS/lib/libphysfs.a # Static link to physfs, installed with brew
    LDFLAGS += -Wl,-framework,CoreFoundation -Wl,-framework,IOKit -Wl,-framework,Foundation # Frameworks for physfs
    CXXFLAGS += -mmacosx-version-min=10.9 # target MacOS 10.9
else ifeq ($(uname_S), Linux)
    #LIBGL = -lGL -lglut
    build_target := all_linux
    STD := -std=gnu++17
    LDFLAGS := /usr/local/lib/libSDL3.a /usr/local/lib/libphysfs.a -ldl -pthread
    CXXFLAGS += /usr/local/lib/libSDL3.a -no-pie -ldl -pthread
endif

LDFLAGS += $(LIBGL)
CXXFLAGS += $(INCDIR)
CXXFLAGS += $(LIBDIR)

# Mac app bundle variables
APPBUNDLE=$(BUILDDIR)/$(APPNAME).app
APPBUNDLECONTENTS=$(APPBUNDLE)/Contents
APPBUNDLEMACOS=$(APPBUNDLECONTENTS)/MacOS
APPBUNDLERESOURCES=$(APPBUNDLECONTENTS)/Resources

.PHONY: clean $(BUILDDIR) osxapp mytarget all_mac all_windows all_linux mac windows linux

mytarget: $(build_target)

all_mac: $(BUILDDIR) $(BUILDDIR)/$(APPNAME) mac

all_windows: $(BUILDDIR) $(BUILDDIR)/$(APPNAME) windows

all_linux: $(BUILDDIR) $(BUILDDIR)/$(APPNAME) linux

mac: osxapp

windows:
    ifeq ($(DYNAMIC), true)
		cp $(MINGWBINDIR)/SDL3.dll $(MINGWBINDIR)/SDL3_image.dll $(BUILDDIR)
		cp $(MINGWBINDIR)/libgcc_s_seh-1.dll $(MINGWBINDIR)/libstdc++-6.dll $(MINGWBINDIR)/libwinpthread-1.dll $(BUILDDIR)
    endif
	cp -R ./Assets $(BUILDDIR)

linux: 
	cp -R ./Assets $(BUILDDIR)

$(BUILDDIR)/$(APPNAME): $(obj) | $(PHYSFS_LIB_DEP)
	$(CC) $(STD) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILDDIR):
	rm -rf $(BUILDDIR)
	mkdir $(BUILDDIR)

deps/include/PhysFS/%.o: deps/include/PhysFS/%.c
	$(CC_C) -O3 -I./deps/include/PhysFS -c -o $@ $<

# Built from the vendored source instead of linking a prebuilt copy — see the Windows branch
# above. -Wl,--export-all-symbols: physfs.h's PHYSFS_DECL only requests __declspec(dllexport)
# for MSVC, so on MinGW this makes the linker export every symbol instead (what a DLL with no
# explicit exports would do by default anyway; made explicit here).
$(BUILDDIR)/libphysfs.dll: $(physfs_obj) | $(BUILDDIR)
	$(CC_C) -shared -o $@ $^ -Wl,--export-all-symbols -Wl,--out-implib,$(BUILDDIR)/libphysfs.dll.a

$(BUILDDIR)/libphysfs.a: $(physfs_obj) | $(BUILDDIR)
	ar rcs $@ $^

osxapp: $(MACDIR)/$(APPNAME).icns $(BUILDDIR) $(BUILDDIR)/$(APPNAME)
	rm -rf $(APPBUNDLE)
	mkdir $(APPBUNDLE)
	mkdir $(APPBUNDLECONTENTS)
	mkdir $(APPBUNDLEMACOS)
	mkdir $(APPBUNDLERESOURCES)
	cp -R ./Assets $(APPBUNDLERESOURCES)
	cp -R $(MACDIR)/Frameworks $(APPBUNDLECONTENTS)
	cp $(MACDIR)/Info.plist $(APPBUNDLECONTENTS)/Info.plist
	cp $(MACDIR)/PkgInfo $(APPBUNDLECONTENTS)/
	cp $(MACDIR)/$(APPNAME).icns $(APPBUNDLERESOURCES)/
	rm -r $(MACDIR)/$(APPNAME).icns
	cp $(BUILDDIR)/$(APPNAME) $(APPBUNDLEMACOS)/
	rm -f $(BUILDDIR)/$(APPNAME)

$(MACDIR)/$(APPNAME).icns: $(MACDIR)/$(APPNAME).png
	rm -rf $(APPNAME).iconset
	mkdir $(APPNAME).iconset
	sips -z 16 16   $(MACDIR)/$(APPNAME).png --out $(APPNAME).iconset/icon_16x16.png
	sips -z 32 32   $(MACDIR)/$(APPNAME).png --out $(APPNAME).iconset/icon_16x16@2x.png
	sips -z 32 32   $(MACDIR)/$(APPNAME).png --out $(APPNAME).iconset/icon_32x32.png
	sips -z 64 64   $(MACDIR)/$(APPNAME).png --out $(APPNAME).iconset/icon_32x32@2x.png
	sips -z 128 128 $(MACDIR)/$(APPNAME).png --out $(APPNAME).iconset/icon_128x128.png
	sips -z 256 256 $(MACDIR)/$(APPNAME).png --out $(APPNAME).iconset/icon_128x128@2x.png
	sips -z 256 256 $(MACDIR)/$(APPNAME).png --out $(APPNAME).iconset/icon_256x256.png
	sips -z 512 512 $(MACDIR)/$(APPNAME).png --out $(APPNAME).iconset/icon_256x256@2x.png
	sips -z 512 512 $(MACDIR)/$(APPNAME).png --out $(APPNAME).iconset/icon_512x512.png
	cp $(MACDIR)/$(APPNAME).png $(APPNAME).iconset/icon_512x512@2x.png
	iconutil -c icns -o $(MACDIR)/$(APPNAME).icns $(APPNAME).iconset
	rm -r $(APPNAME).iconset

# HANDHELDS:

switch:
	make -f Switch.mk

vita:
	make -f Vita.mk

clean:
	rm -f -R $(obj) $(physfs_obj) $(BUILDDIR)