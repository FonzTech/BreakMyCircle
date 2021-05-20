#include "ColoredDrawable.h"

ColoredDrawable::ColoredDrawable(SceneGraph::DrawableGroup3D& group, Shaders::Phong& shader, GL::Mesh& mesh, const Color4& color) : BaseDrawable{ group }
{
	mShader = std::move(shader);
	mMesh = std::move(mesh);
	mColor = color;
	mDrawCallback = nullptr;
}

void ColoredDrawable::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (mDrawCallback != nullptr)
	{
		mDrawCallback->draw(transformationMatrix, camera);
		return;
	}
	mShader
		.setDiffuseColor(mColor)
		.setLightPosition(camera.cameraMatrix().transformPoint({ -3.0f, 10.0f, 10.0f }))
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.draw(mMesh);
}