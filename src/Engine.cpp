#include "Engine.h"
#include "Common/CommonUtility.h"
#include "Audio/StreamedAudioBuffer.h"
#include "Game/OverlayText.h"
#include "InputManager.h"
#include "RoomManager.h"
#include "GameObject.h"

#if defined(DEBUG) and defined(TARGET_MOBILE)
#define DEBUG_OPENGL_CALLS
#endif

#ifdef DEBUG_OPENGL_CALLS
#include <Magnum/GL/DebugOutput.h>
#endif

#if defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_IOS_SIMULATOR)
#include <Magnum/GL/Context.h>
#endif

using namespace Magnum::Math::Literals;

const Float Engine::mDrawFrameTime =
#ifdef TARGET_MOBILE
0.016f
#else
0.009f
#endif
;

const Int Engine::GO_LAYERS[] = {
    GOL_PERSP_FIRST,
    GOL_PERSP_SECOND,
    GOL_ORTHO_FIRST
};

Engine::Engine(const Arguments& arguments) :
#ifdef CORRADE_TARGET_ANDROID
Platform::Application{ arguments, ENGINE_CONFIGURATION }, mWaitForUnpack(true)
#elif defined(CORRADE_TARGET_IOS) or defined(CORRADE_TARGET_IOS_SIMULATOR)
Platform::Application{ arguments, Configuration{}.setTitle("Break My Circle").setWindowFlags(Configuration::WindowFlag::Fullscreen | Configuration::WindowFlag::Resizable) }, mIosDefaultFramebuffer(Containers::NullOpt)
#else
Platform::Application{ arguments, Configuration{}.setTitle("Break My Circle").setSize({ 768, 768 }).setWindowFlags(Configuration::WindowFlag::Resizable) }
#endif
, mFrameTime(0.0f), mCurrentGol(nullptr), mIsInForeground(true)
{
#if defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_IOS_SIMULATOR)
    
    // Get default framebuffer and renderbuffer
    {
        GLint iosDefaultFboId;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &iosDefaultFboId);
        glGetIntegerv(GL_RENDERBUFFER_BINDING, &mIosDefaultRenderbufferId);
        mIosDefaultFramebuffer = GL::Framebuffer::wrap(GLuint(iosDefaultFboId), GL::defaultFramebuffer.viewport());
    }
    
    // Setup app
    ios_SetupApp();
    
#endif

#ifdef DEBUG_OPENGL_CALLS
    GL::Renderer::enable(GL::Renderer::Feature::DebugOutput);
    GL::Renderer::enable(GL::Renderer::Feature::DebugOutputSynchronous);
    GL::DebugOutput::setDefaultCallback();
#endif

    // Set renderer features
    // GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
    // GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
    // GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::OneMinusSourceAlpha, GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::DestinationAlpha);
    GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add, GL::Renderer::BlendEquation::Add);

    // Set clear color
    GL::Renderer::setClearColor(Color4(0.0f, 0.0f, 0.0f, 0.0f));
	GL::Renderer::setColorMask(true, true, true, true);

    // Init common utility
    CommonUtility::singleton = std::make_unique<CommonUtility>();

#if defined(CORRADE_TARGET_IOS) or defined(CORRADE_TARGET_IOS_SIMULATOR)

    // Configure engine parameters
    CommonUtility::singleton->mConfig.assetDir = ios_GetAssetDir();
    CommonUtility::singleton->mConfig.canvasVerticalPadding = ios_GetCanvasVerticalPadding();
    CommonUtility::singleton->mConfig.displayDensity = ios_GetDisplayDensity();
    CommonUtility::singleton->mConfig.saveFile = ios_GetSaveFile();
    
