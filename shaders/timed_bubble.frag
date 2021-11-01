#ifdef GL_ES
  precision mediump float;
#endif

uniform sampler2D colorData;
uniform sampler2D maskData;
uniform float timedRotation;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

void main()
{
  vec2 tc = interpolatedTextureCoordinates - 0.5;
  tc *= 1.25;
  tc += 0.5;

	fragmentColor = texture(colorData, clamp(tc, vec2(0.0), vec2(1.0)));

	float a = texture(maskData, interpolatedTextureCoordinates).r;
  float b = max((timedRotation - 0.9) * 5.0, 0.0);
  float c = min((1.0 - timedRotation * 1.1) * 0.5, 1.0);
  // float d = smoothstep(max(a - 0.02, 0.0), min(a + 0.02, 1.0), b + c);
  float d = step(a, b + c);
  
  fragmentColor.rgb = mix(fragmentColor.rgb, vec3(0.0), d);
}