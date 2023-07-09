set NDK_PATH="%AppData%\..\Local\Android\Sdk\ndk\25.2.9519653"
set ANDROID_PLATFORM=android-21

set BUILD_TYPE="Debug"

call :build_android "arm64-v8a"
call :build_android "armeabi-v7a"
call :build_android "x86_64"
call :build_android "x86"
exit 0

:build_android
cmake ^
-S. ^
-Bcmake-build-android-%1-%BUILD_TYPE% ^
-DANDROID_ABI=%1 ^
-DANDROID_PLATFORM=%ANDROID_PLATFORM% ^
-DANDROID_NDK=%NDK_PATH%/ ^
-DCMAKE_TOOLCHAIN_FILE=%NDK_PATH%/build/cmake/android.toolchain.cmake ^
-DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
-G Ninja
cmake --build cmake-build-android-%1-%BUILD_TYPE% -j 8
exit /B 0