#elif defined(CORRADE_TARGET_ANDROID)

    // Set native activity
    CommonUtility::singleton->mConfig.nativeActivity = nativeActivity();

    // Configure engine parameters
    const std::array<std::string, 4> params = { "asset_dir", "canvas_vertical_height", "density", "save_file" };
    for (UnsignedInt i = 0; i != params.size(); ++i)
    {
        const auto& key = params.at(i);
        const std::unique_ptr<std::string> value = CommonUtility::singleton->getValueFromIntent(key);
        if (value == nullptr)
        {
            Error{} << "Value for key" << key << "was NULL";
        }
        else
        {
            switch (i)
            {
            case 0U:
                CommonUtility::singleton->mConfig.assetDir = *value;
                break;

            case 1U:
                CommonUtility::singleton->mConfig.canvasVerticalPadding = std::stof(*value);
                break;

            case 2U:
                CommonUtility::singleton->mConfig.displayDensity = std::stof(*value);
                break;

            case 3U:
                CommonUtility::singleton->mConfig.saveFile = *value;
                break;
            }
        }
    }
    
#endif

    Debug{} << "Asset base directory is" << CommonUtility::singleton->mConfig.assetDir;

    // Init input manager
    InputManager::singleton = std::make_unique<InputManager>();

    // Init room manager
    RoomManager::singleton = std::make_unique<RoomManager>();
    RoomManager::singleton->setSfxGain(RoomManager::singleton->mSaveData.sfxEnabled ? 1.0f : 0.0f);
    RoomManager::singleton->setup();
    
#if defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_IOS_SIMULATOR)
    RoomManager::singleton->mDefaultFramebufferPtr = &(*mIosDefaultFramebuffer);
#endif

    // Setup room manager
    RoomManager::singleton->setWindowSize(Vector2(windowSize()));
    viewportInternal(nullptr);

#ifndef GO_EN_ASSETS_UNPACKING
    startFirstRoom();
#endif
}

Engine::~Engine()
{
#ifdef CORRADE_TARGET_ANDROID
    exitInternal(nullptr);
#endif
}

