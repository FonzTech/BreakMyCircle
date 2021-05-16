#pragma once

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

typedef Containers::Array<Containers::Optional<GL::Mesh>> AssetMeshes;
typedef Containers::Array<Containers::Optional<GL::Texture2D>> AssetTextures;	
typedef Containers::Array<Containers::Optional<Trade::PhongMaterialData>> AssetMaterials;