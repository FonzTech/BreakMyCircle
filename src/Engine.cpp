#include "Engine.h"
#include "Common/CommonUtility.h"
#include "Audio/StreamedAudioBuffer.h"
#include "Game/OverlayText.h"
#include "InputManager.h"
#include "RoomManager.h"
#include "GameObject.h"

#ifdef CORRADE_TARGET_ANDROID
#define DEBUG_OPENGL_CALLS
#include <android/native_activity.h>
using namespace std::chrono_literals;
#endif

#ifdef DEBUG_OPENGL_CALLS
#include <Magnum/GL/DebugOutput.h>
#endif

using namespace Magnum::Math::Literals;

const Float Engine::mDrawFrameTime =
#ifdef CORRADE_TARGET_ANDROID
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

#ifdef ENABLE_DETACHED_DRAWING_FOR_OVERLAY_TEXT
const std::unordered_set<Int> Engine::GO_DRAW_DETACHED = {
	GOT_OVERLAY_TEXT
};
#endif

Engine::Engine(const Arguments& arguments) : Platform::Application{ arguments, Configuration{}.setTitle("BreakMyCircle") }, mFrameTime(0.0f)
{
	// Setup window
#ifdef MAGNUM_SDL2APPLICATION_MAIN
	setWindowSize({ 432, 768 });
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

    // Start timeline
    mTimeline.start();

	// Init common utility
	CommonUtility::singleton = std::make_unique<CommonUtility>();

#ifdef CORRADE_TARGET_ANDROID
    JNIEnv *env;
    nativeActivity()->vm->AttachCurrentThread(&env, 0);

    jobject me = nativeActivity()->clazz;

    jclass acl = env->GetObjectClass(me); // Class pointer of NativeActivity
    jmethodID giid = env->GetMethodID(acl, "getIntent", "()Landroid/content/Intent;");
    jobject intent = env->CallObjectMethod(me, giid); // Got our intent

    jclass icl = env->GetObjectClass(intent); // Class pointer of Intent
    jmethodID gseid = env->GetMethodID(icl, "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");

    const std::array<std::string, 3> params = { "asset_dir", "canvas_vertical_height", "density" };
    for (UnsignedInt i = 0; i != params.size(); ++i)
    {
        const auto jsParam1 = (jstring) env->CallObjectMethod(intent, gseid, env->NewStringUTF(params.at(i).c_str()));
        const char *Param1 = env->GetStringUTFChars(jsParam1, nullptr);

        const auto& value = std::string(Param1);

        switch (i)
        {
        case 0U:
            CommonUtility::singleton->mConfig.assetDir = value;
            break;

        case 1U:
            CommonUtility::singleton->mConfig.canvasVerticalPadding = std::stof(value);
            break;

        case 2U:
            CommonUtility::singleton->mConfig.displayDensity = std::stof(value);
            break;
        }

        env->ReleaseStringUTFChars(jsParam1, Param1);
    }
#endif

    Debug{} << "Asset base directory is" << CommonUtility::singleton->mConfig.assetDir;

	// Setup screen quad shader (after the CommonUtility has started)
	mScreenQuadShader.setup();

	// Init input manager
	InputManager::singleton = std::make_unique<InputManager>();

	// Init room manager
	RoomManager::singleton = std::make_unique<RoomManager>();
	RoomManager::singleton->setup();

	// Setup room manager
	RoomManager::singleton->setWindowSize(Vector2(windowSize()));
	viewportInternal(nullptr);

	// Build room
	RoomManager::singleton->loadRoom("intro");
	// RoomManager::singleton->createLevelRoom();
}

Engine::~Engine()
{
#ifdef CORRADE_TARGET_ANDROID
	exitInternal(nullptr);
#endif
}

