APPNAME = game

CC = g++

src = $(wildcard src/*.cpp)
obj = $(src:.cpp=.o)

DYNAMIC = true
DEBUG = true

BUILDDIR=./build
MACDIR=./deps/MacOS
WINDIR=./deps/Windows
LNXDIR=./deps/Linux

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
    INCDIR += -I$(WINDIR)/include
    LIBDIR += -L$(WINDIR)/lib
	ifneq ($(DEBUG), true)
        CXXFLAGS+=-w -Wl,-subsystem,windows
	endif
    LDFLAGS := -lmingw32 -lSDL3 -lphysfs
	ifeq ($(DYNAMIC), true)
        #LDFLAGS += -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic
        # Above links statically to only the necessary things
        # Uncomment it if your compiler isn't incompetent and makes the file size 13mb larger.
	else
        LDFLAGS += -static -lkernel32 -ladvapi32 -lgdi32 -limm32 -lmsvcrt -lole32 -loleaut32 -lsetupapi -lshell32 -luser32 -lversion -lwinmm
         # Above links everything statically
	endif
else ifeq ($(uname_S), Darwin)
    #LIBGL = -framework OpenGL -framework GLUT
    build_target := all_mac
    STD := -std=gnu17
    INCDIR += -I$(MACDIR)/include
    LIBDIR += -L$(MACDIR)/lib
    LDFLAGS := -F$(MACDIR)/Frameworks -Wl,-rpath,@executable_path/../Frameworks -framework SDL2
    LDFLAGS += ./deps/MacOS/lib/libphysfs.a # Static link to physfs, installed with brew
    LDFLAGS += -Wl,-framework,CoreFoundation -Wl,-framework,IOKit -Wl,-framework,Foundation # Frameworks for physfs
    CXXFLAGS += -mmacosx-version-min=10.9 # target MacOS 10.9
else ifeq ($(uname_S), Linux)
    #LIBGL = -lGL -lglut
    build_target := all_linux
    STD := -std=gnu17
    LDFLAGS := $(shell sdl2-config --libs)  -lphysfs
    CXXFLAGS += $(shell sdl2-config --cflags) -no-pie
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
		cp $(WINDIR)/lib/*.dll $(BUILDDIR)
    endif
	cp -R ./Assets $(BUILDDIR)

linux: 
	cp -R ./Assets $(BUILDDIR)

$(BUILDDIR)/$(APPNAME): $(obj)
	$(CC) $(STD) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILDDIR):
	rm -rf $(BUILDDIR)
	mkdir $(BUILDDIR)

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
	rm -f -R $(obj) $(BUILDDIR)