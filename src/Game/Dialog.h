#pragma once

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
	void collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects) override;

	std::shared_ptr<OverlayText>& getTitleDrawable();
	std::shared_ptr<OverlayText>& getMessageDrawable();
	void setTitle(const std::string & text);
	void setMessage(const std::string & text);
	void setTitlePosition(const Vector3 & position);
	void setMessagePosition(const Vector3 & position);
	void addAction(const std::string & text, const std::function<void(UnsignedInt)> & callback, const bool isLong = false, const Vector3 & offset = Vector3(0.0f));
	void closeDialog();
	void shakeButton(const UnsignedInt index);

protected:

	struct GD_Action
	{
		std::function<void(UnsignedInt)> callback;
		std::shared_ptr<OverlayGui> buttonGui;
		std::shared_ptr<OverlayText> buttonText;
		Float shake;
	};

	Float mOpened;
	Float mOpacity;
	Int mClickIndex;
	std::shared_ptr<OverlayGui> mBackground;
	std::shared_ptr<OverlayText> mTitle;
	std::shared_ptr<OverlayText> mMessage;
	std::vector<GD_Action> mActions;
};