void Engine::tickEvent()
{
	// Update input events
	InputManager::singleton->updateMouseStates();

#ifndef CORRADE_TARGET_ANDROID
	InputManager::singleton->updateKeyStates();
#endif

	// Compute delta time
	mDeltaTime = mTimeline.previousFrameDuration();

	// Advance frame time
	mFrameTime += mDeltaTime;
	const bool canDraw = mFrameTime >= mDrawFrameTime;

	// Set correct viewport
	RoomManager::singleton->setWindowSize(Vector2(windowSize()));
	RoomManager::singleton->mCamera->setViewport(mScaledFramebufferSize);

	// Iterate through all layers
	for (const auto& index : GO_LAYERS)
	{
		// Get framebufferf for this buffer
		RoomManager::singleton->setCurrentBoundParentIndex(index);
		currentGol = &RoomManager::singleton->mGoLayers[index];

		const bool canDrawLayer = canDraw && currentGol->drawEnabled;

		// Do operations on framebuffer only if drawing for it is enabled
		if (canDrawLayer)
        {
			if (currentGol->depthTestEnabled)
			{
				(*currentGol->frameBuffer)
					.clear(GL::FramebufferClear::Depth | GL::FramebufferClear::Stencil);
			}

			(*currentGol->frameBuffer)
				.clearColor(GLF_COLOR_ATTACHMENT_INDEX, Color4(0.0f, 0.0f, 0.0f, 0.0f))
				.bind();

			/*
			 * Read after binding, since OpenGL buffer *may* be
			 * flushed, and `glReadPixels` causes another flush.
			 */
			if (index == GOL_PERSP_FIRST)
			{
#ifdef CORRADE_TARGET_ANDROID
				const auto& lbs = InputManager::singleton->mMouseStates[PRIMARY_BUTTON];
				if (lbs >= IM_STATE_PRESSED)
#endif
				{
					// Get clicked Object ID
					currentGol->frameBuffer->mapForRead(GL::Framebuffer::ColorAttachment{ GLF_OBJECTID_ATTACHMENT_INDEX });

					const Vector2i position(Vector2(InputManager::singleton->mMousePosition) * Vector2 { mScaledFramebufferSize } / Vector2{ windowSize() });
					const Vector2i fbPosition{ position.x(), mScaledFramebufferSize.y() - position.y() - 1 };

					const Image2D data = currentGol->frameBuffer->read(
							Range2Di::fromSize(fbPosition, { 1, 1 }),
							{ PixelFormat::R32UI }
					);

					InputManager::singleton->mClickedObjectId = data.pixels<UnsignedInt>()[0][0];

					// Clear object ID buffer
					(*currentGol->frameBuffer)
							.clearColor(GLF_OBJECTID_ATTACHMENT_INDEX, Vector4ui{});
				}
#ifdef CORRADE_TARGET_ANDROID
				else if (lbs == IM_STATE_NOT_PRESSED)
				{
					InputManager::singleton->mClickedObjectId = 0U;
				}
#endif
			}
        }

		// Set renderer features
		GL::Renderer::enable(GL::Renderer::Feature::Blending);
		GL::Renderer::setFeature(GL::Renderer::Feature::DepthTest, currentGol->depthTestEnabled);
		GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha, GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::One);

		// Set projection for camera on this layer
		RoomManager::singleton->mCamera->setProjectionMatrix(currentGol->projectionMatrix);

		// Position camera on this layer
		RoomManager::singleton->mCameraObject.setTransformation(Matrix4::lookAt(currentGol->cameraEye, currentGol->cameraTarget, Vector3::yAxis()));

		// Get vector as reference
		const auto& gos = currentGol->list;

		// Update all game objects on this layer
		if (currentGol->updateEnabled)
		{
            for (UnsignedInt i = 0; i != gos->size(); ++i)
            {
                std::shared_ptr<GameObject> & go = gos->at(i);
				go->mDeltaTime = mDeltaTime;
				go->update();
			}

			// Destroy all marked objects as such on this layer
			for (UnsignedInt i = 0; i < gos->size();)
			{
				std::shared_ptr<GameObject> & go = gos->at(i);
				if (go->mDestroyMe)
				{
					gos->erase(gos->begin() + i);
				}
				else
				{
					++i;
				}
			}
		}

		// Draw all game objects on this layer
#ifdef ENABLE_DETACHED_DRAWING_FOR_OVERLAY_TEXT
		GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
#endif

		if (canDrawLayer)
		{
			drawInternal();
		}

		// Check for special actions
#ifdef ENABLE_DETACHED_DRAWING_FOR_OVERLAY_TEXT
		if (index == GOL_ORTHO_FIRST)
		{
			// Set specific blend function for text
			GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
			GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);

			// Get vector as reference
			const auto& gos = currentGol->list;

			// Update all game objects on this layer
			for (UnsignedInt i = 0; i < gos->size(); ++i)
			{
				std::shared_ptr<GameObject> & go = gos->at(i);
				if (GO_DRAW_DETACHED.find(go->getType()) != GO_DRAW_DETACHED.end())
				{
					((std::shared_ptr<IDrawDetached>&)go)->drawDetached();
				}
			}
		}
