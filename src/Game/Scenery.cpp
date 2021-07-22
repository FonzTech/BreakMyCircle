#include "Scenery.h"

#include <Corrade/Containers/LinkedList.h>
#include <Magnum/Primitives/Plane.h>
#include <Magnum/Primitives/Grid.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>

#include "../Common/CommonUtility.h"
#include "../AssetManager.h"
#include "../RoomManager.h"
#include "../InputManager.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

std::shared_ptr<GameObject> Scenery::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Get required data
	Int modelIndex;
	params.at("modelIndex").get_to(modelIndex);

	// Instantiate scenery object
	std::shared_ptr<Scenery> p = std::make_shared<Scenery>(parent, modelIndex);
	return p;
}

Scenery::Scenery(const Int parentIndex, const Int modelIndex) : GameObject(parentIndex)
{
	// Init members
	// mCubicBezier = std::make_unique<CubicBezier2D>(Vector2(0.0f, 0.0f), Vector2(0.11f, -0.02f), Vector2(0.0f, 1.01f), Vector2(1.0f));
	mModelIndex = modelIndex;
	mFrame = 0.0f;

	// Fill manipulator list
	mManipulatorList.push_back(new Object3D{ mManipulator.get() });
	mManipulatorList.push_back(new Object3D{ mManipulator.get() });

	// Apply transformations
	{
		const auto& m = Matrix4::translation(mPosition);
		mManipulatorList[0]->setTransformation(m);
	}

	{
		const auto& m = Matrix4::scaling(Vector3(100.0f));
		mManipulatorList[1]->setTransformation(m);
		mManipulatorList[1]->rotateX(90.0_degf);
		mManipulatorList[1]->translate(mPosition + Vector3(0.0f, 0.3f, 0.0f));
	}

	// Load assets
	{
		AssetManager am(RESOURCE_SHADER_COLORED_PHONG_2, RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE_2, 2);
		am.loadAssets(*this, *mManipulatorList[0], "scenes/world_1.glb", this);
	}

	// Create water drawable
	createWaterDrawable();

	// Set camera position for scenery
	mPosition = Vector3(0.0f);

	/*
	{
		auto& p = RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST];
		p.cameraEye = Vector3(7.65094f, 11.6036f, 11.9944f);
		p.cameraTarget = Vector3(0.0f, 0.0f, 0.0f);
	}
	*/
}

const Int Scenery::getType() const
{
	return GOT_SCENERY;
}

void Scenery::update()
{
	// Update frame
	mFrame += mDeltaTime;
	mWaterParameters.frame = mFrame;

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

		auto& p1 = RoomManager::singleton->mGoLayers[GOL_PERSP_FIRST];
		auto* p2 = isEye ? &p1.cameraEye : &p1.cameraTarget;
		*p2 += delta * mDeltaTime * 10.0f;
	}
#endif

	/*
	// Animation for eye camera
	if (mAnimateInGameCamera)
	{
		auto* p = &RoomManager::singleton->mGoLayers[GOL_PERSP_SECOND].cameraEye[2];
		*p = mCubicBezier->value(Math::min(mFrame * 0.25f, 1.0f))[1] * 44.0f;
	}
	*/
}

void Scenery::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (baseDrawable == mWaterDrawable.get())
	{
		WaterShader& shader = (WaterShader&) baseDrawable->getShader();
		shader
			.setTransformationMatrix(transformationMatrix)
			.setProjectionMatrix(camera.projectionMatrix())
			.setFrame(mWaterParameters.frame)
			.setSpeed(mWaterParameters.speed)
			.setSize(mWaterParameters.size)
			.setHorizonColorUniform(mWaterParameters.horizonColor)
			.bindDisplacementTexture(*mWaterParameters.displacementTexture)
			.bindWaterTexture(*mWaterParameters.waterTexture)
			.bindEffectsTexture(*mWaterParameters.effectsTexture)
			.draw(*baseDrawable->mMesh);
	}
	else
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
				return camera.cameraMatrix().transformPoint(mPosition + vector);
			};

			// Apply array mapping to all its elements
			std::transform(source.begin(), source.end(), std::back_inserter(destLightPos), mapFx);
		}

		// Draw through shader
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
}

void Scenery::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void Scenery::createWaterDrawable()
{
	// Create plane
	Resource<GL::Mesh> resMesh{ CommonUtility::singleton->manager.get<GL::Mesh>(RESOURCE_MESH_PLANE_WATER) };

	if (!resMesh)
	{
		// Create test mesh
		Trade::MeshData meshData = Primitives::planeSolid(Primitives::PlaneFlag::TextureCoordinates);

		GL::Buffer vertices;
		vertices.setData(MeshTools::interleave(meshData.positions3DAsArray(), meshData.textureCoordinates2DAsArray()));

		GL::Mesh mesh;
		mesh
			.setPrimitive(meshData.primitive())
			.setCount(meshData.vertexCount())
			.addVertexBuffer(std::move(vertices), 0, WaterShader::Position{}, WaterShader::TextureCoordinates{});

		// Add to resources
		CommonUtility::singleton->manager.set(resMesh.key(), std::move(mesh));
	}

	// Create shader
	Resource<GL::AbstractShaderProgram, WaterShader> resShader{ CommonUtility::singleton->manager.get<GL::AbstractShaderProgram, WaterShader>(RESOURCE_SHADER_WATER) };

	if (!resShader)
	{
		// Create shader
		std::unique_ptr<GL::AbstractShaderProgram> shader = std::make_unique<WaterShader>();

		// Add to resources
		Containers::Pointer<GL::AbstractShaderProgram> p = std::move(shader);
		CommonUtility::singleton->manager.set(resShader.key(), std::move(p));
	}

	// Create water drawable
	const auto& waterTexture = CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_WATER_TEXTURE);

	auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
	mWaterDrawable = std::make_shared<TexturedDrawable<WaterShader>>(*drawables, resShader, resMesh, waterTexture);
	mWaterDrawable->setParent(mManipulatorList[1]);
	mWaterDrawable->setDrawCallback(this);

	mWaterParameters = {
		CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_WATER_DISPLACEMENT),
		waterTexture,
		CommonUtility::singleton->loadTexture(RESOURCE_TEXTURE_WORLD_1_WEM),
		0.0f,
		2.0f,
		15.0f,
		Color3(1.0f)
	};
}

const Int Scenery::getModelIndex() const
{
	return mModelIndex;
}

const void Scenery::animateInGameCamera()
{
	mAnimateInGameCamera = true;
}