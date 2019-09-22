![License](https://img.shields.io/badge/License-GPLv3-blue.svg)
# SimpleModManager
SimpleModManager is an homebrew app for the Nintendo Switch CFW : Atmosphere. It allows to manage your mods (via LayeredFS).

<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/assets/icon.jpg"></p>

## Usage
- Place the .nro file in the `/switch/` folder of your SDcard.
- At the ROOT of your SDcard, create a `/mods/` folder.
- Tree structure : `/mods/<NameOfTheGame>/<NameOfTheMod>/<ModTreeStructureFromAtmosphereFolder>`

Example : `/mods/The Legend of Zelda - Breath of the Wild/First Person View/titles/01007EF00011E000/romfs/Actor/Pack/GameRomCamera.sbactorpack`

## Prebuilt Binaries
- To download please refer to this link : [Releases](https://github.com/nadrino/SimpleModManager/releases).

## Build From Source

### Prerequisites
cmake + devkitPro + libNX

... to be completed ...

### Compile
```bash
git clone https://github.com/nadrino/SimpleModManager.git
cd SimpleModManager
mkdir build && cd build
cmake ../ -DCMAKE_TOOLCHAIN_FILE=../devkita64-libnx.cmake
make
```
## Screenshots
<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/screenshots/1.jpg"></p>
<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/screenshots/2.jpg"></p>
<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/screenshots/3.jpg"></p>
<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/screenshots/4.jpg"></p>
<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/screenshots/5.jpg"></p>
<p align="center"><img src="https://github.com/nadrino/SimpleModManager/blob/master/screenshots/6.jpg"></p>
