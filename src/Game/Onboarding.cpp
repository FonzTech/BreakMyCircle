#include "Onboarding.h"

#include <string>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/Animation/Easing.h>

#include "../RoomManager.h"
#include "../InputManager.h"
#include "../Common/CommonUtility.h"

std::shared_ptr<GameObject> Onboarding::getInstance(const nlohmann::json & params)
{
	// No default constructor exists for this class!!
	return nullptr;
}

Onboarding::Onboarding(const Int parentIndex, const Int customType, const Float verticalPadding) : GameObject(parentIndex)
{
	// Assign members
	mParentIndex = parentIndex;
	mCustomType = customType;
	mFactor = 0.0f;
	mAngle = Constants::pi();
	mEnd = false;

	// Create GUI
	{
		const auto& p = std::make_shared<OverlayGui>(mParentIndex, RESOURCE_TEXTURE_GUI_TEXTCLOUD);
		p->setPosition({ 1000.0f, 1000.0f });
		p->setSize({ 0.0f, 0.0f });
		p->setAnchor({ 0.0f, 1.0f });

		mOverlayGuis.push_back(p);
		RoomManager::singleton->mGoLayers[mParentIndex].push_back(p);
	}

	{
		const auto& p = std::make_shared<OverlayGui>(mParentIndex, RESOURCE_TEXTURE_GUI_OB_PREFIX + std::to_string(customType));
		p->setPosition({ 1000.0f, 1000.0f });
		p->setSize({ 0.0f, 0.0f });
		p->setAnchor({ 0.0f, 1.0f });

		mOverlayGuis.push_back(p);
		RoomManager::singleton->mGoLayers[mParentIndex].push_back(p);
	}

	// Behaviour depending on provided type
	// const auto& ar = Math::min(1.0f, RoomManager::singleton->getWindowAspectRatio());
	std::string text;

	switch (mCustomType)
	{
	case 1:
		text += "Welcome to BreakMyCircle!\n\n";
		text += "I hope you like this game.\n\n";
		text += "Now I'll help you to onboard\n";
		text += "on this fantastic game!";
		break;

	case 2:
		text += "Now you are in the level\n";
		text += "selector screen. Here you\n";
		text += "can scroll and explore\n";
		text += "the map. To play a level,\n";
		text += "tap on it!\n";
		break;

	case 3:
		text += "You have just completed\n";
		text += "a level. Congratulations!\n";
		text += "Everytime you complete a\n";
		text += "level, you gain coins,\n";
		text += "which you can use to\n";
		text += "buy powerups.\n";
		break;

	case 4:
		text += "If you cannot complete\n";
		text += "a level successfully,\n";
		text += "you can use powerups.\n";
		text += "Just buy them using\n";
		text += "coins or watching ads.\n";
		text += "Check the Settings menu\n";
		text += "while playing a level.";
		break;
	}

	// Create texts
	{
		const auto& ot = std::make_shared<OverlayText>(mParentIndex, Text::Alignment::MiddleCenter, text.length());
		ot->mPosition = Vector3(0.0f, 0.1125f - verticalPadding, 0.0f);
		ot->mColor = Color4(0.95f, 0.95f, 0.95f, 1.0f);
		ot->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
		ot->setSize(Vector2(0.0f));
		ot->setText(text);

		RoomManager::singleton->mGoLayers[mParentIndex].push_back(ot);
		mOverlayTexts.push_back(ot);
	}

	{
		text = "Tap to continue";

		const auto& ot = std::make_shared<OverlayText>(mParentIndex, Text::Alignment::MiddleCenter, text.length());
		ot->mPosition = Vector3(0.0f, 0.375f - verticalPadding, 0.0f);
		ot->mColor = Color4(1.0f, 1.0f, 1.0f, 1.0f);
		ot->mColor = Color4(1.0f, 0.5f, 0.5f, 0.0f);
		ot->setSize(Vector2(0.0f));
		ot->setText(text);

		RoomManager::singleton->mGoLayers[mParentIndex].push_back(ot);
		mOverlayTexts.push_back(ot);
	}
}

Onboarding::~Onboarding()
{
	for (auto& og : mOverlayGuis)
	{
		og->mDestroyMe = true;
	}

	for (auto& ot : mOverlayTexts)
	{
		ot->mDestroyMe = true;
	}
}

const Int Onboarding::getType() const
{
	return GOT_ONBOARDING;
}

void Onboarding::update()
{
	// Animation handling
	mAngle += mDeltaTime * 2.0f;

	if (mEnd)
	{
		mFactor -= mDeltaTime;
	}
	else
	{
		mFactor += mDeltaTime;
	}

	if (mFactor < 0.0f)
	{
		mFactor = 0.0f;
		mDestroyMe = true;
	}
	else if (mFactor > 1.0f)
	{
		mFactor = 1.0f;
	}

	const auto& iv = Math::lerp(0.0f, 1.0f, Animation::Easing::smootherstep(mFactor));
	const auto& nv = 1.0f - iv;

	mOverlayGuis[0]->setPosition({ 0.0f, -0.2f - nv });
	mOverlayGuis[0]->setSize({ 0.45f, 0.45f });

	mOverlayGuis[1]->setPosition({ 0.0f, -0.5f - nv });
	mOverlayGuis[1]->setSize({ 0.3f, 0.3f });

	for (auto& text : mOverlayTexts)
	{
		text->mColor.data()[3] = iv;
		text->mOutlineColor.data()[3] = iv;
	}

	mOverlayTexts[0]->mSize = Vector2(iv * 0.7f);
	mOverlayTexts[1]->mSize = Vector2(iv);

	{
		const auto& av = 0.5f + Math::sin(Rad(mAngle)) * 0.2f;
		mOverlayTexts[1]->mColor = Color4(1.0f, av, av, 1.0f);
	}

	// Tap to continue
	if (!mEnd)
	{
		const auto& lbs = InputManager::singleton->mMouseStates[PRIMARY_BUTTON];
		if (mFactor >= 0.99f && lbs >= IM_STATE_PRESSED)
		{
			mEnd = true;
		}
	}
}

void Onboarding::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
}

Int Onboarding::getCustomType() const
{
	return mCustomType;
}