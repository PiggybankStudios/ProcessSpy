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
in vec3 inNormal1;

out vec4 fColor1;
out vec2 fTexCoord1;
out vec2 fSampleCoord;
out vec3 fPosition;
out vec3 fNormal;

void main()
{
	fPosition = inPosition;
	fNormal = inNormal1;
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

in vec4 fColor1;
in vec2 fTexCoord1;
in vec2 fSampleCoord;
in vec3 fPosition;
in vec3 fNormal;

out vec4 Result;

void main()
{
	vec4 sampleColor = texture(Texture1, fSampleCoord);
	Result = Color1 * fColor1 * sampleColor;
	if (Result.a < 0.02) { discard; }
}