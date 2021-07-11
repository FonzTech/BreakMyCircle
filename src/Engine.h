#pragma once

#define GLF_COLOR_ATTACHMENT_INDEX 0

#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Timeline.h>

#include "Common/CommonTypes.h"
#include "RoomManager.h"
#include "Shaders/ScreenQuadShader.h"

using namespace Magnum;

class Engine : public Platform::Application
{
public:
	explicit Engine(const Arguments& arguments);

protected:
	void tickEvent() override;

private:
	// List of layers
	static const Sint8 GO_LAYERS[];

	// Application methods
	void drawEvent() override;
	void mousePressEvent(MouseEvent& event) override;
	void mouseReleaseEvent(MouseEvent& event) override;
	void mouseMoveEvent(MouseMoveEvent& event) override;
	void keyPressEvent(KeyEvent& event) override;
	void keyReleaseEvent(KeyEvent& event) override;
	void viewportEvent(ViewportEvent& event) override;
	void exitEvent(ExitEvent& event) override;

	// Class methods
	void upsertGameObjectLayers();

	void updateMouseButtonState(const MouseEvent& event, const bool & pressed);
	void updateMouseButtonStates(const MouseMoveEvent& event);
	void updateKeyButtonState(const KeyEvent& event, const bool & pressed);

	// Variables
	Timeline mTimeline;
	Float mDeltaTime;
	ScreenQuadShader mScreenQuadShader;
	RoomManager::GameObjectsLayer* currentGol;
};