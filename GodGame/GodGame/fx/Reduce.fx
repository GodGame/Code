#include "PostDefine.fx"

Texture2D gtxtInput : register(t0);
StructuredBuffer<float> Input : register(t1);
//StructuredBuffer<float> LastLum : register(t2);
RWStructuredBuffer<float> fLum : register(u1);
RWStructuredBuffer<float> fLastLum : register(u2);


cbuffer computeInfo : register(b0)
{
	float4    g_param; // x, y : dispatch call, w, a : inputsize
					  //uint2 gDispatchCalls;
					  //uint2 gInputSize;
};

#define Threadnums 8

groupshared float accum[64];
static const float4 LUM_VECTOR = float4(.299, .587, .114, 0);
[numthreads(Threadnums, Threadnums, 1)]
void LumCompression(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	uint2 gDispatchCalls = g_param.xy;
	//uint2 

	float4 s =
		gtxtInput.Load(uint3(DTid.xy, 0)) +
		gtxtInput.Load(uint3(DTid.xy + uint2(Threadnums * gDispatchCalls.x, 0), 0)) +
		gtxtInput.Load(uint3(DTid.xy + uint2(0, Threadnums * gDispatchCalls.y), 0)) +
		gtxtInput.Load(uint3(DTid.xy + uint2(Threadnums * gDispatchCalls.x, Threadnums * gDispatchCalls.y), 0));

#ifdef LUMCOLOR
	accum[GI] = s.r;//(s.a); dot(s.rgb, float3(0.3, 0.3, 0.3)); //
#else
	accum[GI] = dot(s, LUM_VECTOR); //max(0.2, dot(s, LUM_VECTOR));
#endif

	GroupMemoryBarrierWithGroupSync();
	if (GI < 32)
		accum[GI] += accum[32 + GI];

	GroupMemoryBarrierWithGroupSync();
	if (GI < 16)
		accum[GI] += accum[16 + GI];

	GroupMemoryBarrierWithGroupSync();
	if (GI < 8)
		accum[GI] += accum[8 + GI];

	GroupMemoryBarrierWithGroupSync();
	if (GI < 4)
		accum[GI] += accum[4 + GI];

	GroupMemoryBarrierWithGroupSync();
	if (GI < 2)
		accum[GI] += accum[2 + GI];

	GroupMemoryBarrierWithGroupSync();
	if (GI < 1)
		accum[GI] += accum[1 + GI];

	if (GI == 0)
	{
		float result = accum[0];
		float index = Gid.y * gDispatchCalls.x + Gid.x;

		fLum[index] = result;

		//fLum[Gid.y * gDispatchCalls.x + Gid.x] = accum[0];
	}
}

#define groupthreads 128
groupshared float accumulate[groupthreads];
[numthreads(groupthreads, 1, 1)]
void ReduceToSingle(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	if (DTid.x < g_param.x)
		accumulate[GI] = Input[DTid.x];
	else
		accumulate[GI] = 0;

	// Parallel reduction algorithm follows 
	GroupMemoryBarrierWithGroupSync();
	if (GI < 64)
		accumulate[GI] += accumulate[64 + GI];

	GroupMemoryBarrierWithGroupSync();
	if (GI < 32)
		accumulate[GI] += accumulate[32 + GI];

	GroupMemoryBarrierWithGroupSync();
	if (GI < 16)
		accumulate[GI] += accumulate[16 + GI];

	GroupMemoryBarrierWithGroupSync();
	if (GI < 8)
		accumulate[GI] += accumulate[8 + GI];

	GroupMemoryBarrierWithGroupSync();
	if (GI < 4)
		accumulate[GI] += accumulate[4 + GI];

	GroupMemoryBarrierWithGroupSync();
	if (GI < 2)
		accumulate[GI] += accumulate[2 + GI];

	GroupMemoryBarrierWithGroupSync();
	if (GI < 1)
		accumulate[GI] += accumulate[1 + GI];


	if (GI == 0)
	{
		fLum[Gid.x] = accumulate[0];
		//fLastLum[0] = accumulate[0];
	}
}

#define framesnum 16
groupshared float lums[framesnum + 1];
[numthreads(framesnum, 1, 1)]
void LumAdapted(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	//g_param.x = 가중치 곱, y = 가중치 합, z = 프레임타임 누적 (최대 1초 ), w = 프레임 타임
	lums[GI] = fLastLum[GI];

	GroupMemoryBarrierWithGroupSync();

	if (GI == 0) 
	{
		float fAccum = Input[0];
		float fAdapted = lums[framesnum - 1]; // fLastLum[15];
		float fAdaptedAverage = fAdapted;
		int iGreater = (fAdapted >= fAccum);

		[unroll]
		for (int i = framesnum - 1; i >= 0; --i)
		{
			fAdaptedAverage += lums[i];
			iGreater += (lums[i] >= fAccum);
		}
		fAdaptedAverage = (0.0625f * fAdaptedAverage) * g_param.x + g_param.y;

		float fResult = 0;
		if (iGreater >= (framesnum - 0.01f))
		{
			float ftau = 0.4f * g_param.w;
			fResult = fAdapted + (fAdaptedAverage - fAccum) * (1 - exp(ftau));
		}
		else if (iGreater <= 0.1f)
		{
			float ftau = 0.05f * g_param.w;
			fResult = fAdapted + (fAdaptedAverage - fAccum) * (1 - exp(ftau));
		}
		else
		{
			fResult = fAdapted;// fAdapted - (0.1f * g_param.z) * (fAccum - fAdapted);
		}

		fLum[0] = fResult;
		lums[framesnum] = fResult;
	}
	GroupMemoryBarrierWithGroupSync();

	fLastLum[GI] = lums[GI + 1];
}
//if (fAdaptedAverage >= fAccum)//iGreater >= 15.9)
//{
//	fResult = fAdaptedAverage - (0.1 * g_param.z) * (fAccum - fAdaptedAverage);

//}
//else //if(iGreater <= 0.1f)
//{
//	fResult = fAdaptedAverage + (0.1 * g_param.z) * (fAccum - fAdaptedAverage);
//}


