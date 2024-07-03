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
uniform vec4 Color2; //outline color
uniform float CircleRadius;
uniform vec2 ShiftVec; //used as rectangle size (aspect ratio and resolution info so we can interpret CircleRadius correctly)
uniform float Value0; //outline thickness

in vec4 fColor1;
in vec2 fTexCoord1;
in vec2 fSampleCoord;
in vec3 fPosition;

out vec4 Result;

float RectangleSignedDistance(vec2 uv, vec2 tl, vec2 br)
{
    vec2 d = max(tl-uv, uv-br);
    return length(max(vec2(0.0), d)) + min(0.0, max(d.x, d.y));
}

void main()
{
	vec4 sampleColor = texture(Texture1, fSampleCoord);
	Result = fColor1 * sampleColor;
	vec2 worldCoord = vec2(fTexCoord1.x * ShiftVec.x, fTexCoord1.y * ShiftVec.y);
	
	float distance = RectangleSignedDistance(worldCoord, vec2(CircleRadius, CircleRadius), ShiftVec - vec2(CircleRadius, CircleRadius));
	float smoothDelta = fwidth(distance);
	float edgeDistance = 1 + CircleRadius;
	Result.a = smoothstep(edgeDistance, edgeDistance - smoothDelta, distance);
	
	//If we have a border
	if (Value0 > 0)
	{
		Result *= mix(Color1, Color2, smoothstep(edgeDistance - Value0 - smoothDelta, edgeDistance - Value0, distance));
	}
	else
	{
		Result *= Color1;
	}
	
	// if (distance >= edgeDistance - Value0)
	// {
	// 	Result *= Color2;
	// }
	// else
	// {
	// 	Result *= Color1;
	// }
	
	// vec4 fillColor = Color1;
	// if (worldCoord.x < Value0) { fillColor = Color2; }
	// if (worldCoord.y < Value0) { fillColor = Color2; }
	// if (worldCoord.x > ShiftVec.x - Value0) { fillColor = Color2; }
	// if (worldCoord.y > ShiftVec.y - Value0) { fillColor = Color2; }
	
	// if (worldCoord.x < CircleRadius && worldCoord.y < CircleRadius)
	// {
	// 	float distFromCenter = length(worldCoord - vec2(CircleRadius, CircleRadius));
	// 	float smoothDelta = fwidth(distFromCenter);
	// 	Result.a *= smoothstep(CircleRadius, CircleRadius-smoothDelta, distFromCenter);
	// 	if (distFromCenter > CircleRadius - Value0 - smoothDelta/2) { fillColor = Color2; }
	// }
	// if (worldCoord.x > ShiftVec.x - CircleRadius && worldCoord.y < CircleRadius)
	// {
	// 	float distFromCenter = length(worldCoord - vec2(ShiftVec.x - CircleRadius, CircleRadius));
	// 	float smoothDelta = fwidth(distFromCenter);
	// 	Result.a *= smoothstep(CircleRadius, CircleRadius-smoothDelta, distFromCenter);
	// 	if (distFromCenter > CircleRadius - Value0 - smoothDelta/2) { fillColor = Color2; }
	// }
	// if (worldCoord.x > ShiftVec.x - CircleRadius && worldCoord.y > ShiftVec.y - CircleRadius)
	// {
	// 	float distFromCenter = length(worldCoord - vec2(ShiftVec.x - CircleRadius, ShiftVec.y - CircleRadius));
	// 	float smoothDelta = fwidth(distFromCenter);
	// 	Result.a *= smoothstep(CircleRadius, CircleRadius-smoothDelta, distFromCenter);
	// 	if (distFromCenter > CircleRadius - Value0 - smoothDelta/2) { fillColor = Color2; }
	// }
	// if (worldCoord.x < CircleRadius && worldCoord.y > ShiftVec.y - CircleRadius)
	// {
	// 	float distFromCenter = length(worldCoord - vec2(CircleRadius, ShiftVec.y - CircleRadius));
	// 	float smoothDelta = fwidth(distFromCenter);
	// 	Result.a *= smoothstep(CircleRadius, CircleRadius-smoothDelta, distFromCenter);
	// 	if (distFromCenter > CircleRadius - Value0 - smoothDelta/2) { fillColor = Color2; }
	// }
	
	// Result *= fillColor;
}