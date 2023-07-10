# DragonRuby Socket Extension
A DragonRuby socket library based on [ENet](https://github.com/lsalzman/enet) for developing games with realtime multiplayer.

[Docs](https://lyniat.github.io/dr-socket/)

# How-To
You need [DragonRuby](https://dragonruby.org/toolkit/game) *Indie* or *Pro* to be able to use *C Extensions* and this library.
CMake should handle all the stuff so after successfully building you should be able to start this app just like any other DR app.

Additionally required for building:
- CMake (>= 3.22)
- git

## Windows
- [MingW](https://winlibs.com) (tested on *GCC 13.1.0 (with POSIX threads) + LLVM/Clang/LLD/LLDB 16.0.5 + MinGW-w64 11.0.0 (UCRT) - release 5*)
Run ``build-windows-debug.bat`` or ``build-windows-release.bat``.

## macOS
Run ``build-unix-debug.sh`` or ``build-unix-release.sh``.
clang should be able to compile this out of the box.

## Linux
Run ``build-unix-debug.sh`` or ``build-unix-release.sh``.
Tested with clang but gcc might also work.

## Android
### Windows
Run ``build-android-debug.bat`` or ``build-android-release.bat``.

### macOS / Linux
Run ``build-android-debug.sh`` or ``build-android-release.sh``.

## iOS
Run ``build-ios-debug.sh``.

## Raspberry Pi
WIP

## Oculus Quest
WIP

# About
Currently only proof-of-concept but I plan to make this a useful thing 😉

You can help by joining the official [DragonRuby Discord](https://dragonruby.org/toolkit/game/chat) and visit the *#oss-dr-socket* channel.

# License
- See [LICENSE](LICENSE)
- App icon by [photo3idea_studio](https://www.flaticon.com/de/kostenlose-icons/smart-plug). This is only for testing. Please change it for your own app/game.