void Engine::tickEvent()
{
#ifdef GO_EN_ASSETS_UNPACKING
    // Wait for assets unpacking
    if (mWaitForUnpack)
    {
        const auto& aup = CommonUtility::singleton->getValueFromIntent(GO_EN_ASSETS_UNPACKING);
        while (aup == nullptr)
        {
            mTimeline.nextFrame();
            redraw();
            return;
        }
        mWaitForUnpack = false;

        // Build room
        startFirstRoom();
    }
#endif

#if defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_IOS_SIMULATOR)
    if (!mIsInForeground)
    {
        mTimeline.nextFrame();
        return;
    }
#endif
    
    // Update input events
    InputManager::singleton->updateMouseStates();

#ifndef CORRADE_TARGET_ANDROID
    InputManager::singleton->updateKeyStates();
#endif

    // Compute delta time
    mDeltaTime = mTimeline.previousFrameDuration();

#if DEBUG
	if (InputManager::singleton->mKeyStates[ImKeyButtons::RightCtrl] >= IM_STATE_PRESSED)
	{
		mDeltaTime *= 0.25f;
	}
#endif

    // Advance frame time
    mFrameTime += mDeltaTime;
    const bool canDraw = mFrameTime >= mDrawFrameTime;

    // Iterate through all layers
    for (const auto& index : GO_LAYERS)
    {
        // Get framebufferf for this buffer
        RoomManager::singleton->setCurrentBoundParentIndex(index);
        mCurrentGol = &RoomManager::singleton->mGoLayers[index];

        const bool canDrawLayer = canDraw && mCurrentGol->drawEnabled;

        // Do operations on framebuffer only if drawing for it is enabled
        if (canDrawLayer)
        {
            if (mCurrentGol->depthTestEnabled)
            {
                (*mCurrentGol->frameBuffer)
                    .clear(GL::FramebufferClear::Depth | GL::FramebufferClear::Stencil);
            }

            // Multi-layer color attachment clearing
            {
                mCurrentGol->frameBuffer->bind();
                mCurrentGol->frameBuffer->clearColor(GLF_COLOR_ATTACHMENT_INDEX, Color4(0.0f, 0.0f, 0.0f, 0.0f));
            }

            /*
				Read after binding, since OpenGL buffer *may* be
				flushed, and `glReadPixels` causes another flush.
			*/
            if (index == GOL_PERSP_FIRST)
            {
                if (InputManager::singleton->mReadObjectId)
                {
#ifdef TARGET_MOBILE
                    const auto& lbs = InputManager::singleton->mMouseStates[PRIMARY_BUTTON];
                    if (lbs >= IM_STATE_PRESSED)
#endif
                    {
                        // Get clicked Object ID
                        mCurrentGol->frameBuffer->mapForRead(GL::Framebuffer::ColorAttachment{ GLF_OBJECTID_ATTACHMENT_INDEX });

                        const Vector2i position(Vector2(InputManager::singleton->mMousePosition) * CommonUtility::singleton->mFramebufferSize / Vector2{ windowSize() });
                        const Vector2i fbPosition{ position.x(), mCachedFramebufferSize.y() - position.y() - 1 };

                        const Image2D data = mCurrentGol->frameBuffer->read(
                                Range2Di::fromSize(fbPosition, { 1, 1 }),
                                { PixelFormat::R32UI }
                        );

                        InputManager::singleton->mClickedObjectId = data.pixels<UnsignedInt>()[0][0];

                        // Clear object ID buffer
                        (*mCurrentGol->frameBuffer)
                                .clearColor(GLF_OBJECTID_ATTACHMENT_INDEX, Vector4ui{});
                    }
#ifdef TARGET_MOBILE
                    else if (lbs == IM_STATE_NOT_PRESSED)
                    {
                        InputManager::singleton->mClickedObjectId = 0U;
                    }
#endif
                }
                else
                {
                    InputManager::singleton->mClickedObjectId = 0U;
                }
            }
        }

        // Set renderer features
        GL::Renderer::enable(GL::Renderer::Feature::Blending);
        GL::Renderer::setFeature(GL::Renderer::Feature::DepthTest, mCurrentGol->depthTestEnabled);
        GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha, GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::One);

        // Set projection for camera on this layer
        RoomManager::singleton->mCamera->setProjectionMatrix(mCurrentGol->projectionMatrix);

        // Position camera on this layer
        RoomManager::singleton->mCameraObject.setTransformation(Matrix4::lookAt(mCurrentGol->cameraEye, mCurrentGol->cameraTarget, Vector3::yAxis()));

        // Get vector as reference
        const auto& gos = mCurrentGol->list;

        // Update all game objects on this layer
        if (mCurrentGol->updateEnabled)
        {
            for (auto& go : *gos)
            {
                go->mDeltaTime = mDeltaTime;
                go->update();
            }

            // Destroy all marked objects as such on this layer
            for (auto it = gos->begin(); it != gos->end();)
            {
                if ((*it)->mDestroyMe)
                {
                    it = gos->erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }

        // Draw all game objects on this layer
        if (canDrawLayer)
        {
            drawInternal();
        }
    }
    
    // De-reference game object layer
    mCurrentGol = nullptr;

    // Redraw main frame buffer
    RoomManager::singleton->mCamera->setViewport(framebufferSize());
    RoomManager::singleton->setCurrentBoundParentIndex(-1);

    {
        // Bind default window framebuffer
#if defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_IOS_SIMULATOR)
        
        mIosDefaultFramebuffer->bind();
        glBindRenderbuffer(GL_RENDERBUFFER, mIosDefaultRenderbufferId);
        GL::Context::current().resetState(GL::Context::State::Framebuffers);
        
#else
        GL::defaultFramebuffer.bind();
#endif

        // Set renderer features
        GL::Renderer::disable(GL::Renderer::Feature::Blending);
        GL::Renderer::disable(GL::Renderer::Feature::DepthTest);
        GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha, GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::One);

        // Redraw
        if (canDraw)
        {
            drawInternal();
        }
    }

    // Reset frame time, if required
    if (canDraw)
    {
        mFrameTime = 0.0f;
    }

    // Advance timeline
    mTimeline.nextFrame();
}

void Engine::pauseApp()
{
#ifdef CORRADE_TARGET_ANDROID
    pauseContext();
#endif
    
#ifdef TARGET_MOBILE
    mIsInForeground = false;
    RoomManager::singleton->pauseApp();
#endif
}

