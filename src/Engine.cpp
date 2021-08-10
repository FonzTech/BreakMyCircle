#include <memory>
#include <Magnum/Image.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/GL/RenderbufferFormat.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/TextureFormat.h>

#include "Engine.h"
#include "Common/CommonUtility.h"
#include "Audio/StreamedAudioBuffer.h"
#include "Game/OverlayText.h"
#include "InputManager.h"
#include "RoomManager.h"
#include "GameObject.h"

const Int Engine::GO_LAYERS[] = {
	GOL_PERSP_FIRST,
	GOL_PERSP_SECOND,
	GOL_ORTHO_FIRST
};

const std::unordered_set<Int> Engine::INTRINSIC_GAME_OBJECTS = {
	GOT_OVERLAY_TEXT
};

Engine::Engine(const Arguments& arguments) : Platform::Application{ arguments, Configuration{}.setTitle("BreakMyCircle") }
{
	// Setup window
	#ifdef MAGNUM_SDL2APPLICATION_MAIN
	setWindowSize({ 432, 768 });
	#endif

	// Start timeline
	mTimeline.start();

	// Enable renderer features
	// GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
	GL::Renderer::enable(GL::Renderer::Feature::Blending);
	// GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
	GL::Renderer::setBlendEquation(GL::Renderer::BlendEquation::Add, GL::Renderer::BlendEquation::Add);

	// Set clear color
	GL::Renderer::setClearColor(Color4(0.0f, 0.0f, 0.0f, 0.0f));

	// Init common utility
	CommonUtility::singleton = std::make_unique<CommonUtility>();

	// Init input manager
	InputManager::singleton = std::make_unique<InputManager>();

	// Init room manager
	RoomManager::singleton = std::make_unique<RoomManager>();
	RoomManager::singleton->setup();

	// Setup room manager
	RoomManager::singleton->mWindowSize = windowSize();
	upsertGameObjectLayers();

	// Build room
	RoomManager::singleton->loadRoom("intro");
	// RoomManager::singleton->createLevelRoom();
}

void Engine::tickEvent()
{
	// Update input events
	InputManager::singleton->updateMouseStates();
	InputManager::singleton->updateKeyStates();
	InputManager::singleton->mClickedObjectId = 0;

	// Compute delta time
	mDeltaTime = mTimeline.previousFrameDuration();

	auto ws = windowSize();
	RoomManager::singleton->mWindowSize = ws;
	RoomManager::singleton->mCamera->setViewport(ws);

	// Iterate through all layers
	for (const auto& index : GO_LAYERS)
	{
		// Get layer's framebuffer
		currentGol = &RoomManager::singleton->mGoLayers[index];

		if (index == GOL_PERSP_FIRST)
		{
			// Get clicked Object ID
			currentGol->frameBuffer->mapForRead(GL::Framebuffer::ColorAttachment{ GLF_OBJECTID_ATTACHMENT_INDEX });

			{
				const Vector2i position(Vector2(InputManager::singleton->mMousePosition) * Vector2 { framebufferSize() } / Vector2{ windowSize() });
				const Vector2i fbPosition{ position.x(), GL::defaultFramebuffer.viewport().sizeY() - position.y() - 1 };

				const Image2D data = currentGol->frameBuffer->read(
					Range2Di::fromSize(fbPosition, { 1, 1 }),
					{ PixelFormat::R32UI }
				);

				InputManager::singleton->mClickedObjectId = data.pixels<UnsignedInt>()[0][0];
			}

			// Restore old color attachment
			currentGol->frameBuffer->mapForRead(GL::Framebuffer::ColorAttachment{ GLF_COLOR_ATTACHMENT_INDEX });

			// Clear object ID buffer
			(*currentGol->frameBuffer)
				.clearColor(GLF_OBJECTID_ATTACHMENT_INDEX, Vector4ui{});
		}

		(*currentGol->frameBuffer)
			.clear(GL::FramebufferClear::Depth)
			.clearColor(GLF_COLOR_ATTACHMENT_INDEX, Color4(0.0f, 0.0f, 0.0f, 0.0f))
			.bind();

		// Enable renderer features
		GL::Renderer::setFeature(GL::Renderer::Feature::DepthTest, currentGol->depthTestEnabled);

		// Set projection for camera on this layer
		RoomManager::singleton->mCamera->setProjectionMatrix(currentGol->projectionMatrix);

		// Position camera on this layer
		RoomManager::singleton->mCameraObject.setTransformation(Matrix4::lookAt(currentGol->cameraEye, currentGol->cameraTarget, Vector3::yAxis()));

		// Get vector as reference
		const auto& gos = currentGol->list;

		// Update all game objects on this layer
		for (UnsignedInt i = 0; i < gos->size(); ++i)
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

		// Draw all game objects on this layer
		GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
		drawEvent();

		// Check for special actions
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
				if (INTRINSIC_GAME_OBJECTS.find(go->getType()) != INTRINSIC_GAME_OBJECTS.end())
				{
					((std::shared_ptr<OverlayText>&)go)->drawDetached();
				}
			}
		}

		// De-reference game object layer
		currentGol = nullptr;
	}

	// Redraw main frame buffer
	{
		// Bind default window framebuffer
		GL::defaultFramebuffer
			.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth)
			.bind();

		// Redraw
		GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::SourceAlpha, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
		drawEvent();
	}

	// Advance timeline
	mTimeline.nextFrame();
}

