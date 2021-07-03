#define PI 3.1415926538

uniform sampler2D displacementData;
uniform sampler2D textureData;
uniform sampler2D effectsData;

uniform vec3 waterColor;
uniform float frame;
uniform float speed;
uniform float size;

in vec2 interpolatedTextureCoordinates;

out vec4 fragmentColor;

void main()
{
	// Get texture coordinates repeated N times
	vec2 tc = interpolatedTextureCoordinates * size;

	// Make tiled coordinates
	tc.x = fract(tc.x);
	tc.y = fract(tc.y);

	// Apply distortion
	vec4 wm = texture(displacementData, tc);
	float angle = fract(wm.r + frame * 0.5) * PI * 2.0;
	
	vec2 dc;
	{
		dc.x = fract(tc.x + cos(angle) * speed * 0.01);
		dc.y = fract(tc.y + sin(angle) * speed * 0.01);
	}
	
	// Water color
	fragmentColor.rgb = texture(textureData, dc).rgb;
	
	// Effects data
	vec4 fx = texture(effectsData, interpolatedTextureCoordinates);
	
	// Alpha component
	{
		float a = fragmentColor.r * fragmentColor.g * fragmentColor.b;
		float b = clamp(a * 1.5, 0.0, 1.0);
		fragmentColor.a = mix(b, 1.0, fx.r);
	}
	
	// Seam effect
	fragmentColor.a = mix(fragmentColor.a, 0.0, fx.g);
}
