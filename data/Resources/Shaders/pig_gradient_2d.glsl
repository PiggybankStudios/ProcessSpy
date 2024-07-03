//======================== VERTEX_SHADER ========================

#version 130

uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

uniform vec2 Texture1Size;
uniform vec4 SourceRec1;

// layout(location = 0) 
in vec3 inPosition;
in vec4 inColor1;
in vec2 inTexCoord1;

out vec4 fColor1;
out vec2 fTexCoord1;
out vec2 fSampleCoord;
out vec3 fPosition;

void main()
{
	fPosition = inPosition;
	fColor1 = inColor1;
	fTexCoord1 = inTexCoord1;
	fSampleCoord = (SourceRec1.xy + (inTexCoord1 * SourceRec1.zw)) / Texture1Size;
	mat4 transformMatrix = ProjectionMatrix * (ViewMatrix * WorldMatrix);
	gl_Position = transformMatrix * vec4(inPosition, 1.0);
}

//======================== FRAGMENT_SHADER ========================

#version 130

uniform sampler2D Texture1;
uniform vec2 Texture1Size;

uniform vec4 Color1;
uniform vec4 Color2;

in vec4 fColor1;
in vec2 fTexCoord1;
in vec2 fSampleCoord;
in vec3 fPosition;

out vec4 Result;

vec4 srgb_to_linear(vec4 color)
{
	// note: some people use an approximation for the gamma of 2.0, for efficiency, but 2.2 is more correct
	return vec4(pow(color.r, 2.2), pow(color.g, 2.2), pow(color.b, 2.2), color.a);
}

vec4 linear_to_srgb(vec4 color)
{
	// note: if using gamma of 2.0, instead can use 0.5 as the value here
	return vec4(pow(color.r, 1.0/2.2), pow(color.g, 1.0/2.2), pow(color.b, 1.0/2.2), color.a);
}

void main()
{
	vec4 sampleColor = texture(Texture1, fSampleCoord);
	// Result = fColor1 * mix(Color1, Color2, sampleColor.r);
	Result = fColor1 * linear_to_srgb(mix(srgb_to_linear(Color1), srgb_to_linear(Color2), sampleColor.r));
}