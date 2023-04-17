![License](https://img.shields.io/badge/License-GPLv3-blue.svg) [![GitHub version](https://badge.fury.io/gh/nadrino%2FSimpleModManager.svg)](https://github.com/nadrino/SimpleModManager/releases/) [![Github all releases](https://img.shields.io/github/downloads/nadrino/SimpleModManager/total.svg)](https://GitHub.com/nadrino/SimpleModManager/releases/)

# SimpleModManager
SimpleModManager is an homebrew app for the Nintendo Switch CFW : Atmosphere. It allows to manage your mods (via LayeredFS).

<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/src/Applications/SimpleModManager/resources/assets/icon_gui.jpg"></p>

## Usage
- Place the .nro file in the `/switch/` folder of your SDcard.
- At the root of your SDcard, create a `/mods/` folder.
- Tree structure : `/mods/<NameOfTheGame>/<NameOfTheMod>/<ModTreeStructureFromAtmosphereFolder>`
- For plugins: `/mods/<NameOfTheGame>/.plugins/<NameOfTheNro>.smm`

Example : `/mods/The Legend of Zelda - Breath of the Wild/First Person View/contents/01007EF00011E000/romfs/Actor/Pack/GameRomCamera.sbactorpack`

## Plugins
Plugins can be any hbmenu nro, but should be linked against [libsmm](https://github.com/withertech/libsmm) and have the
```c++
void smmInit();
```
called in initialization and
```c++
void smmExit();
```
called in deinitialization

use
```c++
std::string smmModPathForCfwPath(std::string path);
```
to get the path to a file under `sdmc:/mods/...` from a path to a file under `sdmc:/atmosphere/...`

and include
```c++
#import <libsmm.h>
```
and add
```makefile
LIBS	:= -lsmm -lnx
```
to your makefile

Example :

<details>
  <summary>main.cpp</summary>

```c++

// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include the main libnx system header, for Switch development
#include <switch.h>

#include <libsmm.h>

// Main program entrypoint
int main(int argc, char* argv[])
{
    // This example uses a text console, as a simple way to output text to the screen.
    // If you want to write a software-rendered graphics application,
    //   take a look at the graphics/simplegfx example, which uses the libnx Framebuffer API instead.
    // If on the other hand you want to write an OpenGL based application,
    //   take a look at the graphics/opengl set of examples, which uses EGL instead.
    consoleInit(NULL);
    smmInit();
    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    PadState pad;
    padInitializeDefault(&pad);
    
    // Other initialization goes here. As a demonstration, we print hello world.
    printf(smmModPathForCfwPath("sdmc:/atmosphere/contents/01000A10041EA000/romfs/Skyrim.ini").c_str());
        
    // Main loop
    while (appletMainLoop())
    {
        // Scan the gamepad. This should be done once for each frame
        padUpdate(&pad);

        // padGetButtonsDown returns the set of buttons that have been
        // newly pressed in this frame compared to the previous one
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus)
            break; // break in order to return to hbmenu

        // Your code goes here

        // Update the console, sending a new frame to the display
        consoleUpdate(NULL);
    }

    smmExit();
    // Deinitialize and clean up resources used by the console (important!)
    consoleExit(NULL);
    return 0;
}
```
</details>

<details>
  <summary>makefile</summary>

```makefile
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
# EXEFS_SRC is the optional input directory containing data copied into exefs, if anything this normally should only contain "main.npdm".
# ROMFS is the directory containing data to be added to RomFS, relative to the Makefile (Optional)
#
# NO_ICON: if set to anything, do not use icon.
# NO_NACP: if set to anything, no .nacp file is generated.
# APP_TITLE is the name of the app stored in the .nacp file (Optional)
# APP_AUTHOR is the author of the app stored in the .nacp file (Optional)
# APP_VERSION is the version of the app stored in the .nacp file (Optional)
# APP_TITLEID is the titleID of the app stored in the .nacp file (Optional)
# ICON is the filename of the icon (.jpg), relative to the project folder.
#   If not set, it attempts to use one of the following (in this order):
#     - <Project name>.jpg
#     - icon.jpg
#     - <libnx folder>/default_icon.jpg
#---------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	source
DATA		:=	data
INCLUDES	:=	include
EXEFS_SRC	:=	exefs_src
#ROMFS	:=	romfs

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIE

CFLAGS	:=	-g -Wall -O2 -ffunction-sections \
			$(ARCH) $(DEFINES)

CFLAGS	+=	$(INCLUDE) -D__SWITCH__

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS	:= -lsmm -lnx 

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(PORTLIBS) $(LIBNX)


#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 	:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export BUILD_EXEFS_SRC := $(TOPDIR)/$(EXEFS_SRC)

ifeq ($(strip $(ICON)),)
	icons := $(wildcard *.jpg)
	ifneq (,$(findstring $(TARGET).jpg,$(icons)))
		export APP_ICON := $(TOPDIR)/$(TARGET).jpg
	else
		ifneq (,$(findstring icon.jpg,$(icons)))
			export APP_ICON := $(TOPDIR)/icon.jpg
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/$(ICON)
endif

ifeq ($(strip $(NO_ICON)),)
	export NROFLAGS += --icon=$(APP_ICON)
endif

ifeq ($(strip $(NO_NACP)),)
	export NROFLAGS += --nacp=$(CURDIR)/$(TARGET).nacp
endif

ifneq ($(APP_TITLEID),)
	export NACPFLAGS += --titleid=$(APP_TITLEID)
endif

ifneq ($(ROMFS),)
	export NROFLAGS += --romfsdir=$(CURDIR)/$(ROMFS)
endif

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).pfs0 $(TARGET).nso $(TARGET).nro $(TARGET).nacp $(TARGET).elf


#---------------------------------------------------------------------------------
else
.PHONY:	all

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all	:	$(OUTPUT).pfs0 $(OUTPUT).nro

$(OUTPUT).pfs0	:	$(OUTPUT).nso

$(OUTPUT).nso	:	$(OUTPUT).elf

ifeq ($(strip $(NO_NACP)),)
$(OUTPUT).nro	:	$(OUTPUT).elf $(OUTPUT).nacp
else
$(OUTPUT).nro	:	$(OUTPUT).elf
endif

$(OUTPUT).elf	:	$(OFILES)

$(OFILES_SRC)	: $(HFILES_BIN)

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	%_bin.h :	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
```
</details>

## Prebuilt Binaries
- To download please refer to this link : [Releases](https://github.com/nadrino/SimpleModManager/releases).
- For libsmm there is a dkp-pacman package in the releases for [libsmm](https://github.com/withertech/libsmm/releases)
## Build From Source

### Prerequisites (macos)
- Install XCode via the App Store
- Launch :
```bash
xcode-select --install
```
- Download DevKitPro : https://github.com/devkitPro/pacman/releases
```bash
sudo installer -pkg /path/to/devkitpro-pacman-installer.pkg -target /
```
- Define environment (add the following lines to your bashrc) :
```bash
function setup_devkitpro()
{
    echo "Seting up DevKitPro..." >&2
    export DEVKITPRO=/opt/devkitpro
    export DEVKITA64=${DEVKITPRO}/devkitA64
    export DEVKITARM=${DEVKITPRO}/devkitARM
    export DEVKITPPC=${DEVKITPRO}/devkitPPC
    export PORTLIBS_PREFIX=${DEVKITPRO}/portlibs/switch

    export PATH=${DEVKITPRO}/tools/bin:$PATH
    export PATH=${DEVKITA64}/bin/:$PATH

    source $DEVKITPRO/switchvars.sh
    return;
}
export -f setup_devkitpro
```
- Source your bashrc and execute "setup_devkitpro"
- Install packages (all are not needed)
```bash
sudo dkp-pacman -Sy \
  switch-bulletphysics switch-bzip2 switch-curl\
  switch-examples switch-ffmpeg switch-flac switch-freetype\
  switch-giflib switch-glad switch-glfw switch-glm\
  switch-jansson switch-libass switch-libconfig\
  switch-libdrm_nouveau switch-libexpat switch-libfribidi\
  switch-libgd switch-libjpeg-turbo switch-libjson-c\
  switch-liblzma switch-liblzo2 switch-libmad switch-libmikmod\
  switch-libmodplug switch-libogg switch-libopus\
  switch-libpcre2 switch-libpng switch-libsamplerate\
  switch-libsodium switch-libtheora switch-libtimidity\
  switch-libvorbis switch-libvorbisidec switch-libvpx\
  switch-libwebp switch-libxml2 switch-mbedtls switch-mesa\
  switch-miniupnpc switch-mpg123 switch-ode switch-oniguruma\
  switch-opusfile switch-pkg-config switch-sdl2 switch-sdl2_gfx\
  switch-sdl2_image switch-sdl2_mixer switch-sdl2_net\
  switch-sdl2_ttf switch-smpeg2 switch-zlib switch-zziplib\
  devkitA64 devkitpro-keyring general-tools pkg-config\
  libnx libfilesystem switch-tools devkitpro-pkgbuild-helpers\
  -r /System/Volumes/Data
sudo dkp-pacman -Suy -r /System/Volumes/Data
```

### Compile
```bash
git clone https://github.com/nadrino/SimpleModManager.git
cd SimpleModManager
mkdir build 
cd build
cmake ../ -DCMAKE_TOOLCHAIN_FILE=../devkita64-libnx.cmake
make
```

## Showcase (YouTube)

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/uJiIzLvsW2Y/0.jpg)](https://www.youtube.com/watch?v=uJiIzLvsW2Y)

## Screenshots
<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/screenshots/1.jpg"></p>
<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/screenshots/2.jpg"></p>
<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/screenshots/3.jpg"></p>
<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/screenshots/4.jpg"></p>
<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/screenshots/5.jpg"></p>
<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/screenshots/6.jpg"></p>
