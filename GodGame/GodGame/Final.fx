Texture2D<float4> tex : register(t0);
StructuredBuffer<float> lum : register(t1);
Texture2D<float4> bloom : register(t2);

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

static float  MIDDLE_GRAY = 0.72f;
static  float  LUM_WHITE = 1.5f;

float4 ToneMapping(float4 LinearColor)
{
	return (LinearColor * (6.2 * LinearColor + 0.5)) / (LinearColor * (6.2 * LinearColor + 1.7) + 0.06);
}


float3 LumToColor(float3 fLum)
{
	float3 XYZ;
	// Tone
	XYZ.r = fLum.r * fLum.g / fLum.b;
	XYZ.g = fLum.r;
	XYZ.b = fLum.r * (1 - fLum.g - fLum.b) / fLum.b;

	const float3x3 XYZtoRGB = {
		2.5651, -1.1665, -0.3986,
		-1.0217, 1.9777, 0.0439,
		0.0753, -0.2543, 1.1892
	};
	//const float3x3 XYZtoRGB = {
	//	3.2405, -1.5371, -0.4985,
	//	-0.9693, 1.8760, 0.0416,
	//	0.0556, -0.2040, 1.0572
	//};

	return mul(XYZtoRGB, XYZ);
}

float4 PSFinalPass(PS_SCENE_INPUT Input) : SV_Target
{
	int3 uvm = int3(Input.pos.xy, 0);	// (u, v, level)
	float2 Tex = float2((float)uvm.x / (float)g_param.z, (float)uvm.y / (float)g_param.w);

	float4 vColor = tex.Sample(LinearSampler, Tex);
	float fLum = lum[0] * g_param.x * 0.5f;
	float3 vBloom = bloom.Sample(LinearSampler, Input.tex);
	//return vColor;

	// Tone mapping
	//vColor = LumToColor(vColor).rgbr;
	//return vColor;
	vColor.rgb *= MIDDLE_GRAY / (fLum + 0.001f);
	vColor.rgb *= (1.0f + vColor.rgb / LUM_WHITE);
	vColor.rgb /= (1.0f + vColor.rgb);

	//float fLumAdapted = /*fLumAdapted*/ + (fLum - )
	//float fLumAdapted = fLum * 0.2f + (1 - fLum) * 0.4f;
	//float fLumAdapted = fLum;
	//float LumScaled = vColor.r * MIDDLE_GRAY / (fLumAdapted + 0.001f);
	//vColor.r = (LumScaled * (1.0f + LumScaled / LUM_WHITE)) / (1.0f + LumScaled);

	vColor.rgb += 0.6f * vBloom;
//	vColor.a = 1.0f;

	return  vColor;//vColor;
}
