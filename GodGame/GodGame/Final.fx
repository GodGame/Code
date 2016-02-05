#include "PostDefine.fx"

Texture2D<float4> gTex : register(t0);
StructuredBuffer<float> lum : register(t1);
Texture2D bloom : register(t2);
Texture2D bloom16x16 : register(t3);

SamplerState PointSampler : register (s0);
SamplerState LinearSampler : register (s1);

#define	FRAME_BUFFER_WIDTH		1280
#define	FRAME_BUFFER_HEIGHT		960

cbuffer cbPS : register(b0)
{
	float4    g_param;
};

struct PS_SCENE_INPUT
{
	float4 pos			: SV_POSITION;
	float2 tex			: TEXCOORD;
};

float4 PSFinalPass(PS_SCENE_INPUT Input) : SV_Target
{
	int3 uvm = int3(Input.pos.xy, 0);	// (u, v, level)
	float2 Tex = float2((float)uvm.x / (float)g_param.z, (float)uvm.y / (float)g_param.w);

	float4 vColor = gTex.SampleLevel(PointSampler, Tex, 0);
	float fLum = lum[0] * g_param.x;
	float3 vBloom = bloom.Sample(LinearSampler, Tex);
	float3 vBloomScaled = bloom16x16.Sample(LinearSampler, Tex);
	//return vColor;

	// Tone mapping
	//vColor = LumToColor(vColor).rgbr;
	//return vColor;
	float middle = CalculateMiddleGray(fLum);

	vColor = CalculateToneColor(vColor, fLum, middle);
	//vBloom = CalculateToneColor(vColor, fLum, middle);
	//vBloomScaled = CalculateToneColor(vColor, fLum, middle);

	vColor.rgb += (0.3f * vBloom + 0.4f * vBloomScaled);//vBloom;// max(vBloom, vBloomScaled);
//	vColor.a = 1.0f;

	return vColor;//vColor;
}

struct VS_UI
{
	float4 DrawInfo		: POSITION;	// x, y -> pos z, w -> size
};

struct GS_UI
{
	float4 DrawInfo		: POSITION;
};

struct PS_UI
{
	float4 pos			: SV_POSITION;
	float2 tex			: TEXCOORD;
};

GS_UI VS_UI_Draw(VS_UI input)
{
	GS_UI output;
	output.DrawInfo = input.DrawInfo;

	return output;
}

[maxvertexcount(6)]
void GS_UI_Draw(point GS_UI input[1], uint primID : SV_PrimitiveID,
	inout TriangleStream<PS_UI> triStream)
{
	GS_UI inPoint = input[0];
	PS_UI output[4];

	output[0].pos = float4(inPoint.DrawInfo.xy, 0, 0) + float4(-inPoint.DrawInfo.z, +inPoint.DrawInfo.w, 0, 0);
	output[0].pos = float4(-0, -0, 0, 0);
	output[0].tex = float2(0, 0);

	output[1].pos = float4(inPoint.DrawInfo.xy, 0, 0) + float4(+inPoint.DrawInfo.z, +inPoint.DrawInfo.w, 0, 0);
	output[1].pos = float4(FRAME_BUFFER_WIDTH, -0, 0, 0);
	output[1].tex = float2(1, 0);

	output[2].pos = float4(inPoint.DrawInfo.xy, 0, 0) + float4(-inPoint.DrawInfo.z, -inPoint.DrawInfo.w, 0, 0);
	output[2].pos = float4(-0, FRAME_BUFFER_HEIGHT, 0, 0);
	output[2].tex = float2(0, 1);

	output[3].pos = float4(inPoint.DrawInfo.xy, 0, 0) + float4(+inPoint.DrawInfo.z, -inPoint.DrawInfo.w, 0, 0);
	output[3].pos = float4(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0, 0);
	output[3].tex = float2(1, 1);

	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		output[i].pos.xy /= float2(FRAME_BUFFER_WIDTH  * 10, FRAME_BUFFER_HEIGHT * 10);
		//output[i].pos.xy -= 0.1f;
		triStream.Append(output[i]);
	}
	triStream.RestartStrip();
}

SamplerState gSampler : register (s0);

float4 PS_UI_Draw(PS_UI input) : SV_Target
{
	float4 color = gTex.Sample(gSampler, input.tex);

//	if (color.a == 0.0f) discard;

	return color;
}