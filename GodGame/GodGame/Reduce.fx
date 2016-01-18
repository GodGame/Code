
#include "Define.fx"

Texture2D gtxtInput : register(t0);
StructuredBuffer<float> Input : register(t1);
RWStructuredBuffer<float> fLum : register(u1);


cbuffer computeInfo : register(b0)
{
	uint4    g_param; // x, y : dispatch call, w, a : inputsize
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

	accum[GI] = s.r;// dot(s, LUM_VECTOR); //(s.a); //s.r; dot(s.rgb, float3(0.3, 0.3, 0.3)); //

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
	}
}
