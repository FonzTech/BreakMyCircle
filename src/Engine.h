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
#include <SDL2/SDL.h>
#endif

#define ENGINE_CONFIGURATION Configuration{}.setTitle("BreakMyCircle")

#define GO_EN_ASSETS_UNPACKING "game_asset_unpacking"

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

#if defined(CORRADE_TARGET_ANDROID)
	void pauseApp() override;
	void resumeApp() override;
#else
    void pauseApp();
    void resumeApp();
#endif

	bool mIsInForeground;

private:
	// List of layers
	static const Int GO_LAYERS[];

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
    void anyEvent(SDL_Event& event) override;
#endif

	// Class methods
	void startFirstRoom();
	void upsertGameObjectLayers();
	void drawInternal();
	void exitInternal(void* arg);
	void viewportInternal(ViewportEvent* event);

	void updateMouseButtonState(MouseEvent& event, const bool & pressed);
	void updateMouseButtonStates(MouseMoveEvent& event);

#ifdef CORRADE_TARGET_ANDROID
	bool mWaitForUnpack;
#else
	void updateKeyButtonState(const KeyEvent& event, const bool & pressed);
#endif

	// Variables
	Vector2i mCachedFramebufferSize;
	Float mFrameTime;
	Timeline mTimeline;
	Float mDeltaTime;
	ScreenQuadShader mScreenQuadShader;
	RoomManager::GameObjectsLayer* mCurrentGol;
    
#if defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_IOS_SIMULATOR)
    GLint mIosDefaultRenderbufferId;
    Containers::Optional<GL::Framebuffer> mIosDefaultFramebuffer;
#endif
};
