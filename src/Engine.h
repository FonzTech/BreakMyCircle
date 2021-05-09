#pragma once

#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Timeline.h>

using namespace Magnum;

class Engine : public Platform::Application
{
public:
	explicit Engine(const Arguments& arguments);

protected:
	Magnum::Timeline timeline;
	Magnum::Float deltaTime;

	void tickEvent() override;


private:
	void drawEvent() override;
	void mousePressEvent(MouseEvent& event) override;
	void mouseReleaseEvent(MouseEvent& event) override;
	void mouseMoveEvent(MouseMoveEvent& event) override;
	void viewportEvent(ViewportEvent& event) override;
	void exitEvent(ExitEvent& event) override;
};