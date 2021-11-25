#include "CommonUtility.h"

#include <Corrade/Corrade.h>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/Utility/Assert.h>
#include <Corrade/Utility/Resource.h>

#include <Magnum/Magnum.h>
#include <Magnum/ImageView.h>
#include <Magnum/Audio/AbstractImporter.h>
#include <Magnum/Trade/AbstractImporter.h>
#include <Magnum/Trade/ImageData.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Math.h>
#include <Magnum/Text/AbstractFont.h>

#include "../RoomManager.h"
#include "../AssetManager.h"
#include "../Graphics/GameDrawable.h"

#if defined(CORRADE_TARGET_ANDROID)
#include <android/native_activity.h>
#endif

using namespace Corrade;
using namespace Magnum;
using namespace Magnum::Math::Literals;

std::unique_ptr<CommonUtility> CommonUtility::singleton = nullptr;

const std::string CommonUtility::VECTOR_COMPONENTS[] = { "x", "y", "z", "w" };

CommonUtility::CommonUtility() : mConfig{ nullptr, "", 0.0f, 1.0f, "" }
{
}

void CommonUtility::clear()
{
	manager.clear();
}

Resource<Audio::Buffer> CommonUtility::loadAudioData(const std::string & filename)
{
	// Get required resource
	Resource<Audio::Buffer> resAudio{ CommonUtility::singleton->manager.get<Audio::Buffer>(filename) };

	if (!resAudio)
	{
		// Load importer plugin
		PluginManager::Manager<Audio::AbstractImporter> manager;
		Containers::Pointer<Audio::AbstractImporter> importer = manager.loadAndInstantiate("StbVorbisAudioImporter");
		if (!importer)
		{
            Fatal{} << "Could not instantiate audio importer";
		}

		if (!importer->openFile(mConfig.assetDir + "audios/" + filename + ".ogg"))
		{
            Fatal{} << "Could not load audio" << filename;
		}

		/*
			Get the data from importer and add them to the buffer. Be sure to
			keep a copy to avoid dangling reference.
		*/
		Containers::Array<char> bufferData = importer->data();
		Audio::Buffer buffer;
		buffer.setData(importer->format(), bufferData, importer->frequency());

		// Add to resources
		CommonUtility::singleton->manager.set(resAudio.key(), std::move(buffer));
	}

	return resAudio;
}

Resource<GL::Texture2D> CommonUtility::loadTexture(const std::string & filename)
{
	// Get required resource
	Resource<GL::Texture2D> resTexture{ CommonUtility::singleton->manager.get<GL::Texture2D>(filename) };

	if (!resTexture)
	{
		PluginManager::Manager<Trade::AbstractImporter> manager;
		Containers::Pointer<Trade::AbstractImporter> importer = manager.loadAndInstantiate("PngImporter");

		if (!importer || !importer->openFile(CommonUtility::singleton->mConfig.assetDir + "textures/" + filename + ".png"))
		{
            Fatal{} << "Could not load texture" << filename;
		}

		// Set texture data and parameters
		Containers::Optional<Trade::ImageData2D> image = importer->image2D(0);
		CORRADE_INTERNAL_ASSERT(image);

		GL::Texture2D texture;
		texture
			.setWrapping(GL::SamplerWrapping::ClampToEdge)
			.setMagnificationFilter(GL::SamplerFilter::Linear)
			.setMinificationFilter(GL::SamplerFilter::Linear)
			.setStorage(1, GL::textureFormat(image->format()), image->size())
			.setSubImage(0, {}, *image);

		// Add to resources
		CommonUtility::singleton->manager.set(resTexture.key(), std::move(texture));
	}

	// Return loaded resources
	return resTexture;
}

