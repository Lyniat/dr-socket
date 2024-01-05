from argparse import ArgumentParser
import platform
import re
from shutil import which
import subprocess
import sys


class shell_colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def exit_with_error(error):
    sys.stdout.write(shell_colors.FAIL + "Error: " + error + "\n" + shell_colors.ENDC)
    sys.stdout.flush()
    sys.exit(1)


def warn(message):
    sys.stdout.write(shell_colors.WARNING + "Warning: " + message + "\n" + shell_colors.ENDC)
    sys.stdout.flush()


def warn_wrong_os(host, target_os):
    warn('Unsupported target ' + target_os + ' on platform ' + host + '. Skipping...')
    sys.stdout.flush()

parser = ArgumentParser()
parser.add_argument("-target", "--target", nargs='+', type=str)
parser.add_argument("-fat-binary", "--fat-binary", action='store_true')
parser.add_argument("-release", "--release", action='store_true')
parser.add_argument("-sysroot", "--sysroot", type=str, default="")

args = parser.parse_args()

advanced_shell_support = True

host_os = platform.system()
if host_os == "Darwin":
    host_os = "macos"
elif host_os == "Linux":
    host_os = "linux"
elif host_os == "Windows":
    host_os = "windows"
else:
    exit_with_error("Unsupported host OS: " + host_os + "!")

if args.target is None:
    exit_with_error("You need at least one target!")

cmake_exec = "cmake"

if args.sysroot != "":
    if which("cmake", path=args.sysroot) is not None:
        if host_os == "windows":
            sep = "\\"
            if args.sysroot[len(args.sysroot) - 1] == "\\":
                sep = ""
            cmake_exec = args.sysroot + sep + "cmake.exe"
        else:
            sep = "/"
            if args.sysroot[len(args.sysroot) - 1] == "/":
                sep = ""
            cmake_exec = args.sysroot + sep + "cmake"
    else:
        if which("cmake") is None:
            exit_with_error("Error: Can't find Cmake!")
else:
    if which("cmake") is None:
        exit_with_error("Error: Can't find Cmake!")

build_commands = []
android_abis = ["arm64-v8a", "armeabi-v7a", "x86_64", "x86"]
android_platform = "android-21"
ndk_path = "~/Library/Android/sdk/ndk/25.2.9519653"

if args.release:
    build_type = "Release"
else:
    build_type = "Debug"

os_type = None
os_architecture = "x86_64"

if host_os == "macos":
    if platform.machine() == "arm64":
        os_architecture = "arm64"

