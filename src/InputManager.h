#pragma once

#define IM_STATE_NOT_PRESSED 0
#define IM_STATE_PRESSED 1
#define IM_STATE_PRESSING 2
#define IM_STATE_RELEASED -1

#include <unordered_map>
#include <memory>
#include <Magnum/Magnum.h>

#include "Common/CommonTypes.h"

using namespace Magnum;

class InputManager
{
public:
	static std::unique_ptr<InputManager> singleton;

	Vector2i mMousePosition;
	std::unordered_map<ImMouseButtons, Int> mMouseStates;

#ifndef CORRADE_TARGET_ANDROID
	std::unordered_map<ImKeyButtons, Int> mKeyStates;
#endif

	UnsignedInt mClickedObjectId;

	InputManager();

	void setMouseState(const ImMouseButtons & key, const bool & pressed);

#ifndef CORRADE_TARGET_ANDROID
	void setKeyState(const ImKeyButtons & key, const bool & pressed);
#endif

	void updateMouseStates();

#ifndef CORRADE_TARGET_ANDROID
	void updateKeyStates();
#endif

protected:
	std::unordered_map<ImMouseButtons, Int> mPreTickMouseStates;

#ifndef CORRADE_TARGET_ANDROID
	std::unordered_map<ImKeyButtons, Int> mPreTickKeyStates;
#endif
};