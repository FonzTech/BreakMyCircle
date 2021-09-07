## About CMakeLists.txt for Corrade/Magnum

Please, take note of this snippet below, and replace the `project` directive with the below snippet.

This is necessary, since Windows 10 SDK can change from time to time, so some bugs or peculiar behaviours can be introduced literally the day after.

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
```
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX="D:/Development/C++/corrade-bin" ..
cmake --build .
cmake --build . --target install
```

## Compile for Android

This process should be repeat for both `arm64-v8a` and `x86_64`, and respectively lib suffixes `aarch64-linux-android` and `x86_64`for Target API Level 30.

### First common step
```
mkdir build-android-arm64 && cd build-android-arm64
```

### Corrade (using Windows, NDK 22.0.7026061)
```
cmake .. ^
  -DCMAKE_MAKE_PROGRAM=D:\Development\ninja.exe ^
  -G Ninja ^
  -DCMAKE_CXX_FLAGS=-std=c++11 ^
  -DCMAKE_TOOLCHAIN_FILE=D:\AndroidSDK\ndk-bundle\build\cmake\android.toolchain.cmake ^
  -DCMAKE_SYSTEM_NAME=Android ^
  -DCMAKE_SYSTEM_VERSION=30 ^
  -DNDK_MIN_PLATFORM_LEVEL=21 ^
  -DANDROID_PLATFORM=21 ^
  -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a ^
  -DCMAKE_ANDROID_STL_TYPE=c++_static ^
  -DCMAKE_CXX_STANDARD=11 ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_INSTALL_PREFIX=D:/AndroidSDK/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr ^
  -DCMAKE_FIND_ROOT_PATH=D:/AndroidSDK/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot ^
  -DCMAKE_FIND_LIBRARY_CUSTOM_LIB_SUFFIX=/aarch64-linux-android/30
```

### Magnum Graphics and Plugins (using Windows, NDK 23.0.7599858)
```
cmake .. ^
  -DCMAKE_MAKE_PROGRAM=D:\Development\ninja.exe ^
  -G Ninja ^
  -DCMAKE_CXX_FLAGS=-std=c++11 ^
  -DCMAKE_TOOLCHAIN_FILE=D:\AndroidSDK\ndk-bundle\build\cmake\android.toolchain.cmake ^
  -DCMAKE_SYSTEM_NAME=Android ^
  -DCMAKE_SYSTEM_VERSION=30 ^
  -DNDK_MIN_PLATFORM_LEVEL=21 ^
  -DANDROID_PLATFORM=21 ^
  -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a ^
  -DCMAKE_ANDROID_STL_TYPE=c++_static ^
  -DCMAKE_CXX_STANDARD=11 ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DCMAKE_INSTALL_PREFIX=D:/AndroidSDK/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr ^
  -DCMAKE_FIND_ROOT_PATH=D:/AndroidSDK/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot ^
  -DCMAKE_FIND_LIBRARY_CUSTOM_LIB_SUFFIX=/aarch64-linux-android/30 ^
  -DCORRADE_RC_EXECUTABLE=D:\Development\C++\corrade-bin\bin\corrade-rc.exe ^
  -DTARGET_GLES2=OFF ^
  -DWITH_ANDROIDAPPLICATION=ON
```

### Last common step
```
cmake --build .
cmake --build . --target install
```