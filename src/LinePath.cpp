#include "LinePath.h"

#include <sstream>
#include <Corrade/Containers/PointerStl.h>

#include "CommonUtility.h"

LinePath::LinePath(const std::string & name)
{
	// Load underlying asset
	load(name);

	// Initialize members
	mProgress += 0.0f;
}

void LinePath::update(const Float deltaTime)
{
	// Increment progress
	mProgress += deltaTime;

	// Compute point on line
	Float fv = Math::floor(mProgress);
	Int iv(fv);
	Int indexes[] = { iv, iv + 1 };

	// Compute interpolation factor
	if (indexes[0] < 0)
	{
		mPos = *mAssetResource->begin();
	}
	else if (indexes[0] >= mAssetResource->size() - 1)
	{
		mPos = *(mAssetResource->end() - 1);
	}
	else
	{
		Float delta = mProgress - fv;
		mPos = mAssetResource->at(indexes[0]) + (mAssetResource->at(indexes[1]) - mAssetResource->at(indexes[0])) * delta;
	}
}

Vector3 LinePath::getCurrentPosition()
{
	return mPos;
}

Int LinePath::getSize()
{
	return mAssetResource->size();
}

void LinePath::load(const std::string & name)
{
	// Try to load resource
	mAssetResource = CommonUtility::singleton->manager.get<LinePathAsset>(RESOURCE_PATH_PREFIX + name);
	if (!mAssetResource)
	{
		// Create resource
		std::unique_ptr<LinePathAsset> lpa = std::make_unique<LinePathAsset>();

		// Load raw file
		auto content = Utility::Directory::readString("paths/" + name + ".txt");

		// Read line by line
		std::istringstream sin(content);
		std::string line;
		while (std::getline(sin, line))
		{
			// Read line comma by comma
			std::istringstream lin(line);
			std::string cv;
			Sint8 i = -1;

			Float values[3];

			while (std::getline(lin, cv, ','))
			{
				values[++i] = std::stof(cv);
			}

			// Create Vector3 from raw values
			lpa->push_back(Vector3(values[0], values[1], values[2]));
		}

		// Store resource in manager
		Containers::Pointer<LinePathAsset> p = std::move(lpa);
		CommonUtility::singleton->manager.set(mAssetResource.key(), std::move(p));
	}
}