void Engine::drawEvent()
{
	// Process main frame buffer
	if (currentGol == nullptr)
	{
		// Draw screen quad
		mScreenQuadShader
			.bindTexture(GOL_PERSP_FIRST, *RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST].fbTexture)
			.bindTexture(GOL_PERSP_SECOND, *RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].fbTexture)
			.bindTexture(GOL_ORTHO_FIRST, *RoomManager::singleton->mGoLayers[GOL_ORTHO_FIRST].fbTexture)
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
				return a.second.translation().z() < b.second.translation().z();
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

void Engine::viewportEvent(ViewportEvent& event)
{
	// Update viewports
	GL::defaultFramebuffer.setViewport(Range2Di({ 0, 0 }, event.framebufferSize()));
	RoomManager::singleton->mCamera->setViewport(event.framebufferSize());
	upsertGameObjectLayers();
}

void Engine::exitEvent(ExitEvent& event)
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

	// Pass default behaviour
	event.setAccepted();
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
					layer->orderingByZ = true;
				}
			}
		}

		// Set projection matrix
		{
			const Vector2 v(RoomManager::singleton->mWindowSize);
			const auto& pm = index == GOL_ORTHO_FIRST ?
				Matrix4::orthographicProjection(Vector2(1.0f), 0.01f, 100.0f) :
				Matrix4::perspectiveProjection(35.0_degf, v.x() / v.y(), 0.01f, 1000.0f);
			layer->projectionMatrix = pm;
		}

		// Get size for window framebuffer
		const auto& size = GL::defaultFramebuffer.viewport().size();

		// Create main texture to attach layer
		layer->fbTexture = std::make_unique<GL::Texture2D>();
		layer->fbTexture->setStorage(1, GL::TextureFormat::RGBA8, size);

		GL::Renderbuffer colorBuffer;
		colorBuffer.setStorage(GL::RenderbufferFormat::RGBA8, size);

		// Create framebuffer and attach color buffers
		layer->frameBuffer = std::make_unique<GL::Framebuffer>(Range2Di({}, size));
		layer->frameBuffer->attachTexture(GL::Framebuffer::ColorAttachment{ GLF_COLOR_ATTACHMENT_INDEX }, *layer->fbTexture, 0);
		// layer->frameBuffer->attachRenderbuffer(GL::Framebuffer::ColorAttachment{ GLF_COLOR_ATTACHMENT_INDEX }, colorBuffer);

		// Attach depth buffers only for 3D layers
		if (layer->depthTestEnabled)
		{
			GL::Renderbuffer depthStencilBuffer;
			depthStencilBuffer.setStorage(GL::RenderbufferFormat::Depth24Stencil8, size);

			layer->frameBuffer->attachRenderbuffer(GL::Framebuffer::BufferAttachment::DepthStencil, depthStencilBuffer);
		}

		// Attach Object ID buffer, but only for "Perspective First"
		if (index == GOL_PERSP_FIRST)
		{
			GL::Renderbuffer objectIdBuffer;
			objectIdBuffer.setStorage(GL::RenderbufferFormat::R32UI, size);

			layer->frameBuffer->attachRenderbuffer(GL::Framebuffer::ColorAttachment{ GLF_OBJECTID_ATTACHMENT_INDEX }, objectIdBuffer);
			layer->frameBuffer->mapForDraw({
				{ Shaders::Phong::ColorOutput, GL::Framebuffer::ColorAttachment{ GLF_COLOR_ATTACHMENT_INDEX } },
				{ Shaders::Phong::ObjectIdOutput, GL::Framebuffer::ColorAttachment{ GLF_OBJECTID_ATTACHMENT_INDEX } }
				});
		}

		// Check for framebuffer status
		CORRADE_INTERNAL_ASSERT(layer->frameBuffer->checkStatus(GL::FramebufferTarget::Draw) == GL::Framebuffer::Status::Complete);
	}
}

void Engine::updateMouseButtonState(const MouseEvent& event, const bool & pressed)
{
	// Update state for the button which triggered the event
	InputManager::singleton->setMouseState(event.button(), pressed);
}

void Engine::updateMouseButtonStates(const MouseMoveEvent& event)
{
	// Get current mouse position
	InputManager::singleton->mMousePosition = event.position();

	// Get pressed buttons for this mouse move event
	const auto& mouseButtons = event.buttons();

	// Check if left button is actually pressed
	const auto& value = mouseButtons & MouseMoveEvent::Button::Left;
	InputManager::singleton->setMouseState(ImMouseButtons::Left, value ? true : false);
}

void Engine::updateKeyButtonState(const KeyEvent& event, const bool & pressed)
{
	// Update state for the button which triggered the event
	InputManager::singleton->setKeyState(event.key(), pressed);
}