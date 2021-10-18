#pragma once

#include <nlohmann/json.hpp>

#include "OverlayGui.h"
#include "OverlayText.h"
#include "../Graphics/GameDrawable.h"

class Onboarding : public GameObject
{
public:
	static std::shared_ptr<GameObject> getInstance(const nlohmann::json & params);

	Onboarding(const Int parentIndex, const Int customType);
	~Onboarding();

	const Int getType() const override;
	void update() override;
	void draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override;

	Int getCustomType() const;

protected:
	Int mCustomType;
	Float mFactor;
	Float mAngle;
	bool mEnd;
	std::vector<std::shared_ptr<OverlayGui>> mOverlayGuis;
	std::vector<std::shared_ptr<OverlayText>> mOverlayTexts;
};