Resource<FontHolder> CommonUtility::loadFont(const std::string & filename)
{
	// Get required resource
	Resource<FontHolder> resFont{ CommonUtility::singleton->manager.get<FontHolder>(filename) };;

	if (!resFont)
	{
		std::unique_ptr<FontHolder> fh = std::make_unique<FontHolder>();
		fh->font = fh->manager.loadAndInstantiate("TrueTypeFont");
		if (!fh->font || !fh->font->openFile(mConfig.assetDir + "fonts/" + filename + ".ttf", 100.0f))
		{
            Fatal{} << "Cannot open font file";
		}

		// Fill glyph cache
		fh->cache = std::make_unique<Text::DistanceFieldGlyphCache>(Vector2i{2048}, Vector2i{512}, 22);
		fh->font->fillGlyphCache(*fh->cache, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:-+,.!?()<> ");

		// Add to resources
		Containers::Pointer<FontHolder> p = std::move(fh);
		CommonUtility::singleton->manager.set(resFont.key(), std::move(p));
	}

	return resFont;
}

bool CommonUtility::stringEndsWith(const std::string& data, const std::string& suffix)
{
	return data.find(suffix, data.size() - suffix.size()) != std::string::npos;
}

bool CommonUtility::isBubbleColorValid(const Color3 & color)
{
	return color.r() > 0.001f || color.g() > 0.001f || color.b() > 0.04f;
}

Resource<GL::Texture2D> CommonUtility::getTextureForBubble(const Color3 & color)
{
	// Check for color validity
	const auto& it = RoomManager::singleton->sBubbleColors.find(color.toSrgbInt());
	if (it == RoomManager::singleton->sBubbleColors.end())
	{
		Fatal{} << "Color " + std::to_string(color.toSrgbInt()) + " for bubble was invalid";
	}

	// Load texture
	return CommonUtility::singleton->loadTexture(it->second.textureKey);
}

void CommonUtility::createGameSphere(GameObject* gameObject, Object3D & manipulator, const Color3 & color)
{
	// Create game bubble
	AssetManager().loadAssets(*gameObject, manipulator, RESOURCE_SCENE_BUBBLE, gameObject);

	// Load texture
	gameObject->mDrawables.back()->mTexture = getTextureForBubble(color);
}

std::shared_ptr<BaseDrawable> CommonUtility::createSpriteDrawable(const Int goLayerIndex, Object3D & parent, Resource<GL::Texture2D> & texture, IDrawCallback* drawCallback)
{
	Resource<GL::Mesh> resMesh{ CommonUtility::singleton->manager.get<GL::Mesh>(RESOURCE_MESH_PLANE_SPRITE) };

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
			.addVertexBuffer(std::move(vertices), 0, SpriteShader::Position{}, SpriteShader::TextureCoordinates{});

		// Add to resources
		CommonUtility::singleton->manager.set(resMesh.key(), std::move(mesh));
	}

	// Create shader
	Resource<GL::AbstractShaderProgram, SpriteShader> resShader{ CommonUtility::singleton->manager.get<GL::AbstractShaderProgram, SpriteShader>(RESOURCE_SHADER_SPRITE) };

	if (!resShader)
	{
		// Create shader
		std::unique_ptr<GL::AbstractShaderProgram> shader = std::make_unique<SpriteShader>();

		// Add to resources
		Containers::Pointer<GL::AbstractShaderProgram> p = std::move(shader);
		CommonUtility::singleton->manager.set(resShader.key(), std::move(p));
	}

	// Create textured drawable
	auto& drawables = RoomManager::singleton->mGoLayers[goLayerIndex].drawables;
	std::shared_ptr<GameDrawable<SpriteShader>> td = std::make_shared<GameDrawable<SpriteShader>>(*drawables, resShader, resMesh, texture);
	td->setParent(&parent);
	td->setDrawCallback(drawCallback);
	return td;
}

Resource<GL::AbstractShaderProgram, Shaders::Flat3D> CommonUtility::getFlat3DShader()
{
	return getSpecializedShader<Shaders::Flat3D>(RESOURCE_SHADER_FLAT3D, [] {
		const auto& flags = Shaders::Flat3D::Flag::Textured | Shaders::Flat3D::Flag::AlphaMask;
		return (std::unique_ptr<GL::AbstractShaderProgram>) std::make_unique<Shaders::Flat3D>(flags);
	});
}

Resource<GL::AbstractShaderProgram, TimedBubbleShader> CommonUtility::getTimedBubbleShader()
{
	return getSpecializedShader<TimedBubbleShader>(RESOURCE_SHADER_TIMED_BUBBLE, [] {
		return (std::unique_ptr<GL::AbstractShaderProgram>) std::make_unique<TimedBubbleShader>();
	});
}

Resource<GL::AbstractShaderProgram, PlasmaShader> CommonUtility::getPlasmaShader()
{
	return getSpecializedShader<PlasmaShader>(RESOURCE_SHADER_PLASMA, [] {
		return (std::unique_ptr<GL::AbstractShaderProgram>) std::make_unique<PlasmaShader>();
	});
}

