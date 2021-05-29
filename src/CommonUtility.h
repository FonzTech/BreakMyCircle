#pragma once

#include <memory>

#include "ColoredDrawable.h"

namespace CommonUtility
{
	std::shared_ptr<ColoredDrawable> createGameSphere(const Vector3 & diffuseColor, IDrawCallback* drawCallback);
}