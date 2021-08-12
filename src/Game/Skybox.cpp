#include "Skybox.h"

#include <Magnum/ImageView.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/CubeMapTexture.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/MeshTools/FlipNormals.h>
#include <Magnum/MeshTools/Reference.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/Trade/MeshData.h>

#include "../Common/CommonUtility.h"
#include "../Shaders/CubeMapShader.h"
#include "../RoomManager.h"

std::shared_ptr<GameObject> Skybox::getInstance(const nlohmann::json & params)
{
	// Get parent index
	Int parent;
	params.at("parent").get_to(parent);

	// Instantiate bubble
	std::string name;
	params.at("name").get_to(name);

	// Read parameters
	const Vector3& position = CommonUtility::singleton->getVectorFromJson<3, Float>(params);

	std::shared_ptr<Skybox> p = std::make_shared<Skybox>(parent, name, position);
	return p;
}

Skybox::Skybox(const Int parentIndex, const std::string & name, const Vector3 & position) : GameObject(parentIndex)
{
	mPosition = position;
	createDrawable(name);
}

const Int Skybox::getType() const
{
	return GOT_SKYBOX;
}

void Skybox::update()
{
}

void Skybox::draw(BaseDrawable* baseDrawable, const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	((CubeMapShader&) baseDrawable->getShader())
		.setTransformationProjectionMatrix(camera.projectionMatrix() * transformationMatrix)
		.setTexture(*resTexture)
		.draw(*baseDrawable->mMesh);
}

void Skybox::collidedWith(const std::unique_ptr<std::unordered_set<GameObject*>> & gameObjects)
{
}

void Skybox::createDrawable(const std::string & name)
{
	// Get mesh
	Resource<GL::Mesh> resMesh = CommonUtility::singleton->manager.get<GL::Mesh>(RESOURCE_MESH_CUBE);
	if (!resMesh)
	{
		Trade::MeshData meshData = MeshTools::owned(Primitives::cubeSolid());
		MeshTools::flipFaceWindingInPlace(meshData.mutableIndices());
		GL::Mesh meshCube = MeshTools::compile(meshData);
		CommonUtility::singleton->manager.set(resMesh.key(), std::move(meshCube));
	}

	// Generate cubemap
	resTexture = CommonUtility::singleton->manager.get<GL::CubeMapTexture>(RESOURCE_TEXTURE_CUBEMAP_SKYBOX_1);
	if (!resTexture)
	{
		GL::CubeMapTexture cubeMap;

		cubeMap
			.setWrapping(GL::SamplerWrapping::ClampToEdge)
			.setMagnificationFilter(GL::SamplerFilter::Linear)
			.setMinificationFilter(GL::SamplerFilter::Linear, GL::SamplerMipmap::Linear);

		// Configure texture storage using size of first image
		PluginManager::Manager<Trade::AbstractImporter> manager;
		Containers::Pointer<Trade::AbstractImporter> importer = manager.loadAndInstantiate("PngImporter");

		if (!importer)
		{
			std::exit(-4);
		}

		importer->openFile("textures/" + name + "_px.png");
		Containers::Optional<Trade::ImageData2D> image = importer->image2D(0);
		CORRADE_INTERNAL_ASSERT(image);
		Vector2i size = image->size();
		cubeMap
			.setStorage(Math::log2(size.min()) + 1, GL::TextureFormat::RGB8, size)
			.setSubImage(GL::CubeMapCoordinate::PositiveX, 0, {}, *image);

		importer->openFile("textures/" + name + "_nx.png");
		CORRADE_INTERNAL_ASSERT_OUTPUT(image = importer->image2D(0));
		cubeMap.setSubImage(GL::CubeMapCoordinate::NegativeX, 0, {}, *image);

		importer->openFile("textures/" + name + "_ny.png");
		CORRADE_INTERNAL_ASSERT_OUTPUT(image = importer->image2D(0));
		cubeMap.setSubImage(GL::CubeMapCoordinate::PositiveY, 0, {}, *image);

		importer->openFile("textures/" + name + "_py.png");
		CORRADE_INTERNAL_ASSERT_OUTPUT(image = importer->image2D(0));
		cubeMap.setSubImage(GL::CubeMapCoordinate::NegativeY, 0, {}, *image);

		importer->openFile("textures/" + name + "_pz.png");
		CORRADE_INTERNAL_ASSERT_OUTPUT(image = importer->image2D(0));
		cubeMap.setSubImage(GL::CubeMapCoordinate::PositiveZ, 0, {}, *image);

		importer->openFile("textures/" + name + "_nz.png");
		CORRADE_INTERNAL_ASSERT_OUTPUT(image = importer->image2D(0));
		cubeMap.setSubImage(GL::CubeMapCoordinate::NegativeZ, 0, {}, *image);

		cubeMap.generateMipmap();

		CommonUtility::singleton->manager.set(resTexture.key(), std::move(cubeMap));
	}

	// Create shader
	Resource<GL::AbstractShaderProgram, CubeMapShader> resShader{ CommonUtility::singleton->manager.get<GL::AbstractShaderProgram, CubeMapShader>(RESOURCE_SHADER_CUBEMAP) };

	if (!resShader)
	{
		// Create shader
		std::unique_ptr<GL::AbstractShaderProgram> shader = std::make_unique<CubeMapShader>();

		// Add to resources
		Containers::Pointer<GL::AbstractShaderProgram> p = std::move(shader);
		CommonUtility::singleton->manager.set(resShader.key(), std::move(p));
	}

	// Load dummy texture
	Resource<GL::Texture2D> resTexture{ CommonUtility::singleton->manager.get<GL::Texture2D>("_non_existing_texture") };

	// Create textured drawable
	auto& drawables = RoomManager::singleton->mGoLayers[mParentIndex].drawables;
	std::shared_ptr<GameDrawable<CubeMapShader>> td = std::make_shared<GameDrawable<CubeMapShader>>(*drawables, resShader, resMesh, resTexture);
	td->setParent(mManipulator.get());
	td->setDrawCallback(this);
	mDrawables.emplace_back(td);

	// Apply transformation
	mManipulator->setTransformation(Matrix4::scaling(Vector3(100.0f)));
	mManipulator->rotateZ(180.0_degf);
	mManipulator->translate(mPosition);
}