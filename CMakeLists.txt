cmake_minimum_required(VERSION 3.8)

if(CORRADE_TARGET_EMSCRIPTEN AND CMAKE_VERSION VERSION_LESS 3.13)
    message(FATAL_ERROR "The Emscripten target requires CMake 3.13 or newer")
endif()

if(CMAKE_GENERATOR MATCHES "^Visual Studio" AND NOT CORRADE_TARGET_ANDROID AND NOT CORRADE_TARGET_EMSCRIPTEN)
    set(CMAKE_GENERATOR_PLATFORM "x64") # BEFORE PROJECT
    set(CMAKE_VS_PLATFORM_TOOLSET "v141") # BEFORE ON cmake build
endif()

if(CORRADE_TARGET_ANDROID OR CORRADE_TARGET_IOS)
  project(native-lib CXX)
else()
  project(BreakMyCircle CXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CORRADE_TARGET_ANDROID AND NOT CORRADE_TARGET_EMSCRIPTEN)
    set(CMAKE_BUILD_TYPE "Debug")
    if(CMAKE_GENERATOR MATCHES "^Visual Studio")
        set(CMAKE_VS_PLATFORM_TOOLSET "v141") # AFTER ON cmake install
    endif()
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(CORRADE_SRC_PATH "D:/Development/C++/corrade")
set(MAGNUM_SRC_PATH "D:/Development/C++/magnum")

# Add module path in case this is project root
set(MAGNUM_MODULES_PATH "D:/Development/C++/magnum/modules")
set(MAGNUM_PLUGINS_MODULES_PATH "D:/Development/C++/magnum-plugins/modules")

list(APPEND CMAKE_MODULE_PATH ${MAGNUM_MODULES_PATH})
list(APPEND CMAKE_MODULE_PATH ${MAGNUM_PLUGINS_MODULES_PATH})

if(CORRADE_TARGET_ANDROID)

    set(NDK_SYSROOT "D:/AndroidSDK/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr")
    
    set(NDK_API30LIBS_ARM64 "D:/AndroidSDK/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/lib/aarch64-linux-android/30")
    set(NDK_API30LIBS_X8664 "D:/AndroidSDK/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/lib/x86_64-linux-android/30")

    list(APPEND CMAKE_MODULE_PATH ${NDK_SYSROOT})

    if(${ANDROID_ABI} MATCHES "arm64-v8a")
        list(APPEND CMAKE_MODULE_PATH ${NDK_API30LIBS_ARM64})
    else()
        list(APPEND CMAKE_MODULE_PATH ${NDK_API30LIBS_X8664})
    endif()

    include_directories(
        PUBLIC
        "D:/AndroidSDK/ndk-bundle/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include/AL"
        "D:/Development/C++/nlohmann-json"
        "D:/Development/C++/magnum-plugins/src/external/stb"
    )

    if(${ANDROID_ABI} MATCHES "arm64-v8a")
        link_directories(${NDK_API30LIBS_ARM64})
    else()
        link_directories(${NDK_API30LIBS_X8664})
    endif()

    if(${ANDROID_ABI} MATCHES "arm64-v8a")
        list(APPEND CMAKE_PREFIX_PATH ${NDK_API30LIBS_ARM64})
    else()
        list(APPEND CMAKE_PREFIX_PATH ${NDK_API30LIBS_X8664})
    endif()

else()

    if(NOT CORRADE_TARGET_EMSCRIPTEN)
        set(CORRADE_MODULES_PATH "D:/Development/C++/corrade/modules")

        set(SDL2_INCLUDE_DIR "D:/Development/C++/SDL2-2.0.14/include/SDL2" CACHE PATH "SDL2 include directory" FORCE)
        set(SDL2_LIBRARY "D:/Development/C++/SDL2-2.0.14/lib/x64")
        set(CORRADE_BIN_PATH "D:/Development/C++/corrade-bin")
        set(MAGNUM_BIN_PATH "D:/Development/C++/magnum-bin")

        set(OPENAL_BIN_PATH "D:/Development/C++/openal-bin")

        list(APPEND CMAKE_PREFIX_PATH ${MAGNUM_BIN_PATH})
        list(APPEND CMAKE_PREFIX_PATH ${CORRADE_BIN_PATH})
        list(APPEND CMAKE_MODULE_PATH ${CORRADE_MODULES_PATH})
        list(APPEND CMAKE_PREFIX_PATH ${SDL2_INCLUDE_DIR})
        list(APPEND CMAKE_PREFIX_PATH ${SDL2_LIBRARY})
        list(APPEND CMAKE_PREFIX_PATH ${OPENAL_BIN_PATH})
    endif()
