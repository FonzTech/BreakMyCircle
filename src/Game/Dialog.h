#pragma once

#define GO_DG_MODE_ACTIONS 1
#define GO_DG_MODE_LOADING 2

#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <Magnum/Timeline.h>
#include <Magnum/Math/Bezier.h>
#include <Magnum/Animation/Animation.h>
#include <Magnum/Animation/Track.h>
#include <Magnum/Animation/Player.h>

#include "../GameObject.h"
#include "../Game/OverlayGui.h"
#include "../Game/OverlayText.h"

class Dialog : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Dialog(const Int parentIndex, const UnsignedInt messageCapacity = 100U, const UnsignedInt titleCapacity = 0U);
	~Dialog();

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

	std::shared_ptr<OverlayText>& getTitleDrawable();
	std::shared_ptr<OverlayText>& getMessageDrawable();
	void setTitle(const std::string & text);
	void setMessage(const std::string & text);
	void setTitlePosition(const Vector3 & position);
	void setMessagePosition(const Vector3 & position);
	void addAction(const std::string & text, const std::function<void(UnsignedInt)> & callback, const bool isLong = false, const Vector3 & offset = Vector3(0.0f), const UnsignedInt capacity = 0U);
	void setActionText(const UnsignedInt index, const std::string & text);
	void closeDialog();
	void shakeButton(const UnsignedInt index);
	void setMode(const Int mode);

protected:

	struct GD_Action
	{
		std::function<void(UnsignedInt)> callback;
		std::shared_ptr<OverlayGui> buttonGui;
		std::shared_ptr<OverlayText> buttonText;
		Float shake;
	};

	Int mMode;
	Float mOpened;
	Float mOpacity;
	Float mRotation;
	Int mClickIndex;
	std::shared_ptr<OverlayGui> mBackground;
	std::shared_ptr<OverlayText> mTitle;
	std::shared_ptr<OverlayText> mMessage;
	std::shared_ptr<OverlayGui> mLoading;
	std::vector<GD_Action> mActions;
};