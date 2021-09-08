uniform sampler2D colorPerspFirst;
uniform sampler2D depthStencilPerspFirst;
uniform sampler2D colorPerspSecond;
uniform sampler2D colorOrthoFirst;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

float linearize_Z(float depth, float zNear, float zFar)
{
  return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main()
{
	// P First - Color
  fragmentColor.rgb = texture(colorPerspFirst, interpolatedTextureCoordinates).rgb;
  fragmentColor.a = 1.0;
	
	// P First - Depth Stencil
  if (false)
	{
		float c = texture(depthStencilPerspFirst, interpolatedTextureCoordinates).r;
    c = linearize_Z(c, 0.01, 25.0);
    c = clamp(pow(c, 8.0), 0.0, 1.0);
    c = clamp(c * 4.0, 0.0, 1.0);
		fragmentColor.rgb = mix(fragmentColor.rgb, vec3(1.0), c * 0.001);
	}
	
	// P Second - Color
	{
		vec4 c = texture(colorPerspSecond, interpolatedTextureCoordinates);
		fragmentColor.rgb = mix(fragmentColor.rgb, c.rgb, c.a);
	}
	
	// O First - Color
	{
		vec4 c = texture(colorOrthoFirst, interpolatedTextureCoordinates);
		fragmentColor.rgb = mix(fragmentColor.rgb, c.rgb, c.a);
	}
}