endif()

if(CORRADE_TARGET_EMSCRIPTEN)
    list(APPEND CMAKE_FIND_ROOT_PATH ${CMAKE_PREFIX_PATH})
endif()

find_package(Corrade REQUIRED Main)

if(CORRADE_TARGET_ANDROID)

    find_package(Magnum REQUIRED AndroidApplication)

elseif(CORRADE_TARGET_EMSCRIPTEN)

    find_package(Magnum REQUIRED EmscriptenApplication)

else()

    find_package(Magnum REQUIRED Sdl2Application)
    find_package(SDL2 REQUIRED)

endif()

find_package(Magnum REQUIRED
    Audio
    GL
    MeshTools
    Primitives
    Shaders
    SceneGraph
    Text
    Trade
    AnySceneImporter
)

if (MAGNUM_BUILD_STATIC AND USE_MAGNUM_FONT)
    find_package(Magnum REQUIRED
        MagnumFont
    )
endif()

find_package(MagnumPlugins REQUIRED
    StbVorbisAudioImporter
    StbTrueTypeFont
    TinyGltfImporter
    PngImporter
)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

set(SOURCES
    src/AssetManager.cpp
    src/Audio/StreamedAudioBuffer.cpp
    src/Audio/StreamedAudioPlayable.cpp
    src/CollisionManager.cpp
    src/Common/AbstractCustomRenderer.cpp
    src/Common/CommonUtility.cpp
    src/Common/CustomRenderers/LSNumberRenderer.cpp
    src/Common/CustomRenderers/PlasmaSquareRenderer.cpp
    src/Common/LinePath.cpp
    src/Engine.cpp
    src/Game/Callbacks/IAppStateCallback.cpp
    src/Game/AbstractGuiElement.cpp
    src/Game/Bubble.cpp
    src/Game/Congrats.cpp
    src/Game/Dialog.cpp
    src/Game/ElectricBall.cpp
    src/Game/FallingBubble.cpp
    src/Game/LevelSelector.cpp
    src/Game/LevelSelectorSidecar.cpp
    src/Game/LimitLine.cpp
    src/Game/Logo.cpp
    src/Game/MapPickup.cpp
    src/Game/Onboarding.cpp
    src/Game/OverlayGui.cpp
    src/Game/OverlayGuiDetached.cpp
    src/Game/OverlayText.cpp
    src/Game/Player.cpp
    src/Game/Projectile.cpp
    src/Game/SafeMinigame.cpp
    src/Game/Scenery.cpp
    src/Game/Skybox.cpp
    src/GameObject.cpp
    src/Graphics/BaseDrawable.cpp
    src/InputManager.cpp
    src/main.cpp
    src/RoomManager.cpp
    src/Shaders/CubeMapShader.cpp
    src/Shaders/PlasmaShader.cpp
    src/Shaders/ScreenQuadShader.cpp
    src/Shaders/SpriteShader.cpp
    src/Shaders/StarRoadShader.cpp
    src/Shaders/SunShader.cpp
    src/Shaders/ShootPathShader.cpp
    src/Shaders/TimedBubbleShader.cpp
    src/Shaders/WaterShader.cpp
)

if(CORRADE_TARGET_ANDROID)

    add_library(
        ${PROJECT_NAME}
        SHARED
        ${SOURCES}
    )

elseif(CORRADE_TARGET_EMSCRIPTEN)

    add_executable(
        ${PROJECT_NAME}
        ${SOURCES}
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "my-application")

else()

    add_executable(
        ${PROJECT_NAME}
        WIN32
        ${SOURCES}
    )

endif()

if(CORRADE_TARGET_ANDROID)

  target_link_libraries(${PROJECT_NAME} PRIVATE
    Corrade::Main
    Magnum::Application
    Magnum::Audio
    Magnum::GL
    Magnum::Magnum
    Magnum::MeshTools
    Magnum::Primitives
    Magnum::SceneGraph
    Magnum::Shaders
    Magnum::Text
    Magnum::Trade
    Magnum::AnySceneImporter
  )

  if (MAGNUM_BUILD_STATIC AND USE_MAGNUM_FONT)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Magnum::MagnumFont
    )
  endif()

  target_link_libraries(${PROJECT_NAME} PRIVATE
      MagnumPlugins::StbVorbisAudioImporter
      MagnumPlugins::StbTrueTypeFont
      MagnumPlugins::TinyGltfImporter
      MagnumPlugins::PngImporter
  )

