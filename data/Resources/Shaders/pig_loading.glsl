//======================== VERTEX_SHADER ========================

#version 130

uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

uniform vec2 TextureSize;
uniform vec4 SourceRectangle;

in vec3 inPosition;
in vec4 inColor;
in vec2 inTexCoord;

out vec4 fColor;
out vec2 fTexCoord;
out vec2 fSampleCoord;

void main()
{
	fColor = inColor;
	fTexCoord = inTexCoord;
	fSampleCoord = (SourceRectangle.xy + (inTexCoord * SourceRectangle.zw)) / TextureSize;
	mat4 transformMatrix = ProjectionMatrix * (ViewMatrix * WorldMatrix);
	gl_Position = transformMatrix * vec4(inPosition, 1.0);
}

//======================== FRAGMENT_SHADER ========================

#version 130

uniform vec4 PrimaryColor;
uniform vec4 SecondaryColor;

uniform sampler2D Texture;
uniform vec2 TextureSize;
uniform bool SingleChannelTexture;

uniform float Value0; //vignetteInnerRadius
uniform float Value1; //vignetteOuterRadius

in vec4 fColor;
in vec2 fTexCoord;
in vec2 fSampleCoord;

out vec4 Result;

vec3 srgb_to_linear(vec3 color) {
	// note: some people use an approximation for the gamma of 2.0, for efficiency, but 2.2 is more correct
	return vec3(pow(color.r, 2.2), pow(color.g, 2.2), pow(color.b, 2.2));
}

vec3 linear_to_srgb(vec3 color) {
	// note: if using gamma of 2.0, instead can use 0.5 as the value here
	return vec3(pow(color.r, 1.0/2.2), pow(color.g, 1.0/2.2), pow(color.b, 1.0/2.2));
}

void main()
{
	vec4 sampleColor = texture(Texture, fSampleCoord);
	if (SingleChannelTexture)
	{
		sampleColor = vec4(1, 1, 1, sampleColor.r);
	}
	Result = PrimaryColor * fColor * sampleColor;
	// Result.a = 1;
	// Result = vec4(fTexCoord.x, fTexCoord.y, 0.5, 1);
	// Result = vec4(fSampleCoord.x, fSampleCoord.y, 0.5, 1);
	
	if (Value1 > 0)
	{
		float distFromCenter = length(fTexCoord - vec2(0.5, 0.5))*2;
		if (distFromCenter > Value1)
		{
			Result.rgb = vec3(0);
		}
		else if (distFromCenter > Value0)
		{
			Result.rgb = srgb_to_linear(Result.rgb);
			Result = mix(Result, vec4(0, 0, 0, 1), (distFromCenter - Value0) / (Value1 - Value0));
			Result.rgb = linear_to_srgb(Result.rgb);
		}
	}
	
	if (Result.a < 0.02) { discard; }
}