void Engine::resumeApp()
{
#ifdef CORRADE_TARGET_ANDROID
    resumeContext(ENGINE_CONFIGURATION.setSize({ 0, 0 }));
#endif
    
#ifdef TARGET_MOBILE
    mIsInForeground = true;
    RoomManager::singleton->resumeApp();
#endif
}

void Engine::drawEvent()
{
#ifdef CORRADE_TARGET_ANDROID
    if (mIsInForeground)
    {
        // Engine loop
        tickEvent();
        redraw();
    }
    else
    {
        mTimeline.nextFrame();
    }
#endif
}

void Engine::drawInternal()
{
    // Process main frame buffer
    if (mCurrentGol == nullptr)
    {
        // Draw screen quad
        mScreenQuadShader
                .bindColorTexture(GOL_PERSP_FIRST, *RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST].colorTexture)
                .bindColorTexture(GOL_PERSP_SECOND, *RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].colorTexture)
                .bindColorTexture(GOL_ORTHO_FIRST, *RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].colorTexture)
                // .bindDepthStencilTexture(GOL_PERSP_FIRST, *RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST].depthTexture)
                .draw(mScreenQuadShader.mMesh);

        // Swap buffers
        swapBuffers();
    }
    else
    {
        if (mCurrentGol->orderingByZ)
        {
            // Z ordering
            std::vector<std::pair<std::reference_wrapper<SceneGraph::Drawable3D>, Matrix4>> drawableTransformations = RoomManager::singleton->mCamera->drawableTransformations(*mCurrentGol->drawables);
            std::sort(drawableTransformations.begin(), drawableTransformations.end(),
                [](const std::pair<std::reference_wrapper<SceneGraph::Drawable3D>, Matrix4>& a,
                    const std::pair<std::reference_wrapper<SceneGraph::Drawable3D>, Matrix4>& b) {
                return a.second.translation().y() < b.second.translation().y();
            });

            // Draw scene
            RoomManager::singleton->mCamera->draw(drawableTransformations);
        }
        else
        {
            // Draw scene
            RoomManager::singleton->mCamera->draw(*mCurrentGol->drawables);
        }
    }
}

void Engine::mousePressEvent(MouseEvent& event)
{
    // Update state for pressed mouse button
    updateMouseButtonState(event, true);

    // Capture event
    event.setAccepted();
}

void Engine::mouseReleaseEvent(MouseEvent& event)
{
    // Update state for released mouse button
    updateMouseButtonState(event, false);

    // Capture event
    event.setAccepted();
}

void Engine::mouseMoveEvent(MouseMoveEvent& event)
{
    /*
    if (!(event.buttons() & MouseMoveEvent::Button::Left))
        return;
    */

    // Update state for all mouse buttons
    updateMouseButtonStates(event);
    // Capture event
    event.setAccepted();
}

void Engine::viewportEvent(ViewportEvent& event)
{
    viewportInternal(&event);
}

void Engine::viewportInternal(ViewportEvent* event)
{
    // Update viewports
#if defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_IOS_SIMULATOR)
    mIosDefaultFramebuffer->setViewport(Range2Di({ 0, 0 }, framebufferSize()));
#else
    GL::defaultFramebuffer.setViewport(Range2Di({ 0, 0 }, framebufferSize()));
#endif
    
    CommonUtility::singleton->mFramebufferSize = Vector2(framebufferSize());
    mCachedFramebufferSize = Vector2i(CommonUtility::singleton->mFramebufferSize);

    RoomManager::singleton->setWindowSize(Vector2(windowSize()));
    RoomManager::singleton->mCamera->setViewport(mCachedFramebufferSize);

#if defined(CORRADE_TARGET_WINDOWS) || defined(CORRADE_TARGET_WINDOWS_RT) || defined(CORRADE_TARGET_APPLE)
#if !defined(CORRADE_TARGET_IOS) && !defined(CORRADE_TARGET_IOS_SIMULATOR)
    CommonUtility::singleton->mConfig.displayDensity = Math::max(1.0f, Math::round(mCachedFramebufferSize.y() / 256.0f) * 0.5f - 1.0f);
#endif
#endif
    
    // Update layers
    upsertGameObjectLayers();
    
    // Notify to room manager
    RoomManager::singleton->viewportChange(event);
}

