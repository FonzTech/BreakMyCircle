#include "ColoredDrawable.h"

ColoredDrawable::ColoredDrawable(SceneGraph::DrawableGroup3D& group, Shaders::Phong& shader, GL::Mesh& mesh, const Color4& color) : SceneGraph::Drawable3D{ *this, &group }, mShader(shader), mMesh(mesh)
{
	mColor = color;
	mDrawCallback = nullptr;
	printf("5555 %d\n", mesh.count());
	printf("6666 %d\n", mMesh.count());
	printf("aaaa %d\n", &mesh);
}

void ColoredDrawable::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	printf("bbbb %p\n", &mMesh);
	printf("2222 %d\n", mMesh.count());
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

void ColoredDrawable::setDrawCallback(IDrawCallback* drawCallback)
{
	mDrawCallback = drawCallback;
}