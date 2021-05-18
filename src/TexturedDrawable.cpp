#include "TexturedDrawable.h"

TexturedDrawable::TexturedDrawable(SceneGraph::DrawableGroup3D& group, Shaders::Phong& shader, GL::Mesh& mesh, GL::Texture2D& texture) : SceneGraph::Drawable3D{ *this, &group }
{
	mShader = std::move(shader);
	mMesh = std::move(mesh);
	mTexture = std::move(texture);
	mDrawCallback = nullptr;
}

void TexturedDrawable::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	if (mDrawCallback != nullptr)
	{
		mDrawCallback->draw(transformationMatrix, camera);
		return;
	}
	mShader
		.setLightPosition(camera.cameraMatrix().transformPoint({ -3.0f, 10.0f, 10.0f }))
		.setTransformationMatrix(transformationMatrix)
		.setNormalMatrix(transformationMatrix.normalMatrix())
		.setProjectionMatrix(camera.projectionMatrix())
		.bindDiffuseTexture(mTexture)
		.draw(mMesh);
}

void TexturedDrawable::setDrawCallback(IDrawCallback* drawCallback)
{
	mDrawCallback = drawCallback;
}