#ifndef CORRADE_TARGET_ANDROID
void Engine::keyPressEvent(KeyEvent& event)
{
    // Update state for pressed key button
    updateKeyButtonState(event, true);

    // Capture event
    event.setAccepted();
}

void Engine::keyReleaseEvent(KeyEvent& event)
{
    // Update state for released key button
    updateKeyButtonState(event, false);

    // Capture event
    event.setAccepted();
}
#endif

#ifndef CORRADE_TARGET_ANDROID
void Engine::exitEvent(ExitEvent& event)
{
    exitInternal(&event);
}

void Engine::anyEvent(SDL_Event& event)
{
#if defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_IOS_SIMULATOR)
    switch(event.type)
    {
    case SDL_APP_WILLENTERBACKGROUND:
        ios_WillEnterBackground();
        pauseApp();
        break;
            
    case SDL_APP_DIDENTERFOREGROUND:
        ios_DidEnterForeground();
        resumeApp();
        break;
            
    case SDL_APP_TERMINATING:
        exitInternal(nullptr);
        break;
    }
#endif
}
#endif

void Engine::exitInternal(void* arg)
{
    /*
        Clear the entire room. Must be done now, because this
        object holds data about game objects, and so they holds
        references about meshes, textures, shaders, etc...
    */
    RoomManager::singleton->clear();
    RoomManager::singleton = nullptr;

    /*
        Now, common utility can be cleared, expecially because
        it contains the resource manager. It can be cleared now,
        and only now, because no more references are present
        for contained resources.
    */
    CommonUtility::singleton = nullptr;

    // Clear input manager
    InputManager::singleton = nullptr;

#ifndef CORRADE_TARGET_ANDROID
    // Pass default behaviour
    if (arg != nullptr)
    {
        ((ExitEvent*)arg)->setAccepted();
    }
#endif
}

void Engine::startFirstRoom()
{
    mTimeline.start();
    mScreenQuadShader.setup();
    RoomManager::singleton->loadRoom("intro");
}

