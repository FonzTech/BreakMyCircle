#pragma once

#include <memory>

#include "CommonTypes.h"
#include "ColoredDrawable.h"

namespace CommonUtility
{
	std::shared_ptr<ColoredDrawable> createGameSphere(Object3D & parent, const Vector3 & diffuseColor, IDrawCallback* drawCallback);
}