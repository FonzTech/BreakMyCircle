#pragma once

#define RESOURCE_SCENE_BUBBLE "scenes/bubble.glb"
#define RESOURCE_SCENE_LOGO "scenes/logo.glb"
#define RESOURCE_SCENE_LEVEL_BUTTON "scenes/level_button.glb"
#define RESOURCE_SCENE_COIN "scenes/coin.glb"
#define RESOURCE_SCENE_CANNON_1 "scenes/cannon_1.glb"
#define RESOURCE_SCENE_WORLD_1 "scenes/world_1.glb"

#define RESOURCE_MESH_PLANE_SPRITE "mesh_plane_sprite"
#define RESOURCE_MESH_PLANE_WATER "mesh_plane_water"
#define RESOURCE_MESH_SAND_FLOOR "mesh_plane_sand_floor"
#define RESOURCE_MESH_PLANE_FLAT "mesh_plane_flat"
#define RESOURCE_MESH_CUBE "mesh_plane_cube"

#define RESOURCE_TEXTURE_SPARKLES "tex_sparkles"
#define RESOURCE_TEXTURE_BUBBLE_RED "tex_bubble_red"
#define RESOURCE_TEXTURE_BUBBLE_GREEN "tex_bubble_green"
#define RESOURCE_TEXTURE_BUBBLE_BLUE "tex_bubble_blue"
#define RESOURCE_TEXTURE_BUBBLE_YELLOW "tex_bubble_yellow"
#define RESOURCE_TEXTURE_BUBBLE_ORANGE "tex_bubble_orange"
#define RESOURCE_TEXTURE_BUBBLE_PURPLE "tex_bubble_purple"
#define RESOURCE_TEXTURE_BUBBLE_CYAN "tex_bubble_cyan"
#define RESOURCE_TEXTURE_BUBBLE_TRANSLUCENT "tex_bubble_translucent"
#define RESOURCE_TEXTURE_SKYBOX_1_PX "tex_skybox_1_px"
#define RESOURCE_TEXTURE_WATER_DISPLACEMENT "tex_water_dm"
#define RESOURCE_TEXTURE_WATER_TEXTURE "tex_water_tm"
#define RESOURCE_TEXTURE_WORLD_1_WEM "tex_world_1_wem"
#define RESOURCE_TEXTURE_CUBEMAP_SKYBOX_1 "tex_cubemap_skybox_1"
#define RESOURCE_TEXTURE_GUI_SETTINGS "tex_gui_settings"
#define RESOURCE_TEXTURE_GUI_REPLAY "tex_gui_replay"
#define RESOURCE_TEXTURE_GUI_NEXT "tex_gui_next"
#define RESOURCE_TEXTURE_GUI_SHARE "tex_gui_share"
#define RESOURCE_TEXTURE_GUI_EXIT "tex_gui_exit"
#define RESOURCE_TEXTURE_GUI_LEVEL_PANEL "tex_gui_level_panel"
#define RESOURCE_TEXTURE_GUI_STAR_GRAY "tex_gui_star_gray"
#define RESOURCE_TEXTURE_GUI_BACK_ARROW "tex_gui_back_arrow"
#define RESOURCE_TEXTURE_GUI_PLAY "tex_gui_play"
#define RESOURCE_TEXTURE_GUI_COIN "tex_gui_coin"
#define RESOURCE_TEXTURE_GUI_BUTTON_2X1 "tex_gui_button_2x1"
#define RESOURCE_TEXTURE_WHITE "tex_white"

#define RESOURCE_SHADER_COLORED_PHONG "shader_colored_phong"
#define RESOURCE_SHADER_COLORED_PHONG_2 "shader_colored_phong_2"
#define RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE "shader_textured_phong"
#define RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE_2 "shader_textured_phong_2"
#define RESOURCE_SHADER_SPRITE "shader_sprite"
#define RESOURCE_SHADER_WATER "shader_water"
#define RESOURCE_SHADER_CUBEMAP "shader_cubemap"
#define RESOURCE_SHADER_FLAT3D "shader_flat3d"
#define RESOURCE_SHADER_DISTANCE_FIELD_VECTOR "shader_distance_field_vector"

#define RESOURCE_FONT_UBUNTU_TITLE "ubuntu-title"

#define RESOURCE_AUDIO_BUBBLE_STOMP "bubble_stomp"
#define RESOURCE_AUDIO_BUBBLE_POP "bubble_pop"
#define RESOURCE_AUDIO_BUBBLE_FALL "bubble_fall"
#define RESOURCE_AUDIO_SHOT_PREFIX "shot_"
#define RESOURCE_AUDIO_SHOT_WIN "end_win"
#define RESOURCE_AUDIO_SHOT_LOSE "end_lose"
#define RESOURCE_AUDIO_COIN "COIN"

#define RESOURCE_PATH_PREFIX "path_"
#define RESOURCE_PATH_NEW_SPHERE "new_sphere"

#define GOT_PLAYER 1
#define GOT_BUBBLE 2
#define GOT_PROJECTILE 3
#define GOT_FALLING_BUBBLE 4
#define GOT_SCENERY 5
#define GOT_LOGO 6
#define GOT_SKYBOX 7
#define GOT_OVERLAY_GUI 8
#define GOT_LEVEL_SELECTOR 9
#define GOT_OVERLAY_TEXT 10
#define GOT_OVERLAY_GUI_DETACHED 11
#define GOT_LIMIT_LINE 12
#define GOT_DIALOG 12

#include <Magnum/Magnum.h>
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/Optional.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Trade/PhongMaterialData.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Texture.h>

using namespace Magnum;

typedef SceneGraph::Object<SceneGraph::MatrixTransformation3D> Object3D;
typedef SceneGraph::Scene<SceneGraph::MatrixTransformation3D> Scene3D;

typedef Platform::Application::MouseEvent::Button ImMouseButtons;
typedef Platform::Application::KeyEvent::Key ImKeyButtons;

typedef Containers::Array<Containers::Optional<GL::Mesh>> AssetMeshes;
typedef Containers::Array<Containers::Optional<GL::Texture2D>> AssetTextures;
typedef Containers::Array<Containers::Optional<Trade::PhongMaterialData>> AssetMaterials;

typedef std::vector<Vector3> LinePathAsset;