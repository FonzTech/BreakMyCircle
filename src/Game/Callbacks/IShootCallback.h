#pragma once

#define ISC_STATE_SHOOT_STARTED 1
#define ISC_STATE_SHOOT_FINISHED 2

#include <Magnum/Magnum.h>

using namespace Magnum;

class IShootCallback
{
public:
	virtual void shootCallback(const Int state, const Color3 & preColor, const Color3 & postColor) = 0;
};