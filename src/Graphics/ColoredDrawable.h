#pragma once

#include "../Common/CommonTypes.h"
#include "../Common/CommonUtility.h"
#include "BaseDrawable.h"

template <class ShaderType>
class ColoredDrawable : public BaseDrawable
{
public:
	template <class ShaderType>
	explicit ColoredDrawable(SceneGraph::DrawableGroup3D& group, const Resource<GL::AbstractShaderProgram, ShaderType>& shader, const Resource<GL::Mesh>& mesh, const Color4& color) : BaseDrawable{ group }
	{
		mShader = shader;
		mMesh = mesh;
		mColor = color;
		mDrawCallback = nullptr;
	}

	template <class ShaderType>
	explicit ColoredDrawable(const ColoredDrawable * cd) : BaseDrawable{ *cd }
	{
		mShader = CommonUtility::singleton->manager.get<GL::AbstractShaderProgram, ShaderType>(cd->mShader.key());
		mMesh = CommonUtility::singleton->manager.get<GL::Mesh>(cd->mMesh.key());
		mColor = cd->mColor;
		mDrawCallback = nullptr;
	}

	ShaderType& getShader()
	{
		return *mShader;
	}

	void draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera) override
	{
		if (mDrawCallback != nullptr)
		{
			mDrawCallback->draw(this, transformationMatrix, camera);
			return;
		}

		((Shaders::Phong&) *mShader)
			.setAmbientColor(0x000000ff_rgbaf)
			.setDiffuseColor(mColor)
			.setLightPosition(camera.cameraMatrix().transformPoint({ -3.0f, 5.0f, 10.0f }))
			.setTransformationMatrix(transformationMatrix)
			.setNormalMatrix(transformationMatrix.normalMatrix())
			.setProjectionMatrix(camera.projectionMatrix())
			.draw(*mMesh);
	}

	Resource<GL::AbstractShaderProgram, ShaderType> mShader;
};