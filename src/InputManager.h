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
	static std::unique_ptr<InputManager> singleton;

	Vector2i mMousePosition;
	std::unordered_map<ImMouseButtons, Sint8> mMouseStates;
	std::unordered_map<ImKeyButtons, Sint8> mKeyStates;

	InputManager();

	void setMouseState(const ImMouseButtons & key, const bool & pressed);
	void setKeyState(const ImKeyButtons & key, const bool & pressed);

	void updateMouseStates();
	void updateKeyStates();

protected:
	std::unordered_map<ImMouseButtons, Sint8> mPreTickMouseStates;
	std::unordered_map<ImKeyButtons, Sint8> mPreTickKeyStates;
};