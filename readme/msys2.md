# MSYS2 on Windows

## Installing
1. get [MSYS2](https://www.msys2.org) and install it as in the description (use *C:\msys64*)
2. run *msys2.exe* in the MSYS2 root directory
3. install CMake by running ``pacman -S mingw-w64-x86_64-cmake``
4. install toolchain by running ``pacman -S mingw-w64-x86_64-toolchain``
5. install psutil by running ``pacman -S mingw-w64-x86_64-python-psutil``
6. if you are asked what exactly to install, just choose all (pressing Enter)
7. run *mingw64.exe* in the MSYS2 root directory
8. install pip by running ``python -m ensurepip``

## Building
1. run *mingw64.exe* in the MSYS2 root directory
2. navigate to the project directory (e.g. ``cd C:/Users/lyniat/Documents/dragonruby-windows-amd64/dr-socket``)
3. run ``sh build-unix.sh -target windows``