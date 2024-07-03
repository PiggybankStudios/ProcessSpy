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
out vec2 fSampleCoord;
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

uniform float CircleRadius; //used as actual circle radius
uniform float CircleInnerRadius; //used as a base percentage to start rendering at (for drawing pie charts with more than 8 pieces using multiple passes)
uniform vec4 Color1;

const float Pi32 = 3.1415927410125732421875;
const vec3 PieColor1 = vec3(249.0/255.0,  38.0/255.0, 114.0/255.0); //0xFFF92672
const vec3 PieColor2 = vec3(253.0/255.0, 151.0/255.0,  31.0/255.0); //0xFFFD971F
const vec3 PieColor3 = vec3(230.0/255.0, 219.0/255.0, 116.0/255.0); //0xFFE6DB74
const vec3 PieColor4 = vec3(166.0/255.0, 226.0/255.0,  46.0/255.0); //0xFFA6E22E
const vec3 PieColor5 = vec3(102.0/255.0, 155.0/255.0, 239.0/255.0); //0xFF669BEF
const vec3 PieColor6 = vec3(174.0/255.0, 129.0/255.0, 255.0/255.0); //0xFFAE81FF
const vec3 PieColor7 = vec3(251.0/255.0, 108.0/255.0, 127.0/255.0); //0xFFFB6C7F
const vec3 PieColor8 = vec3(254.0/255.0, 182.0/255.0,  75.0/255.0); //0xFFFEB64B
const vec3 PieColor9 = vec3(144.0/255.0, 136.0/255.0,  97.0/255.0); //0xFF908861

//each of these values represent a percentage pie piece
uniform float Value0;
uniform float Value1;
uniform float Value2;
uniform float Value3;
uniform float Value4;
uniform float Value5;
uniform float Value6;
uniform float Value7;

in vec4 fColor1;
in vec2 fTexCoord1;
in vec3 fPosition;

out vec4 Result;

void main()
{
	vec2 circleCoord = fTexCoord1 - vec2(0.5, 0.5);
	float distFromCenter = length(circleCoord) * 2;
	float angle = atan(circleCoord.y, circleCoord.x) / (Pi32*2);
	if (angle < 0) { angle += 1; }
	
	Result = vec4(1, 1, 1, 1);
	if (CircleRadius != 0)
	{
		float smoothDelta = fwidth(distFromCenter);
		Result.a *= smoothstep(CircleRadius, CircleRadius-smoothDelta, distFromCenter);
	}
	
	if      (angle < CircleInnerRadius)                                                                         { Result.a   = 0; }
	else if (angle < CircleInnerRadius + Value0)                                                                { Result.rgb = PieColor1; }
	else if (angle < CircleInnerRadius + Value0 + Value1)                                                       { Result.rgb = PieColor2; }
	else if (angle < CircleInnerRadius + Value0 + Value1 + Value2)                                              { Result.rgb = PieColor3; }
	else if (angle < CircleInnerRadius + Value0 + Value1 + Value2 + Value3)                                     { Result.rgb = PieColor4; }
	else if (angle < CircleInnerRadius + Value0 + Value1 + Value2 + Value3 + Value4)                            { Result.rgb = PieColor5; }
	else if (angle < CircleInnerRadius + Value0 + Value1 + Value2 + Value3 + Value4 + Value5)                   { Result.rgb = PieColor6; }
	else if (angle < CircleInnerRadius + Value0 + Value1 + Value2 + Value3 + Value4 + Value5 + Value6)          { Result.rgb = PieColor7; }
	else if (angle < CircleInnerRadius + Value0 + Value1 + Value2 + Value3 + Value4 + Value5 + Value6 + Value7) { Result.rgb = PieColor8; }
	else                                                                                                        { Result.a   = 0; }
	
	Result = fColor1 * Color1 * Result;
}