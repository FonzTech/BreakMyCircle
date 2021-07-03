#define PI 3.1415926538

uniform sampler2D displacementData;
uniform sampler2D textureData;

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
	float angle = wm.r + frame;
	
	vec2 dc;
	{
		dc.x = fract(tc.x + cos(angle) * speed * 0.07);
		dc.y = fract(tc.y + sin(angle) * speed * 0.12);
	}
	
	// Color fragment
	fragmentColor.rgb = texture(textureData, dc).rgb;
	fragmentColor.a = texture(displacementData, dc).r;
}