void Engine::upsertGameObjectLayers()
{
    // Cycle through all required layers
    for (const auto& index : GO_LAYERS)
    {
        // Check if layer has been already created
        RoomManager::GameObjectsLayer* layer;
        {
            // Check if layer exists
            const auto& it = RoomManager::singleton->mGoLayers.find(index);
            if (it != RoomManager::singleton->mGoLayers.end())
            {
                // Layer exists
                layer = &it->second;
                
                // Avoid recreating if viewport did not change its size
                if (layer->frameBuffer->viewport().max() == mCachedFramebufferSize)
                {
                    continue;
                }

                // Clear framebuffer
                layer->frameBuffer = nullptr;
            }
            else
            {
                // Create layer
                RoomManager::singleton->mGoLayers[index] = RoomManager::GameObjectsLayer();
                layer = &RoomManager::singleton->mGoLayers[index];
                layer->index = index;

                // Create game object list
                layer->list = std::make_unique<GameObjectList>();

                // Create drawables holder
                layer->drawables = std::make_unique<SceneGraph::DrawableGroup3D>();

                // Special setup
                layer->updateEnabled = true;
                layer->drawEnabled = true;

                if (index == GOL_ORTHO_FIRST)
                {
                    // Set renderer features
                    layer->depthTestEnabled = false;
                    layer->orderingByZ = false;

                    // Set camera position
                    layer->cameraEye = { 0.0f, 0.0f, 1.0f };
                    layer->cameraTarget = { 0.0f, 0.0f, 0.0f };
                }
                else
                {
                    // Set renderer features
                    layer->depthTestEnabled = true;
                    // layer->orderingByZ = true;
                    layer->orderingByZ = false;
                }
            }
        }

        // Set projection matrix
        {
            const auto& v = RoomManager::singleton->getWindowSize();
            const auto& pm = index == GOL_ORTHO_FIRST ?
                Matrix4::orthographicProjection(Vector2(1.0f), 0.01f, 100.0f) :
                Matrix4::perspectiveProjection(35.0_degf, v.aspectRatio(), 0.01f, 1000.0f);
            layer->projectionMatrix = pm;
        }

        // Create framebuffer and attach color buffers
        layer->frameBuffer = std::make_unique<GL::Framebuffer>(Range2Di({}, mCachedFramebufferSize));

        {
            // Create main texture to attach layer
            layer->colorTexture = std::make_unique<GL::Texture2D>();
            layer->colorTexture->setStorage(1, GL::TextureFormat::RGBA8, mCachedFramebufferSize);
            layer->frameBuffer->attachTexture(GL::Framebuffer::ColorAttachment{ GLF_COLOR_ATTACHMENT_INDEX }, *layer->colorTexture, 0);
            // layer->frameBuffer->attachRenderbuffer(GL::Framebuffer::ColorAttachment{ GLF_COLOR_ATTACHMENT_INDEX }, colorBuffer);
        }

        // Attach depth buffers only for 3D layers
        if (layer->depthTestEnabled)
        {
            layer->depthTexture = std::make_unique<GL::Texture2D>();
            layer->depthTexture->setStorage(1, GL::TextureFormat::Depth24Stencil8, mCachedFramebufferSize);
            layer->depthTexture->setCompareMode(GL::SamplerCompareMode::None);
            layer->depthTexture->setMagnificationFilter(Magnum::SamplerFilter::Nearest);
            layer->depthTexture->setMagnificationFilter(Magnum::SamplerFilter::Nearest);
            layer->frameBuffer->attachTexture(GL::Framebuffer::BufferAttachment::DepthStencil, *layer->depthTexture, 0);
        }

        // Attach Object ID buffer, but only for "Perspective First"
        if (index == GOL_PERSP_FIRST)
        {
            layer->objectIdBuffer = std::make_unique<GL::Renderbuffer>();
            layer->objectIdBuffer->setStorage(GL::RenderbufferFormat::R32UI, mCachedFramebufferSize);

            layer->frameBuffer->attachRenderbuffer(GL::Framebuffer::ColorAttachment{ GLF_OBJECTID_ATTACHMENT_INDEX }, *layer->objectIdBuffer);
            layer->frameBuffer->mapForDraw({
                { Shaders::Phong::ColorOutput, GL::Framebuffer::ColorAttachment{ GLF_COLOR_ATTACHMENT_INDEX } },
                { Shaders::Phong::ObjectIdOutput, GL::Framebuffer::ColorAttachment{ GLF_OBJECTID_ATTACHMENT_INDEX } }
            });
        }
        else
        {
            layer->frameBuffer->mapForDraw({
               { Shaders::Phong::ColorOutput, GL::Framebuffer::ColorAttachment{ GLF_COLOR_ATTACHMENT_INDEX } }
           });
        }

        // Check for framebuffer status
        CORRADE_INTERNAL_ASSERT(layer->frameBuffer->checkStatus(GL::FramebufferTarget::Draw) == GL::Framebuffer::Status::Complete);
    }
}

void Engine::updateMouseButtonState(MouseEvent& event, const bool & pressed)
{
    // Update state for the button which triggered the event
    InputManager::singleton->mMousePosition = event.position();

    InputManager::singleton->setMouseState(event.button(), pressed);
}

void Engine::updateMouseButtonStates(MouseMoveEvent& event)
{
    // Get current mouse position
    InputManager::singleton->mMousePosition = event.position();
    
#if defined(CORRADE_TARGET_ANDROID)
    InputManager::singleton->setMouseState(Platform::Application::MouseEvent::Button::None, true);
#else
    // Get pressed buttons for this mouse move event
    const auto& mouseButtons = event.buttons();

    // Check if left button is actually pressed
    const auto& value = mouseButtons & MouseMoveEvent::Button::Left;
    InputManager::singleton->setMouseState(ImMouseButtons::Left, value ? true : false);
#endif
}

#ifndef CORRADE_TARGET_ANDROID
void Engine::updateKeyButtonState(const KeyEvent& event, const bool & pressed)
{
    // Update state for the button which triggered the event
    InputManager::singleton->setKeyState(event.key(), pressed);
}
#endif
