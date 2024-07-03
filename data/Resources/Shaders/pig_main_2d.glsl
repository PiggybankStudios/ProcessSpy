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

uniform float CircleRadius;
uniform float CircleInnerRadius;
uniform float Saturation;
uniform float Brightness;

uniform float Value0; //vignetteInnerRadius
uniform float Value1; //vignetteOuterRadius

in vec4 fColor1;
in vec2 fTexCoord1;
in vec2 fSampleCoord;
in vec3 fPosition;

out vec4 Result;

vec3 czm_saturation(vec3 rgb, float adjustment)
{
	// Algorithm from Chapter 16 of OpenGL Shading Language
	const vec3 W = vec3(0.2125, 0.7154, 0.0721);
	vec3 intensity = vec3(dot(rgb, W));
	return mix(intensity, rgb, adjustment);
}

vec3 srgb_to_linear(vec3 color)
{
	// note: some people use an approximation for the gamma of 2.0, for efficiency, but 2.2 is more correct
	return vec3(pow(color.r, 2.2), pow(color.g, 2.2), pow(color.b, 2.2));
}

vec3 linear_to_srgb(vec3 color)
{
	// note: if using gamma of 2.0, instead can use 0.5 as the value here
	return vec3(pow(color.r, 1.0/2.2), pow(color.g, 1.0/2.2), pow(color.b, 1.0/2.2));
}

void main()
{
	vec4 sampleColor = texture(Texture1, fSampleCoord);
	
	Result = Color1 * fColor1 * sampleColor;
	// Result = vec4(fPosition.x,fPosition.y,0,1);
	// Result = vec4(fTexCoord1.x,fTexCoord1.y,0,1);
	// Result = vec4(fSampleCoord.x,fSampleCoord.y,0,1);
	// Result = Color1;
	
	if (CircleRadius != 0)
	{
		float distFromCenter = length(fTexCoord1 - vec2(0.5, 0.5)) * 2;
		float smoothDelta = fwidth(distFromCenter);
		Result.a *= smoothstep(CircleRadius, CircleRadius-smoothDelta, distFromCenter);
		if (CircleInnerRadius != 0)
		{
			Result.a *= smoothstep(CircleInnerRadius, CircleInnerRadius+smoothDelta, distFromCenter);
		}
	}
	
	if (Value1 > 0)
	{
		float distFromCenter = length(fTexCoord1 - vec2(0.5, 0.5))*2;
		if (distFromCenter > Value1)
		{
			Result.rgb = vec3(0);
		}
		else if (distFromCenter > Value0)
		{
			Result.rgb = srgb_to_linear(Result.rgb);
			Result.rgb = mix(Result.rgb, vec3(0), (distFromCenter - Value0) / (Value1 - Value0));
			Result.rgb = linear_to_srgb(Result.rgb);
		}
	}
	
	if (Result.a < 0.02) { discard; }
	
	// Result.rg = fTexCoord1.xy; //TODO: Delete me!
	// Result.rg = fSampleCoord.xy; //TODO: Delete me!
	// Result.b = 1;
	// Result.a = 1;
	// Result.rgb = czm_saturation(Result.rgb, Saturation); //TODO: Re-enable me!
	// Result.rgb *= Brightness; //TODO: Re-enable me!
	// Result.rgb *= Result.a;
}