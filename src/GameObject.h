#pragma once

#include <vector>
#include <memory>
#include <unordered_set>

#include <Magnum/Audio/Audio.h>
#include <Magnum/Audio/Buffer.h>
#include <Magnum/Audio/Playable.h>
#include <Magnum/Audio/Source.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Math/Range.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Scene.h>

#include "Common/CommonTypes.h"
#include "Graphics/BaseDrawable.h"

using namespace Magnum;

class GameObject : public IDrawCallback
{
public:
	GameObject();
	GameObject(const Int parentIndex);
	~GameObject();

	bool mDestroyMe;
	Float mDeltaTime;
	Int mParentIndex = std::numeric_limits<Int>::min();

	Object3D* mManipulator;
	std::vector<std::shared_ptr<BaseDrawable>> mDrawables;
	std::unordered_map<Int, std::shared_ptr<Audio::Playable3D>> mPlayables;

	Vector3 mPosition;
	Range3D mBbox;

	virtual const Int getType() const = 0;
	virtual void update() = 0;

	const void playSfxAudio(const Int index, const Float offset = 0.0f);
	const void pushToFront();
};