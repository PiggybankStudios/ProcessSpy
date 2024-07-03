
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

const float Pi32 = 3.1415927410125732421875;

uniform vec4 Color1;

in vec4 fColor1;
in vec2 fTexCoord1;
in vec3 fPosition;

out vec4 Result;

uniform vec2 Center;
uniform vec2 Radius;
uniform float Rotation;
uniform vec2 AngleRange;
uniform float Thickness;

float EllipseSignedDistance(vec2 uv)
{
	float distance = 1.e20;
	vec2 rightAxis = vec2(cos(Rotation), sin(Rotation));
	vec2 downAxis = vec2(-rightAxis.y, rightAxis.x);
	vec2 uvPrime = vec2(dot(uv - Center, rightAxis), dot(uv - Center, downAxis));
	uvPrime.x /= Radius.x;
	uvPrime.y /= Radius.y;
	float result = abs(length(uvPrime) - 1)/2;
	float angle = atan(uvPrime.y, uvPrime.x);
	if (angle < AngleRange.x) { angle += 2*Pi32; }
	if (angle > AngleRange.x + AngleRange.y) { angle -= 2*Pi32; }
	//TODO: Make this continuous to some degree so we get a nice rounded arc endpoint on each side
	if (angle < AngleRange.x) { result = 100; }
	if (angle > AngleRange.x + AngleRange.y) { result = 100; }
	return result;
}

void main()
{
	Result = fColor1 * Color1;
	float signedDistance = 1.0f - (EllipseSignedDistance(fTexCoord1) / (Thickness/2));
	float smoothDelta = fwidth(signedDistance);
	Result.a *= smoothstep(0.0f-smoothDelta/2, 0.0f+smoothDelta/2, signedDistance);
	if (signedDistance <= -1)
	{
		discard;
	}
}