Resource<GL::AbstractShaderProgram, WaterShader> CommonUtility::getWaterShader()
{
	return getSpecializedShader<WaterShader>(RESOURCE_SHADER_WATER, [] {
		return (std::unique_ptr<GL::AbstractShaderProgram>) std::make_unique<WaterShader>();
	});
}

Resource<GL::AbstractShaderProgram, StarRoadShader> CommonUtility::getStarRoadShader()
{
	return getSpecializedShader<StarRoadShader>(RESOURCE_SHADER_STARROAD, [] {
		return (std::unique_ptr<GL::AbstractShaderProgram>) std::make_unique<StarRoadShader>();
	});
}

Resource<GL::AbstractShaderProgram, SunShader> CommonUtility::getSunShader()
{
	return getSpecializedShader<SunShader>(RESOURCE_SHADER_SUN, [] {
		return (std::unique_ptr<GL::AbstractShaderProgram>) std::make_unique<SunShader>();
	});
}

Resource<GL::AbstractShaderProgram, ShootPathShader> CommonUtility::getShootPathShader()
{
	return getSpecializedShader<ShootPathShader>(RESOURCE_SHADER_SHOOT_PATH, [] {
		return (std::unique_ptr<GL::AbstractShaderProgram>) std::make_unique<ShootPathShader>();
	});
}

std::unique_ptr<std::string> CommonUtility::getValueFromIntent(const std::string & key)
{
#if defined(CORRADE_TARGET_ANDROID)
    
	auto* na = (ANativeActivity*)mConfig.nativeActivity;

	JNIEnv *env;
	na->vm->AttachCurrentThread(&env, nullptr);

	jobject me = na->clazz;

	jclass acl = env->GetObjectClass(me); // Class pointer of NativeActivity
	jmethodID giid = env->GetMethodID(acl, "getIntent", "()Landroid/content/Intent;");
	jobject intent = env->CallObjectMethod(me, giid); // Got our intent

	jclass icl = env->GetObjectClass(intent); // Class pointer of Intent
	jmethodID gseid = env->GetMethodID(icl, "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");

	std::unique_ptr<std::string> valueToReturn = nullptr;
	{
		const auto jsParam = (jstring) env->CallObjectMethod(intent, gseid, env->NewStringUTF(key.c_str()));
		if (jsParam != nullptr)
		{
			const char *Param = env->GetStringUTFChars(jsParam, nullptr);
			valueToReturn = std::make_unique<std::string>(std::string(Param));
			env->ReleaseStringUTFChars(jsParam, Param);
		}
	}

	return valueToReturn;
    
#elif defined(CORRADE_TARGET_IOS) || defined(CORRADE_TARGET_IOS_SIMULATOR)
    
    if (key == INTENT_GP_EXPIRE)
    {
        const Long value = ios_GetGamePowerupExpire();
        if (value != 0) {
            std::unique_ptr<std::string> str = std::make_unique<std::string>(std::to_string(value));
            return str;
        }
    }
    else if (key == INTENT_GP_AMOUNT)
    {
        const Int value = ios_GetGamePowerupAmount();
        if (value != 0) {
            std::unique_ptr<std::string> str = std::make_unique<std::string>(std::to_string(value));
            return str;
        }
    }
    else if (key == INTENT_PLAY_AD_THRESHOLD)
    {
        const Int value = ios_GetPlayAdThreshold();
        std::unique_ptr<std::string> str = std::make_unique<std::string>(std::to_string(value));
        return str;
    }
    else if (key.rfind("game_", 0) == 0)
    {
        const Int value = ios_GetLaunchOptionValue(key.c_str());
        if (value != 0) {
            std::unique_ptr<std::string> str = std::make_unique<std::string>(std::to_string(value));
            return str;
        }
    }
    return nullptr;
    
#else
    
    return nullptr;
    
#endif
}

std::string CommonUtility::getTextureNameForPowerup(const UnsignedInt index)
{
	switch (index)
	{
	case 0:
		return RESOURCE_TEXTURE_GUI_PU_BOMB;

	case 1:
		return RESOURCE_TEXTURE_GUI_PU_PLASMA;

	case 2:
		return RESOURCE_TEXTURE_GUI_PU_TIME;

	case 3:
		return RESOURCE_TEXTURE_GUI_PU_ELECTRIC;
	}
	return "";
}


Float CommonUtility::getScaledVerticalPadding()
{
	return mConfig.canvasVerticalPadding / mFramebufferSize.y();
}
