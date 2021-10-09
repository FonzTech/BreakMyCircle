#pragma once

#define GLF_COLOR_ATTACHMENT_INDEX 0
#define GLF_OBJECTID_ATTACHMENT_INDEX 1

#include <unordered_set>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Timeline.h>

#include "Common/CommonTypes.h"
#include "RoomManager.h"
#include "Shaders/ScreenQuadShader.h"

#if defined(CORRADE_TARGET_ANDROID)
#include <Magnum/Platform/AndroidApplication.h>
#else
#include <Magnum/Platform/Sdl2Application.h>
#endif

#define ENGINE_CONFIGURATION Configuration{}.setTitle("BreakMyCircle")

#if defined(CORRADE_TARGET_IOS) or defined(CORRADE_TARGET_IOS_SIMULATOR)
extern "C"
{
    const char* ios_GetAssetDir();
    Float ios_GetDisplayDensity();
    const char* ios_GetSaveFile();
}
#define DEBUG_OPENGL_CALLS
#endif

using namespace Magnum;

class Engine : public Platform::Application
{
public:

	explicit Engine(const Arguments& arguments);
	~Engine();

protected:
	static const Float mDrawFrameTime;

#ifdef CORRADE_TARGET_ANDROID
    void tickEvent();
#else
    void tickEvent() override;
#endif

protected:

#ifdef CORRADE_TARGET_ANDROID
	void pauseApp() override;
	void resumeApp() override;

	bool isInForeground;
#endif

private:
	// List of layers
	static const Int GO_LAYERS[];

#ifdef ENABLE_DETACHED_DRAWING_FOR_OVERLAY_TEXT
	// List of intrinstic game objects
	static const std::unordered_set<Int> GO_DRAW_DETACHED;
#endif

	// Application methods
	void drawEvent() override;
	void mousePressEvent(MouseEvent& event) override;
	void mouseReleaseEvent(MouseEvent& event) override;
	void mouseMoveEvent(MouseMoveEvent& event) override;
	void viewportEvent(ViewportEvent& event) override;

#ifndef CORRADE_TARGET_ANDROID
	void keyPressEvent(KeyEvent& event) override;
	void keyReleaseEvent(KeyEvent& event) override;
	void exitEvent(ExitEvent& event) override;
#endif

	// Class methods
	void upsertGameObjectLayers();
	void drawInternal();
	void exitInternal(void* arg);
	void viewportInternal(ViewportEvent* event);

	void updateMouseButtonState(MouseEvent& event, const bool & pressed);
	void updateMouseButtonStates(MouseMoveEvent& event);

#ifndef CORRADE_TARGET_ANDROID
	void updateKeyButtonState(const KeyEvent& event, const bool & pressed);
#endif

	// Variables
	Vector2i mCachedFramebufferSize;
	Float mFrameTime;
	Timeline mTimeline;
	Float mDeltaTime;
	ScreenQuadShader mScreenQuadShader;
	RoomManager::GameObjectsLayer* mCurrentGol;
};
