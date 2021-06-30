#include "Scenery.h"

#include <Corrade/Containers/LinkedList.h>

#include "AssetManager.h"
#include "RoomManager.h"
#include "CommonUtility.h"
#include "InputManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> Scenery::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Sint8 parent;
	params.at("parent").get_to(parent);

	// Instantiate scenery object
	std::shared_ptr<Scenery> p = std::make_shared<Scenery>(parent);
	return p;
}

Scenery::Scenery(const Sint8 parentIndex) : GameObject(parentIndex)
{
	// Init members
	mCubicBezier = std::make_unique<CubicBezier2D>(Vector2(0.0f, 0.0f), Vector2(0.11f, -0.02f), Vector2(0.0f, 1.01f), Vector2(1.0f));
	mFrame = 0.0f;

	// Create manipulator list
	mManipulatorList.emplace_back(mManipulator.get());

	// Load assets
	{
		AssetManager am(RESOURCE_SHADER_COLORED_PHONG_2, RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE_2, 2);
		am.loadAssets(*this, *mManipulatorList[0], "scenes/world_1.glb", this);
	}

	// Set camera position for scenery
	position = Vector3(0.0f);

	auto& p = RoomManager::singleton->mGoLayers[GOL_FIRST];
	p.mCameraEye = Vector3(7.65094f, 11.6036f, 11.9944f);
	p.mCameraTarget = Vector3(0.0f, 0.0f, 0.0f);
}

const Int Scenery::getType() const
{
	return GOT_SCENERY;
}

void Scenery::update()
{
	// Update frame
	mFrame += mDeltaTime;

	// Debug camera move
#ifdef _DEBUG or NDEBUG
	{
		Vector3 delta;

		if (InputManager::singleton->mKeyStates[ImKeyButtons::Left] >= IM_STATE_PRESSED)
		{
			delta += Vector3(-1.0f, 0.0f, 0.0f);
		}

		if (InputManager::singleton->mKeyStates[ImKeyButtons::Right] >= IM_STATE_PRESSED)
		{
			delta += Vector3(1.0f, 0.0f, 0.0f);
		}

		if (InputManager::singleton->mKeyStates[ImKeyButtons::Up] >= IM_STATE_PRESSED)
		{
			delta += Vector3(0.0f, 1.0f, 0.0f);
		}

		if (InputManager::singleton->mKeyStates[ImKeyButtons::Down] >= IM_STATE_PRESSED)
		{
			delta += Vector3(0.0f, -1.0f, 0.0f);
		}

		if (InputManager::singleton->mKeyStates[ImKeyButtons::LeftShift] >= IM_STATE_PRESSED)
		{
			delta += Vector3(0.0f, 0.0f, -1.0f);
		}

		if (InputManager::singleton->mKeyStates[ImKeyButtons::LeftCtrl] >= IM_STATE_PRESSED)
		{
			delta += Vector3(0.0f, 0.0f, 1.0f);
		}

		const bool isEye = InputManager::singleton->mKeyStates[ImKeyButtons::Tab] >= IM_STATE_PRESSED;

		auto& p1 = RoomManager::singleton->mGoLayers[GOL_FIRST];
		auto* p2 = isEye ? &p1.mCameraEye : &p1.mCameraTarget;
		*p2 += delta * mDeltaTime * 10.0f;

		printf("%f %f %f - %f %f %f\n", p1.mCameraEye[0], p1.mCameraEye[1], p1.mCameraEye[2], p1.mCameraTarget[0], p1.mCameraTarget[1], p1.mCameraTarget[2]);
	}
#endif

	// Animation for eye camera
	{
		auto* p = &RoomManager::singleton->mGoLayers[GOL_SECOND].mCameraEye[2];
		*p = mCubicBezier->value(Math::min(mFrame * 0.25f, 1.0f))[1] * 44.0f;
	}

	// Apply transformations
	{
		const auto& m = Matrix4::translation(position);
		mManipulatorList[0]->setTransformation(m);
	}
}

void Scenery::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	// Create array of transformed light positions
	std::vector<Vector3> destLightPos;
	std::vector<Color4> destLightColors;

	{
		// Create array of light positions
		std::vector<Vector3> source;

		source.emplace_back(8.0f, 20.0f, 10.0f);
		source.emplace_back(8.0f, -40.0f, 10.0f);

		destLightColors.emplace_back(0xffffff00_rgbaf);
		destLightColors.emplace_back(0xffffff00_rgbaf);

		// Create map function
		auto mapFx = [&](const decltype(source)::value_type & vector)
		{
			return camera.cameraMatrix().transformPoint(position + vector);
		};

		// Apply array mapping to all its elements
		std::transform(source.begin(), source.end(), std::back_inserter(destLightPos), mapFx);
	}

	// Shader through shader
	Shaders::Phong& shader = (Shaders::Phong&) baseDrawable->getShader();
	shader
		.setLightPositions(destLightPos)
		.setLightColors(destLightColors)
		.setSpecularColor(0xffffff00_rgbaf)
		.setAmbientColor(0x444444ff_rgbaf)
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix());

	if (baseDrawable->mTexture != nullptr)
	{
		shader.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr);
	}

	shader.draw(*baseDrawable->mMesh);
}

void Scenery::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}