#include "Scenery.h"

#include "AssetManager.h"
#include "RoomManager.h"
#include "CommonUtility.h"

using namespace Magnum;
using namespace Magnum::Math::Literals;

Scenery::Scenery()
{
	// Create manipulator list
	mManipulatorList.push_back(new Object3D{ mManipulator.get() });
	mManipulatorList.push_back(new Object3D{ mManipulator.get() });

	// Load assets
	{
		AssetManager am;
		am.loadAssets(*this, *mManipulatorList[0], "scenes/world_1.glb", this);
	}

	{
		AssetManager am(RESOURCE_SHADER_COLORED_PHONG, RESOURCE_SHADER_TEXTURED_PHONG_DIFFUSE_2, 2);
		am.loadAssets(*this, *mManipulatorList[1], "scenes/scenery_1.glb", this);
	}
}

const Int Scenery::getType() const
{
	return GOT_SCENERY;
}

void Scenery::update()
{
	// Apply transformations
	{
		const auto& m = Matrix4::translation(position + Vector3(8.0f, 1.0f, 0.0f)) * Matrix4::rotationX(Deg(90.0f));
		mManipulatorList[0]->setTransformation(m);
	}

	{
		const auto& m = Matrix4::translation(position + Vector3(8.0f, -35.0f, -1.1f));
		mManipulatorList[1]->setTransformation(m);
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

		if (baseDrawable->parent()->parent() == mManipulatorList[1])
		{
			source.emplace_back(8.0f, 20.0f, 10.0f);
			source.emplace_back(8.0f, -40.0f, 10.0f);

			destLightColors.emplace_back(0xffffff00_rgbaf);
			destLightColors.emplace_back(0xffffff00_rgbaf);
		}
		else
		{
			source.emplace_back(8.0f, 20.0f, 20.0f);

			destLightColors.emplace_back(0xffffff00_rgbaf);
		}

		// Create map function
		auto mapFx = [&](const decltype(source)::value_type & vector)
		{
			return camera.cameraMatrix().transformPoint(position + vector);
		};

		// Apply array mapping to all its elements
		std::transform(source.begin(), source.end(), std::back_inserter(destLightPos), mapFx);
	}

	// Shader through shader
	((Shaders::Phong&) baseDrawable->getShader())
		.setLightPositions(destLightPos)
		.setLightColors(destLightColors)
		.setSpecularColor(0xffffff00_rgbaf)
		.setAmbientColor(0x444444ff_rgbaf)
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.bindTextures(baseDrawable->mTexture, baseDrawable->mTexture, nullptr, nullptr)
		.draw(*baseDrawable->mMesh);
}

void Scenery::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}