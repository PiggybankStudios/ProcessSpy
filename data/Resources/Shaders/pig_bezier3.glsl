
//======================== VERTEX_SHADER ========================

#version 130

uniform mat4 WorldMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

// layout(location = 0) 
in vec3 inPosition;
in vec4 inColor1;
in vec2 inTexCoord1;

out vec4 fColor1;
out vec2 fTexCoord1;
out vec3 fPosition;

void main()
{
	fPosition = inPosition;
	fColor1 = inColor1;
	fTexCoord1 = inTexCoord1;
	mat4 transformMatrix = ProjectionMatrix * (ViewMatrix * WorldMatrix);
	gl_Position = transformMatrix * vec4(inPosition, 1.0);
}

//======================== FRAGMENT_SHADER ========================

#version 130

uniform vec4 Color1;

in vec4 fColor1;
in vec2 fTexCoord1;
in vec3 fPosition;

out vec4 Result;

//From https://www.shadertoy.com/view/ldXXWH
//Which I found through https://www.shadertoy.com/view/4sKyzW
#define BEZIER_POINT_COUNT 20

uniform vec2 StartPos;
uniform vec2 Control;
uniform vec2 EndPos;
uniform float Thickness;

vec2 Bezier(float time)
{
	float invTime = (1.0 - time);
	return invTime*invTime*StartPos + 2*time*invTime*Control + time*time*EndPos;
}
float LineSignedDistance(vec2 point, vec2 lineStart, vec2 lineEnd)
{
	lineEnd -= lineStart;
	point -= lineStart;
	point = point - lineEnd * clamp(dot(point, lineEnd) / (dot(lineEnd, lineEnd)), 0, 1); //Fortunately it still works well when lineStart==endPos => division by 0
	return dot(point, point);
}
float Bezier3SignedDistance(vec2 uv)
{
	float distance = 1.e20;
	float time = 0;
	vec2 prevPoint = Bezier(time);
	for (int i = 0; i < BEZIER_POINT_COUNT; i++)
	{
		time += 1.0 / float(BEZIER_POINT_COUNT);
		vec2 newPoint = Bezier(time);
		distance = min(distance, LineSignedDistance(uv, prevPoint, newPoint));
		prevPoint = newPoint;
	}
	return sqrt(distance);
}

void main()
{
	Result = fColor1 * Color1;
	float signedDistance = 1.0f - (Bezier3SignedDistance(fTexCoord1) / (Thickness/2));
	float smoothDelta = fwidth(signedDistance);
	Result.a *= smoothstep(0.0f-smoothDelta/2, 0.0f+smoothDelta/2, signedDistance);
	if (signedDistance <= -1)
	{
		discard;
	}
}