#include "Congrats.h"
#include "../RoomManager.h"
#include "../Common/CommonUtility.h"

std::shared_ptr<GameObject> Congrats::getInstance(const nlohmann::json & params)
{
	return nullptr;
}

Congrats::Congrats(const Int parentIndex, const Int customType) : GameObject(parentIndex), mAnimation(-1.0f)
{
	// Create GUI
	{
		mOverlayGui = std::make_shared<OverlayGui>(parentIndex, RESOURCE_TEXTURE_GUI_CONGRATS);
		mOverlayGui->setPosition({ 0.0f, 0.0f });
		mOverlayGui->setSize({ 0.0f, 0.0f });
		mOverlayGui->setAnchor({ 0.0f, 0.0f });
		mOverlayGui->setColor({ 1.0f, 1.0f, 1.0f, 0.0f });
		RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(mOverlayGui);
	}

	// Create text
	{
		std::string text;
		switch (customType)
		{
		case 0:
			text = "Great";
			break;

		case 1:
			text = "Fabulous";
			break;

		case 2:
			text = "Awesome";
			break;

		case 3:
			text = "Fantastic";
			break;

		case 4:
			text = "Extraordinary";
			break;
		}

		mOverlayText = std::make_shared<OverlayText>(GOL_ORTHO_FIRST, Text::Alignment::MiddleCenter, text.length());
		mOverlayText->mPosition = Vector3(2.0f, 2.0f, 0.0f);
		mOverlayText->mColor = Color4(0.868f, 0.241f, 0.186f, 1.0f);
		mOverlayText->mOutlineColor = Color4(0.0f, 0.0f, 0.0f, 1.0f);
		mOverlayText->setSize(Vector2(1.0f));
		mOverlayText->setText(text);
		RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].push_back(mOverlayText);
	}

	// Load audio
	{
		Resource<Audio::Buffer> buffer = CommonUtility::singleton->loadAudioData(RESOURCE_AUDIO_CONGRATS_PREFIX + std::to_string(customType + 1));
		mPlayables[0] = std::make_shared<Audio::Playable3D>(*mManipulator.get(), &RoomManager::singleton->mAudioPlayables);
		mPlayables[0]->source()
			.setBuffer(buffer)
			.setLooping(false);
		playSfxAudio(0);
	}
}

Congrats::~Congrats()
{
	mOverlayGui->mDestroyMe = true;
	mOverlayGui = nullptr;

	mOverlayText->mDestroyMe = true;
	mOverlayText = nullptr;
}

const Int Congrats::getType() const
{
	return GOT_CONGRATS;
}

void Congrats::update()
{
	if (mAnimation < 0.0f)
	{
		mAnimation = 0.0f;
	}
	else
	{
		mAnimation += mDeltaTime * 0.5f;
	}

	if (mAnimation >= 1.0f)
	{
		mDestroyMe = true;
		return;
	}

	{
		const auto& p = Vector2(0.0f, -0.1f + mAnimation * 0.2f);
		mOverlayGui->setPosition(p);
		mOverlayText->setPosition(p);
	}

	if (mAnimation < 0.25f)
	{
		const auto& x = mAnimation * 4.0f;

		mOverlayGui->color()[3] = x;
		mOverlayGui->setSize({ 0.3f * x, 0.15f * x });

		mOverlayText->mColor[3] = x;
		mOverlayText->mOutlineColor[3] = x;
		mOverlayText->setSize(Vector2(x));
	}
	else if (mAnimation < 0.75f)
	{
		mOverlayGui->color()[3] = 1.0f;
		mOverlayGui->setSize({ 0.3f, 0.15f });

		mOverlayText->mColor[3] = 1.0f;
		mOverlayText->mOutlineColor[3] = 1.0f;
		mOverlayText->setSize(Vector2(1.0f));
	}
	else
	{
		const auto& x = (mAnimation - 0.75f) * 4.0f;
		const auto& c = 1.0f - x;

		mOverlayGui->color()[3] = c;
		mOverlayGui->setSize({ 0.3f - x * 0.3f, 0.15f - x * 0.15f });

		mOverlayText->mColor[3] = c;
		mOverlayText->mOutlineColor[3] = c;
	}
}

void Congrats::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
}