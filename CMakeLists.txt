cmake_minimum_required(VERSION 3.4)

if(NOT CORRADE_TARGET_ANDROID)
    set(CMAKE_GENERATOR_PLATFORM "x64") # BEFORE PROJECT
    set(CMAKE_VS_PLATFORM_TOOLSET "v141") # BEFORE ON cmake build
endif()

project(BreakMyCircle CXX)

if(NOT CORRADE_TARGET_ANDROID)
    set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_VS_PLATFORM_TOOLSET "v141") # AFTER ON cmake install
endif()

# Add module path in case this is project root
set(CORRADE_SRC_PATH "D:/Development/C++/corrade/modules")
set(CORRADE_BIN_PATH "D:/Development/C++/corrade-bin")
set(SDL2_INCLUDE_DIR "D:/Development/C++/SDL2-2.0.14/include")

set(MAGNUM_SRC_PATH "D:/Development/C++/magnum/modules")
set(MAGNUM_BIN_PATH "D:/Development/C++/magnum-bin")

if(CORRADE_TARGET_ANDROID)
    find_package(Magnum REQUIRED AndroidApplication)
else()
    find_package(Magnum REQUIRED Sdl2Application)
    set(SDL2_LIBRARY "D:/Development/C++/SDL2-2.0.14/lib/x64")
endif()

set(NLOHMANN_JSON_INCLUDE_DIR "D:/Development/C++/nlohmann-json")

set(OPENAL_BIN_PATH "D:/Development/C++/openal-bin")

list(APPEND CMAKE_MODULE_PATH ${CORRADE_SRC_PATH})
list(APPEND CMAKE_MODULE_PATH ${MAGNUM_SRC_PATH})

list(APPEND CMAKE_PREFIX_PATH ${CORRADE_BIN_PATH})
list(APPEND CMAKE_PREFIX_PATH ${MAGNUM_BIN_PATH})
list(APPEND CMAKE_PREFIX_PATH ${SDL2_INCLUDE_DIR})
list(APPEND CMAKE_PREFIX_PATH ${SDL2_LIBRARY})
list(APPEND CMAKE_PREFIX_PATH ${OPENAL_BIN_PATH})

find_package(SDL2 REQUIRED)
find_package(Corrade REQUIRED Main)
find_package(Magnum REQUIRED
    GL
    MeshTools
	  Primitives
    Shaders
    SceneGraph
    Trade)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

if(CORRADE_TARGET_ANDROID)
    add_library(
        native-lib
        SHARED
        src/main.cpp
        src/AssetManager.cpp
        src/CollisionManager.cpp
        src/Engine.cpp
        src/GameObject.cpp
        src/InputManager.cpp
        src/RoomManager.cpp
        src/Audio/StreamedAudioBuffer.cpp
        src/Audio/StreamedAudioPlayable.cpp
        src/Common/AbstractCustomRenderer.cpp
        src/Common/CommonUtility.cpp
        src/Common/LinePath.cpp
        src/Game/AbstractGuiElement.cpp
        src/Game/Bubble.cpp
        src/Game/Congrats.cpp
        src/Game/Dialog.cpp
        src/Game/ElectricBall.cpp
        src/Game/FallingBubble.cpp
        src/Game/LevelSelector.cpp
        src/Game/LimitLine.cpp
        src/Game/Logo.cpp
        src/Game/MapPickup.cpp
        src/Game/OverlayGui.cpp
        src/Game/OverlayGuiDetached.cpp
        src/Game/OverlayText.cpp
        src/Game/Player.cpp
        src/Game/Projectile.cpp
        src/Game/Scenery.cpp
        src/Game/Skybox.cpp
        src/Graphics/BaseDrawable.cpp
        src/Shaders/CubeMapShader.cpp
        src/Shaders/PlasmaShader.cpp
        src/Shaders/ScreenQuadShader.cpp
        src/Shaders/SpriteShader.cpp
        src/Shaders/WaterShader.cpp
    )
else()
    add_executable(
        ${PROJECT_NAME}
        WIN32
        src/main.cpp
        src/AssetManager.cpp
        src/CollisionManager.cpp
        src/Engine.cpp
        src/GameObject.cpp
        src/InputManager.cpp
        src/RoomManager.cpp
        src/Audio/StreamedAudioBuffer.cpp
        src/Audio/StreamedAudioPlayable.cpp
        src/Common/AbstractCustomRenderer.cpp
        src/Common/CommonUtility.cpp
        src/Common/LinePath.cpp
        src/Game/AbstractGuiElement.cpp
        src/Game/Bubble.cpp
        src/Game/Congrats.cpp
        src/Game/Dialog.cpp
        src/Game/ElectricBall.cpp
        src/Game/FallingBubble.cpp
        src/Game/LevelSelector.cpp
        src/Game/LimitLine.cpp
        src/Game/Logo.cpp
        src/Game/OverlayGui.cpp
        src/Game/OverlayGuiDetached.cpp
        src/Game/OverlayText.cpp
        src/Game/Player.cpp
        src/Game/Projectile.cpp
        src/Game/Scenery.cpp
        src/Game/Skybox.cpp
        src/Graphics/BaseDrawable.cpp
        src/Shaders/CubeMapShader.cpp
        src/Shaders/PlasmaShader.cpp
        src/Shaders/ScreenQuadShader.cpp
        src/Shaders/SpriteShader.cpp
        src/Shaders/WaterShader.cpp
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Corrade::Main
    Magnum::Application
    Magnum::GL
    Magnum::Magnum
    Magnum::MeshTools
    Magnum::Primitives
    Magnum::SceneGraph
    Magnum::Shaders
    Magnum::Trade
)

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

# Make the executable a default target to build & run in Visual Studio
# set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})