#pragma once

#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Timeline.h>

#include "CommonTypes.h"

using namespace Magnum;

class Engine : public Platform::Application
{
public:
	explicit Engine(const Arguments& arguments);

protected:
	Timeline mTimeline;
	Float mDeltaTime;

	void tickEvent() override;

private:
	void drawEvent() override;
	void mousePressEvent(MouseEvent& event) override;
	void mouseReleaseEvent(MouseEvent& event) override;
	void mouseMoveEvent(MouseMoveEvent& event) override;
	void keyPressEvent(KeyEvent& event) override;
	void keyReleaseEvent(KeyEvent& event) override;
	void viewportEvent(ViewportEvent& event) override;
	void exitEvent(ExitEvent& event) override;

	void updateMouseButtonState(const MouseEvent& event, const bool & pressed);
	void updateMouseButtonStates(const MouseMoveEvent& event);
	void updateKeyButtonState(const KeyEvent& event, const bool & pressed);
};