# DragonRuby Socket Extension
A DragonRuby socket library based on [ENet](https://github.com/lsalzman/enet) for developing games with realtime multiplayer.

[Docs](https://lyniat.github.io/dr-socket/)

# How-To
You need [DragonRuby](https://dragonruby.org/toolkit/game) *Indie* or *Pro* to be able to use *C Extensions* and this library.
CMake should handle all the stuff so after successfully building you should be able to start this app just like any other DR app.

Additionally required for building:
- CMake (>= 3.22)
- git
- Python 3

## Windows
- [read this](readme/msys2.md) if you want to use MSYS2
- [MingW](https://winlibs.com) (tested on *GCC 13.1.0 (with POSIX threads) + LLVM/Clang/LLD/LLDB 16.0.5 + MinGW-w64 11.0.0 (UCRT) - release 5*)
- Run ``build-windows.bat --target windows``.

## macOS
- Run ``build-unix.sh --target macos``.
- Add ``-fat-binary`` if you want to create a fat binary for both x86_64 and amd64.

## Linux
- Run ``build-unix.sh --target linux``.

## Android
### Windows
- Run ``build-windows.bat --target android``.

### macOS / Linux
- Run ``build-unix.sh --target android``.

## iOS
- Run ``build-unix.sh --target ios``.

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