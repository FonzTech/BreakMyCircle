#pragma once

#define IM_STATE_NOT_PRESSED 0
#define IM_STATE_PRESSED 1
#define IM_STATE_PRESSING 2
#define IM_STATE_RELEASED -1

#include <unordered_map>
#include <memory>
#include <Magnum/Magnum.h>
#include <Magnum/Platform/Sdl2Application.h>

#include "CommonTypes.h"

using namespace Magnum;

class InputManager
{
public:
	static std::shared_ptr<InputManager> singleton;

	Vector2i mousePosition;
	std::unordered_map<ImMouseButtons, Sint8> preTickMouseStates, mouseStates;

	void setMouseState(const ImMouseButtons & key, const bool & pressed);
	void updateMouseStates();
};