elseif(CORRADE_TARGET_EMSCRIPTEN)

    target_include_directories(${PROJECT_NAME} PRIVATE
        "D:/Development/C++/nlohmann-json"
        "D:/Development/C++/magnum-plugins/src/external/stb"
        ${MAGNUMPLUGINS_INCLUDE_DIR}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        Corrade::Main
        Magnum::EmscriptenApplication
        Magnum::Audio
        Magnum::GL
        Magnum::Magnum
        Magnum::MeshTools
        Magnum::Primitives
        Magnum::SceneGraph
        Magnum::Shaders
        Magnum::Text
        Magnum::Trade
        Magnum::AnySceneImporter
        MagnumPlugins::StbVorbisAudioImporter
        MagnumPlugins::StbTrueTypeFont
        MagnumPlugins::TinyGltfImporter
        MagnumPlugins::PngImporter
    )

    foreach(ASSET_DIRECTORY scenes textures shaders audios paths rooms fonts)
        target_link_options(${PROJECT_NAME} PRIVATE
            "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/${ASSET_DIRECTORY}@/${ASSET_DIRECTORY}"
        )
    endforeach()
    target_link_options(${PROJECT_NAME} PRIVATE
        "-sALLOW_MEMORY_GROWTH=1"
        "-sDYNCALLS=1"
    )

else()

  target_include_directories(${PROJECT_NAME} PRIVATE
      "D:/Development/C++/nlohmann-json"
      "D:/Development/C++/magnum-plugins/src/external/stb"
      "D:/Development/C++/SDL2-2.0.14/include"
  )

  target_link_libraries(${PROJECT_NAME} PRIVATE
    Corrade::Main
    Magnum::Application
    Magnum::Audio
    Magnum::GL
    Magnum::Magnum
    Magnum::MeshTools
    Magnum::Primitives
    Magnum::SceneGraph
    Magnum::Shaders
    Magnum::Text
    Magnum::Trade
  )

  if (MAGNUM_BUILD_STATIC AND USE_MAGNUM_FONT)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Magnum::MagnumFont
    )
  endif()

  target_link_libraries(${PROJECT_NAME} PRIVATE
      MagnumPlugins::StbVorbisAudioImporter
      MagnumPlugins::StbTrueTypeFont
      MagnumPlugins::TinyGltfImporter
      MagnumPlugins::PngImporter
  )

endif()

if(CORRADE_TARGET_IOS)
  set_target_properties(my-application PROPERTIES
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/MacOSXBundleInfo.plist.in
    XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "YES"
  )
endif()

if(NOT CORRADE_TARGET_ANDROID AND NOT CORRADE_TARGET_EMSCRIPTEN)

    install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

    install(FILES scenes/cannon_1.glb DESTINATION ${CMAKE_INSTALL_PREFIX}/scenes)

    install(FILES shaders/screen_quad.vert DESTINATION ${CMAKE_INSTALL_PREFIX}/shaders)
    install(FILES shaders/screen_quad.frag DESTINATION ${CMAKE_INSTALL_PREFIX}/shaders)
    install(FILES shaders/passthrough.frag DESTINATION ${CMAKE_INSTALL_PREFIX}/shaders)
    install(FILES shaders/sprite.frag DESTINATION ${CMAKE_INSTALL_PREFIX}/shaders)
    install(FILES shaders/water.frag DESTINATION ${CMAKE_INSTALL_PREFIX}/shaders)

    install(FILES textures/sparkles.png DESTINATION ${CMAKE_INSTALL_PREFIX}/textures)

    install(FILES paths/new_sphere.txt DESTINATION ${CMAKE_INSTALL_PREFIX}/paths)

    install(FILES rooms/intro.txt DESTINATION ${CMAKE_INSTALL_PREFIX}/rooms)

    install(FILES audios/bgmusic.ogg DESTINATION ${CMAKE_INSTALL_PREFIX}/audios)

endif()

if(CORRADE_TARGET_EMSCRIPTEN)
    install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

    install(FILES
        web/my-application.html
        web/my-application.css
        ${MAGNUM_EMSCRIPTENAPPLICATION_JS}
        ${MAGNUM_WEBAPPLICATION_CSS}
        DESTINATION ${CMAKE_INSTALL_PREFIX})

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/my-application.wasm
        ${CMAKE_CURRENT_BINARY_DIR}/my-application.data
        DESTINATION ${CMAKE_INSTALL_PREFIX} OPTIONAL)
endif()

# Make the executable a default target to build & run in Visual Studio
# set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})