for target in args.target:
    target = target.lower()
    if target == "windows":
        if host_os != "windows":
            warn_wrong_os(host_os, target)
            continue
        os_type = "windows"
        sysroot = ""
        if args.sysroot != "":
            sysroot = "-DCMAKE_SYSROOT=" + args.sysroot
        build_commands.append({
            "conf": "%s -S. -Bcmake-build-%s-%s -DCMAKE_BUILD_TYPE=%s -G \"MinGW Makefiles\" %s" % (cmake_exec, os_type, build_type, build_type, sysroot),
            "make": "%s --build cmake-build-%s-%s -j 8" % (cmake_exec, os_type, build_type),
            "target": "windows"
        })
    elif target == "macos":
        if args.fat_binary:
            default_target = None
        else:
            default_target = os_architecture
        if host_os != "macos":
            warn_wrong_os(host_os, target)
            continue
        os_type = "macos"
        build_commands.append({
            "conf": "%s -Bcmake-build-%s-%s-%s -DCMAKE_BUILD_TYPE=%s -DCMAKE_OSX_ARCHITECTURES=%s" % (cmake_exec, os_type, os_architecture, build_type, build_type, os_architecture),
            "make": "%s --build cmake-build-%s-%s-%s -j 8" % (cmake_exec, os_type, os_architecture, build_type),
            "target": default_target
        })
        if args.fat_binary:
            if os_architecture == "x86_64":
                alt_os_architecture = "arm64"
            else:
                alt_os_architecture = "x86_64"
            build_commands.append({
                "conf": "%s -Bcmake-build-%s-%s-%s -DCMAKE_BUILD_TYPE=%s -DCMAKE_OSX_ARCHITECTURES=%s" % (cmake_exec, os_type, alt_os_architecture, build_type, build_type, alt_os_architecture),
                "make": "%s --build cmake-build-%s-%s-%s -j 8" % (cmake_exec, os_type, alt_os_architecture, build_type),
                "target": default_target
            })
    elif target == "linux":
        if host_os != "linux":
            warn_wrong_os(host_os, target)
            continue
        os_type = "linux"
        build_commands.append({
            "conf": "%s -S. -Bcmake-build-%s-%s -DCMAKE_BUILD_TYPE=%s" % (cmake_exec, os_type, build_type, build_type),
            "make": "%s --build cmake-build-%s-%s -j 8" % (cmake_exec, os_type, build_type),
            "target": "linux"
        })
    elif target == "raspberrypi":
        warn('Unsupported target ' + target + '. Skipping...')
        continue
        if host_os != "linux":
            warn_wrong_os(host_os, target)
            continue
        os_type = "linux-raspberrypi"
        build_commands.append({
            "conf": "%s -S. -Bcmake-build-%s-%s -DCMAKE_BUILD_TYPE=%s" % (cmake_exec, os_type, build_type, build_type),
            "make": "%s --build cmake-build-%s-%s -j 8" % (cmake_exec, os_type, build_type),
            "target": "linux-raspberrypi"
        })
    elif target == "ios":
        if host_os != "macos":
            warn_wrong_os(host_os, target)
            continue
        os_type = "ios"
        build_commands.append({
            "conf": "%s -S. -Bcmake-build-%s-%s -DCMAKE_BUILD_TYPE=%s -DAPPLE_IOS=TRUE" % (cmake_exec, os_type, build_type, build_type),
            "make": "%s --build cmake-build-%s-%s -j 8" % (cmake_exec, os_type, build_type),
            "target": "ios"
        })
    elif target == "android":
        os_type = "android"
        for abi in android_abis:
            conf = ("%s -S. -Bcmake-build-android-%s-%s -DANDROID_ABI=%s -DANDROID_PLATFORM=%s"
             "-DANDROID_NDK=%s -DCMAKE_TOOLCHAIN_FILE=%s/build/cmake/android.toolchain.cmake"
             "-DCMAKE_BUILD_TYPE=%s -G Ninja") % (cmake_exec, abi, build_type, abi, android_platform, ndk_path, ndk_path, build_type)
            build_commands.append({
                "conf": conf,
                "make": "%s --build cmake-build-android-%s-%s -j 8" % (cmake_exec, abi, build_type),
                "target": "android"
            })
    elif target == "wasm":
        warn('Unsupported target ' + target + '. Skipping...')
        continue
        os_type = "emscripten"
    else:
        warn('Unsupported target ' + target + '. Skipping...')
        continue

def exec_or_raise(command):
    subprocess.run(command, shell=True, check=True)


for command in build_commands:
    print(command["conf"])
    print(command["make"])
    exec_or_raise(command["conf"])
    exec_or_raise(command["make"])

if host_os != "macos" and args.fat_binary:
    exec_or_raise("mkdir -p cmake-build-macos-fat-%s" % build_type)
    exec_or_raise("lipo -create -output"
                  "cmake-build-macos-fat-%s/socket.dylib"
                  "cmake-build-macos-x86_64-%s/socket.dylib"
                  "cmake-build-macos-arm64-%s/socket.dylib" % (build_type, build_type, build_type))
    exec_or_raise("cp cmake-build-macos-fat-%s/socket.dylib native/macos/socket.dylib" % build_type)
