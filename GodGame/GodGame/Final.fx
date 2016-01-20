#include "PostDefine.fx"

Texture2D<float4> tex : register(t0);
StructuredBuffer<float> lum : register(t1);
Texture2D bloom : register(t2);
Texture2D bloom8x8 : register(t3);


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

	float4 vColor = tex.SampleLevel(PointSampler, Tex, 0);
	float fLum = lum[0] * g_param.x;
	float3 vBloom = bloom.Sample(LinearSampler, Tex);
	float3 vBloomScaled = bloom8x8.Sample(LinearSampler, Tex);
	//return vColor;

	// Tone mapping
	//vColor = LumToColor(vColor).rgbr;
	//return vColor;
	float middle = CalculateMiddleGray(fLum);
	
	vColor = CalculateToneColor(vColor, fLum, middle);
	//vBloom = CalculateToneColor(vColor, fLum, middle);
	//vBloomScaled = CalculateToneColor(vColor, fLum, middle);


	vColor.rgb += max(0.6f * vBloom, 0.4f * vBloomScaled);//vBloom;// max(vBloom, vBloomScaled);
//	vColor.a = 1.0f;

	return vColor;//vColor;
}
