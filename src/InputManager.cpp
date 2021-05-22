#include "InputManager.h"

std::unique_ptr<InputManager> InputManager::singleton = nullptr;

InputManager::InputManager()
{
	mMousePosition = { 0, 0 };
}

void InputManager::setMouseState(const ImMouseButtons & key, const bool & pressed)
{
	mPreTickMouseStates[key] = pressed;	
}

void InputManager::updateMouseStates()
{
	for (std::unordered_map<ImMouseButtons, Sint8>::iterator it = mPreTickMouseStates.begin(); it != mPreTickMouseStates.end(); ++it)
	{
		auto* ms = &mMouseStates[it->first];
		if (mPreTickMouseStates[it->first])
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