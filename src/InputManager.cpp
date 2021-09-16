#include "InputManager.h"

std::unique_ptr<InputManager> InputManager::singleton = nullptr;

InputManager::InputManager()
{
	mMousePosition = { 0, 0 };
	mPreTickMouseStates = {};
	mMouseStates = {};

#ifndef CORRADE_TARGET_ANDROID
	mKeyStates = {};
#endif

	mClickedObjectId = 0U;
}

void InputManager::setMouseState(const ImMouseButtons & key, const bool & pressed)
{
	mPreTickMouseStates[key] = pressed;
}

#ifndef CORRADE_TARGET_ANDROID
void InputManager::setKeyState(const ImKeyButtons & key, const bool & pressed)
{
	mPreTickKeyStates[key] = pressed;
}
#endif

void InputManager::updateMouseStates()
{
	for (auto it = mPreTickMouseStates.begin(); it != mPreTickMouseStates.end(); ++it)
	{
		if (mMouseStates.find(it->first) == mMouseStates.end())
		{
			mMouseStates[it->first] = IM_STATE_NOT_PRESSED;
		}

		auto value = mMouseStates[it->first];
		if (mPreTickMouseStates[it->first])
		{
			if (value != IM_STATE_PRESSING)
			{
				mMouseStates[it->first] = value == IM_STATE_PRESSED ? IM_STATE_PRESSING : IM_STATE_PRESSED;
			}
		}
		else if (value != IM_STATE_NOT_PRESSED)
		{
			mMouseStates[it->first] = value == IM_STATE_RELEASED ? IM_STATE_NOT_PRESSED : IM_STATE_RELEASED;
		}
	}
}

#ifndef CORRADE_TARGET_ANDROID
void InputManager::updateKeyStates()
{
	for (std::unordered_map<ImKeyButtons, Int>::iterator it = mPreTickKeyStates.begin(); it != mPreTickKeyStates.end(); ++it)
	{
		auto* ms = &mKeyStates[it->first];
		if (mPreTickKeyStates[it->first])
		{
			if (*ms != IM_STATE_PRESSING)
			{
				*ms = *ms == IM_STATE_PRESSED ? IM_STATE_PRESSING : IM_STATE_PRESSED;
			}
		}
		else if (*ms != IM_STATE_NOT_PRESSED)
		{
			*ms = *ms == IM_STATE_RELEASED ? IM_STATE_NOT_PRESSED : IM_STATE_RELEASED;
		}
	}
}
#endif