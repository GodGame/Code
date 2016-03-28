#include "fx/PostDefine.fx"

Texture2D<float4> gTex      : register(t0);
StructuredBuffer<float> lum : register(t1);
Texture2D bloom             : register(t2);
Texture2D bloom16x16        : register(t3);

SamplerState PointSampler   : register (s0);
SamplerState LinearSampler  : register (s1);

#define	FRAME_BUFFER_WIDTH		1280
#define	FRAME_BUFFER_HEIGHT		960

static float2 gvQuadTexCoord[4] = { float2(0.0f, 0.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f), float2(1.0f, 1.0f) };

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
	int3 uvm            = int3(Input.pos.xy, 0);	// (u, v, level)
	float2 Tex          = float2((float)uvm.x / (float)g_param.z, (float)uvm.y / (float)g_param.w);

	float4 vColor       = gTex.SampleLevel(PointSampler, Tex, 0);
	float fLum          = lum[0] * g_param.x;
	float3 vBloom       = bloom.Sample(LinearSampler, Tex);
	float3 vBloomScaled = bloom16x16.Sample(LinearSampler, Tex);
	//return vColor;

	// Tone mapping
	//vColor            = LumToColor(vColor).rgbr;
	//return vColor;
	float middle        = CalculateMiddleGray(fLum);

	vColor = CalculateToneColor(vColor, fLum, middle);
	//vBloom = CalculateToneColor(vColor, fLum, middle);
	//vBloomScaled = CalculateToneColor(vColor, fLum, middle);

	vColor.rgb += (0.4f * vBloom + 0.4f * vBloomScaled);//vBloom;// max(vBloom, vBloomScaled);
//	vColor.a = 1.0f;

	return vColor;//vColor;
}

struct GS_UI
{
	// x, y -> pos / z, w -> size
	float4 DrawInfo		: POSITION;
};

struct PS_UI
{
	float4 posH			: SV_POSITION;
	float2 tex			: TEXCOORD;
};

GS_UI VS_UI_Draw(float4 input : POSITION)
{
	GS_UI output;
	output.DrawInfo = input;

	return output;
}

[maxvertexcount(4)]
void GS_UI_Draw(point GS_UI input[1], inout TriangleStream<PS_UI> triStream)
{
	PS_UI output[4];

	float2 fHalfSize = g_param.xy * 0.5f; //(g_param.x * 0.5f, g_param.y * 0.5f);
	float2 pos = input[0].DrawInfo.xy + g_param.zw;
	output[0].posH = float4(pos + float2(-input[0].DrawInfo.z, +input[0].DrawInfo.w), 0, 1);
	output[1].posH = float4(pos + float2(+input[0].DrawInfo.z, +input[0].DrawInfo.w), 0, 1);
	output[2].posH = float4(pos + float2(-input[0].DrawInfo.z, -input[0].DrawInfo.w), 0, 1);
	output[3].posH = float4(pos + float2(+input[0].DrawInfo.z, -input[0].DrawInfo.w), 0, 1);

	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		output[i].tex      = gvQuadTexCoord[i];
		output[i].posH.xy -= fHalfSize;
		output[i].posH.xy /= fHalfSize;

		triStream.Append(output[i]);
	}
	triStream.RestartStrip();
}

SamplerState gSampler : register (s0);

float4 PS_UI_Draw(PS_UI input) : SV_Target
{
	float4 color = gTex.SampleLevel(gSampler, input.tex, 0);

	if (color.a < 0.45f) discard;

	return color;
}