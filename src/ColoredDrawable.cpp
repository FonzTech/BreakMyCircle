#include "ColoredDrawable.h"

ColoredDrawable::ColoredDrawable(SceneGraph::DrawableGroup3D& group, Shaders::Phong& shader, GL::Mesh& mesh, const Color4& color) : SceneGraph::Drawable3D{ *this, &group }, mShader(shader), mMesh(mesh)
{
	mColor = color;
}

void ColoredDrawable::draw(const Matrix4& transformationMatrix, SceneGraph::Camera3D& camera)
{
	mShader.setDiffuseColor(mColor);
	mShader.setLightPosition(camera.cameraMatrix().transformPoint({ -3.0f, 10.0f, 10.0f }));
	mShader.setTransformationMatrix(transformationMatrix);
	mShader.setNormalMatrix(transformationMatrix.normalMatrix());
	mShader.setProjectionMatrix(camera.projectionMatrix());
	mShader.draw(mMesh);
}