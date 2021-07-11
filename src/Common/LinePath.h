#pragma once

#include <string>
#include <Corrade/Corrade.h>
#include <Corrade/Utility/Directory.h>
#include <Magnum/Magnum.h>
#include <Magnum/Resource.h>

#include "CommonTypes.h"

using namespace Corrade;
using namespace Magnum;

class LinePath
{
public:
	LinePath(const std::string & name);

	void update(const Float deltaTime);
	Vector3 getCurrentPosition();
	Int getSize();

	Float mProgress;

protected:
	void load(const std::string & name);

	Resource<LinePathAsset> mAssetResource;
	Vector3 mPos;
};