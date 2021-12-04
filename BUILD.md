## About BreakMyCircle

This project makes use of C++17. **NO GNU-C++17**!!

## About CMakeLists.txt for dependencies

Please, take note of this snippet below, and replace the `project` directive with the below snippet.

This is necessary, since Windows 10 SDK can change from time to time, so some bugs or peculiar behaviours can be introduced literally the day after.

Also, be aware of cache when working with Android Studio. You may have to delete manually the entire `.cxx` directory, otherwise you end up with mixed files, inconsistent builds and strange (fake) errors.

## Cross-Compiling tips

- Avoid compile *Test* and *Utility* programs when working on dependencies, such as *OpenAL* and *libPNG*, expecially on mobile platforms like iOS, where binaries require *Code Signing*.

- Avoid compile dynamic libraries for *SDL2* when working for mobile platforms.

- Don't build all `SDL_SUBSYSTEMS` for *SDL2*, otherwise iOS will have to be linked against `CoreBluetooth` and other potentially undesired frameworks, unless you are planning to add game-pad support to your application. This means you'll have to add Bluetooth permission if you use SDL2 library with the game-pad support, which is useless (if you don't really require it), expecially if you use Magnum integration.

  This is **NOT** necessary, since **2.0.16**. Disabling the `hidapi` and `Haptic` subsystem in the `CMakeLists.txt` is enough. Check the modified one in the `magnum-edited-sources` repository.
  
  Anyway, to exclude "problematic subsystems" from the build (`Joystick` and `Haptic`), set the `SDL_SUBSYSTEMS` to:

  - `Audio Video Render Events Threads Timers File Loadso Filesystem Sensor Locale`.

Here <https://gitlab.com/fonztech-personal/magnum-edited-sources> can be found the Magnum's Edited Sources, along with other dependencies, which add required capatibilies and fixes, such as:
- fixes on `CMakeLists.txt` for Windows 10 SDK
- pause/resume capability on AndroidApplication with correct GLES Window/Surface/Context handling.

```
if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Android")
  set(CMAKE_GENERATOR_PLATFORM x64) # BEFORE PROJECT
  set(CMAKE_VS_PLATFORM_TOOLSET "v141") # BEFORE ON cmake build
endif()

project(Corrade/Magnum/Whatever-It-Is ${LANG})

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Android")
  set(CMAKE_VS_PLATFORM_TOOLSET "v141") # AFTER ON cmake install
endif()
```

## Compile for Windows from Windows/macOS/Linux

Change paths accordingly to the host system and your needs.

Change the `^` characters, used in Windows' CMD, to `\` when using Linux or macOS.

```
mkdir build && cd build
cmake .. ^
  -DCMAKE_INSTALL_PREFIX="D:/Development/C++/corrade-bin" ^
  -DCORRADE_RC_EXECUTABLE=D:/Development/C++/corrade-bin/bin/corrade-rc.exe ^
  -DTARGET_GLES2=OFF ^
  -DWITH_RC=ON ^
  -DWITH_ANDROIDAPPLICATION=ON ^
  -DWITH_TINYGLTFIMPORTER=ON ^
  -DWITH_PNGIMPORTER=ON ^
  -DWITH_STBVORBISAUDIOIMPORTER=ON ^
  -DWITH_STBTRUETYPEFONT=ON ^
  -DWITH_SDL2APPLICATION=ON ^
  -DWITH_SHADERTOOLS=ON ^
  -DWITH_AUDIO=ON ^
  -DWITH_ANYSCENEIMPORTER=ON ^
  -DWITH_ANYIMAGEIMPORTER=ON
cmake --build .
cmake --build . --target install
```

## Compile for Android

This process should be repeated for both `arm64-v8a` and `x86_64`, and respectively lib suffixes `aarch64-linux-android` and `x86_64` for Target API Level 30.

- CMake arguments `-std=c++17` and `-fno-rtti` must be set. `c++_static` must be used as *Android STL Library*.

### First common step
```
mkdir build && cd build
```

### Windows 10 (NDK 22.0.7026061)
```
cmake .. ^
  -DCMAKE_MAKE_PROGRAM=D:\Development\ninja.exe ^
  -G Ninja ^
  -DCMAKE_CXX_FLAGS=-std=c++11 ^
  -DCMAKE_TOOLCHAIN_FILE=D:\AndroidSDK\ndk-bundle\build\cmake\android.toolchain.cmake ^
  -DCMAKE_SYSTEM_NAME=Android ^
  -DCMAKE_SYSTEM_VERSION=30 ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DNDK_MIN_PLATFORM_LEVEL=21 ^
  -DANDROID_PLATFORM=21 ^
  -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a ^
  -DANDROID_ABI=arm64-v8a ^
  -DCMAKE_ANDROID_STL_TYPE=c++_static ^
  -DCMAKE_CXX_STANDARD=11 ^
  -DCMAKE_INSTALL_PREFIX=D:/AndroidSDK/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr ^
  -DCMAKE_FIND_ROOT_PATH=D:/AndroidSDK/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot ^
  -DCMAKE_FIND_LIBRARY_CUSTOM_LIB_SUFFIX=/aarch64-linux-android/30 ^
  -DLIB_SUFFIX=/aarch64-linux-android/30 ^
  -DCORRADE_RC_EXECUTABLE=D:/Development/C++/corrade-bin/bin/corrade-rc.exe ^
  -DTARGET_GLES2=OFF ^
  -DWITH_RC=ON ^
  -DWITH_ANDROIDAPPLICATION=ON ^
  -DWITH_TINYGLTFIMPORTER=ON ^
  -DWITH_PNGIMPORTER=ON ^
  -DWITH_STBVORBISAUDIOIMPORTER=ON ^
  -DWITH_STBTRUETYPEFONT=ON ^
  -DWITH_SDL2APPLICATION=ON ^
  -DWITH_SHADERTOOLS=ON ^
  -DWITH_AUDIO=ON ^
  -DWITH_ANYSCENEIMPORTER=ON ^
  -DWITH_ANYIMAGEIMPORTER=ON
```

### Last common step
```
cmake --build .
cmake --build . --target install
```

## Compile for iOS

This process allows a single step to compile multiple architectures in a single shot, by specifing a value like `arm64;armv7;armv7s` for the `CMAKE_OSX_ARCHITECTURES` variable. However, from iOS 11 and beyond, only 64-bit architectures are supported. So, we are setting the value `arm64`.

Toolchains must be downloaded from <https://github.com/mosra/toolchains>, whose contents goes into the `toolchains` folder of *Corrade* and *Magnum Graphics* library.

- `CORRADE_RC_EXECUTABLE` can be omitted, by installing the *last stable Corrade* build using Homebrew, with the following command: `brew install mosra/magnum/corrade`

- `CMAKE_TOOLCHAIN_FILE` must NOT be set when compiling *OpenAL*.

- `PNG_LIBRARY` and `PNG_PNG_INCLUDE_DIR` must be set when compiling *Magnum*. Try, respectively, with values `/usr/local/` and `/usr/local/include`. Otherwise, try your own path where you compiled *libPNG* manually.

- For *SDL2*, although the same build method/pipeline/whatever-you-use shall be used, along other dependencies, the script `build-script/iosbuild.sh` can be used to compile correctly for iOS. Also, this script must be edited, so only `arm64` and `x86_64` get compiled and installed as available dependencies. Look for other things which don't meet this requirements, such as `lipo` commands used to create a single library, which are used for iOS Simulator, for example.

- In your Xcode project, Apple Clang argument `-std=c++17` must be used.  `libc++` must be used as *C++ Standard Library*.

- File list in various *Build Phases* must be fixed, otherwise the compilation will not succed. If it succeded, the binary would be bloated with useless files (such as Source files!!). Beware of "Xcode magic"... bleh.

- To build for iOS-Simulator, you must:
  - set `CMAKE_OSX_ARCHITECTURES` to `x86_64`;
  - set `CMAKE_OSX_SYSROOT` to `/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator.sdk`.

- Like the Android guide, the `-fno-rtti` custom flag must be set in *Build Settings*, in *Other C++ Flags*.

- You may have to referrer to this answer to disable *ARM NEON optimizations* on `arm64`:
  - <https://stackoverflow.com/questions/19089014/iphone-device-linker-error>

- The following frameworks must be added in *Build Phases*:
  - Foundation
  - UIKit
  - OpenGLES
  - OpenAL

### First common step
```
mkdir build && cd build
```

### MacOS (Xcode 12.4)
```
cmake .. \
  -G Xcode \
  -DCMAKE_SYSTEM_NAME=iOS \
  -DCMAKE_TOOLCHAIN_FILE=../toolchains/generic/iOS.cmake \
  -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk \
  -DCMAKE_OSX_ARCHITECTURES="arm64" \
  -DCMAKE_INSTALL_PREFIX=~/ios-libs \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_PLUGINS_STATIC=ON \
  -DBUILD_STATIC=ON \
  -DSKIP_INSTALL_PROGRAMS=ON \
  -DTARGET_GLES2=OFF \
  -DWITH_RC=ON \
  -DWITH_TINYGLTFIMPORTER=ON \
  -DWITH_PNGIMPORTER=ON \
  -DWITH_STBVORBISAUDIOIMPORTER=ON \
  -DWITH_STBTRUETYPEFONT=ON \
  -DWITH_SDL2APPLICATION=ON \
  -DWITH_SHADERTOOLS=ON \
  -DWITH_AUDIO=ON \
  -DWITH_ANYSCENEIMPORTER=ON \
  -DWITH_ANYIMAGEIMPORTER=ON
```

### Last common step
```
cmake --build .
cmake --build . --target install
```

### Application Entrypoint

The following sources from *SDL 2.0.16* are modified:

- `src/video/uikit/SDL_uikitappdelegate.h` (added the `getLaunchOption` instance method and `_launchOption` instance property).
- `src/video/uikit/SDL_uikitappdelegate.m` (implemented the above instance method; "original" launch option are saved into the above-mentioned `_launchOption` property).

A slight modification to `SDL_uikitappdelegate` interface is applied, as described above. Specifically, the method `(NSDictionary*)SDL_uikitappdelegate::getLaunchOption` can be used in project, which uses this modified version of SDL2, to obtain launch options for the application, such as detecting if it was launched by a notification tap. The rest of the source is the same that can be found in the *SDL 2.0.16* release.

### AppStore Upload

- Disable *Bitcode support*, since third-party dependencies shall be compiled with bitcode support as well.
- Error **ITMS-90429** is due to this <https://developer.apple.com/library/archive/technotes/tn2435/_index.html#//apple_ref/doc/uid/DTS40017543-CH1-TROUBLESHOOTING-BUNDLE_ERRORS>. For every `dylib`, wrap it in a `framework` file like the one included in this repo, in the `ios` folder. Then, build a `Info.plist` accordingly to the bundled dynamic library file.