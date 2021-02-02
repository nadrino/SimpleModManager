![License](https://img.shields.io/badge/License-GPLv3-blue.svg) [![GitHub version](https://badge.fury.io/gh/nadrino%2FSimpleModManager.svg)](https://github.com/nadrino/SimpleModManager/releases/) [![Github all releases](https://img.shields.io/github/downloads/nadrino/SimpleModManager/total.svg)](https://GitHub.com/nadrino/SimpleModManager/releases/)

# SimpleModManager
SimpleModManager is an homebrew app for the Nintendo Switch CFW : Atmosphere. It allows to manage your mods (via LayeredFS).

<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/assets/icon_gui.jpg"></p>

## Usage
- Place the .nro file in the `/switch/` folder of your SDcard.
- At the root of your SDcard, create a `/mods/` folder.
- Tree structure : `/mods/<NameOfTheGame>/<NameOfTheMod>/<ModTreeStructureFromAtmosphereFolder>`
- For plugins: `/mods/<NameOfTheGame>/.plugins/<NameOfTheNro>.nro`
- Plugins can be any hbmenu nro and can be used for added functionality

Example : `/mods/The Legend of Zelda - Breath of the Wild/First Person View/contents/01007EF00011E000/romfs/Actor/Pack/GameRomCamera.sbactorpack`

## Prebuilt Binaries
- To download please refer to this link : [Releases](https://github.com/nadrino/SimpleModManager/releases).

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