#endif

		// De-reference game object layer
		currentGol = nullptr;
	}

	// Redraw main frame buffer
	RoomManager::singleton->mCamera->setViewport(framebufferSize());
	RoomManager::singleton->setCurrentBoundParentIndex(-1);

	{
		// Bind default window framebuffer
		GL::defaultFramebuffer.bind();

		// Set renderer features
		GL::Renderer::disable(GL::Renderer::Feature::Blending);
		GL::Renderer::disable(GL::Renderer::Feature::DepthTest);
		GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha, GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::One);

		// Redraw
#ifdef ENABLE_DETACHED_DRAWING_FOR_OVERLAY_TEXT
		GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
#endif
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

void Engine::drawEvent()
{
#ifdef CORRADE_TARGET_ANDROID
    tickEvent();
    redraw();
#endif
}

void Engine::drawInternal()
{
	// Process main frame buffer
	if (currentGol == nullptr)
	{
		// Draw screen quad
		mScreenQuadShader
				.bindColorTexture(GOL_PERSP_FIRST, *RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST].colorTexture)
				.bindColorTexture(GOL_PERSP_SECOND, *RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].colorTexture)
				.bindColorTexture(GOL_ORTHO_FIRST, *RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].colorTexture)
				.bindDepthStencilTexture(GOL_PERSP_FIRST, *RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST].depthTexture)
				.draw(mScreenQuadShader.mMesh);

		// Swap buffers
		swapBuffers();
	}
	else
	{
		if (currentGol->orderingByZ)
		{
			// Z ordering
			std::vector<std::pair<std::reference_wrapper<SceneGraph::Drawable3D>, Matrix4>> drawableTransformations = RoomManager::singleton->mCamera->drawableTransformations(*currentGol->drawables);
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
			RoomManager::singleton->mCamera->draw(*currentGol->drawables);
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
	GL::defaultFramebuffer.setViewport(Range2Di({ 0, 0 }, framebufferSize()));
	mScaledFramebufferSize = Vector2i(Vector2d(framebufferSize()) / CommonUtility::singleton->mConfig.displayDensity);
	upsertGameObjectLayers();
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
#endif

void Engine::exitInternal(void* arg)
{
	/*
		Clear the entire room. Must be done now, because this
		object holds data about game objects, and so they holds
		references about meshes, textures, shaders, etc...
	*/
	if (RoomManager::singleton != nullptr)
	{
		RoomManager::singleton->clear();
	}
	RoomManager::singleton = nullptr;

	/*
		Now, common utility can be cleared, expecially because
		it contains the resource manager. It can be cleared now,
		and only now, because no more references are present
		for contained resources.
	*/
	if (CommonUtility::singleton != nullptr)
	{
		CommonUtility::singleton->clear();
	}
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
				Matrix4::perspectiveProjection(35.0_degf, v.x() / v.y(), 0.01f, 1000.0f);
			layer->projectionMatrix = pm;
		}

		// Create framebuffer and attach color buffers
		layer->frameBuffer = std::make_unique<GL::Framebuffer>(Range2Di({}, mScaledFramebufferSize));

		{
			// Create main texture to attach layer
			layer->colorTexture = std::make_unique<GL::Texture2D>();
			layer->colorTexture->setStorage(1, GL::TextureFormat::RGBA8, mScaledFramebufferSize);
			layer->frameBuffer->attachTexture(GL::Framebuffer::ColorAttachment{ GLF_COLOR_ATTACHMENT_INDEX }, *layer->colorTexture, 0);
			// layer->frameBuffer->attachRenderbuffer(GL::Framebuffer::ColorAttachment{ GLF_COLOR_ATTACHMENT_INDEX }, colorBuffer);
		}

		// Attach depth buffers only for 3D layers
		if (layer->depthTestEnabled)
		{
			layer->depthTexture = std::make_unique<GL::Texture2D>();
			layer->depthTexture->setStorage(1, GL::TextureFormat::Depth24Stencil8, mScaledFramebufferSize);
			layer->depthTexture->setCompareMode(GL::SamplerCompareMode::None);
			layer->depthTexture->setMagnificationFilter(Magnum::SamplerFilter::Nearest);
			layer->depthTexture->setMagnificationFilter(Magnum::SamplerFilter::Nearest);
			layer->frameBuffer->attachTexture(GL::Framebuffer::BufferAttachment::DepthStencil, *layer->depthTexture, 0);
		}

		// Attach Object ID buffer, but only for "Perspective First"
		if (index == GOL_PERSP_FIRST)
		{
            layer->objectIdBuffer = std::make_unique<GL::Renderbuffer>();
            layer->objectIdBuffer->setStorage(GL::RenderbufferFormat::R32UI, mScaledFramebufferSize);

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

#ifdef CORRADE_TARGET_ANDROID
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