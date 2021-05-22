cmake_minimum_required(VERSION 3.4)

set(CMAKE_GENERATOR_PLATFORM "x64") # BEFORE PROJECT
set(CMAKE_VS_PLATFORM_TOOLSET "v141") # BEFORE ON cmake build

project(BreakMyCircle CXX)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_VS_PLATFORM_TOOLSET "v141") # AFTER ON cmake install

# Add module path in case this is project root
set(CORRADE_SRC_PATH "D:/Development/C++/corrade/modules")
set(CORRADE_BIN_PATH "D:/Development/C++/corrade-bin")
set(SDL2_INCLUDE_DIR "D:/Development/C++/SDL2-2.0.14/include")

set(MAGNUM_SRC_PATH "D:/Development/C++/magnum/modules")
set(MAGNUM_BIN_PATH "D:/Development/C++/magnum-bin")

set(SDL2_LIBRARY "D:/Development/C++/SDL2-2.0.14/lib/x64")

list(APPEND CMAKE_MODULE_PATH ${CORRADE_SRC_PATH})
list(APPEND CMAKE_MODULE_PATH ${MAGNUM_SRC_PATH})

list(APPEND CMAKE_PREFIX_PATH ${CORRADE_BIN_PATH})
list(APPEND CMAKE_PREFIX_PATH ${MAGNUM_BIN_PATH})
list(APPEND CMAKE_PREFIX_PATH ${SDL2_INCLUDE_DIR})
list(APPEND CMAKE_PREFIX_PATH ${SDL2_LIBRARY})

find_package(SDL2 REQUIRED)
find_package(Corrade REQUIRED Main)
find_package(Magnum REQUIRED
    GL
    MeshTools
    Shaders
    SceneGraph
    Trade
    Sdl2Application
)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

add_executable(${PROJECT_NAME} WIN32
	src/AssetManager.cpp
	src/BaseDrawable.cpp
	src/Bubble.cpp
	src/CollisionManager.cpp
	src/ColoredDrawable.cpp
	src/Engine.cpp
	src/GameObject.cpp
	src/InputManager.cpp
	src/Player.cpp
	src/Projectile.cpp
	src/RoomManager.cpp
	src/TexturedDrawable.cpp
	src/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Corrade::Main
    Magnum::Application
    Magnum::GL
    Magnum::Magnum
    Magnum::MeshTools
    Magnum::SceneGraph
    Magnum::Shaders
    Magnum::Trade
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES scene/test.glb DESTINATION ${CMAKE_INSTALL_PREFIX}/output)

# Make the executable a default target to build & run in Visual Studio
# set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})