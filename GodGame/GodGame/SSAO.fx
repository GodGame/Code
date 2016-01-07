#include "Define.fx"

Texture2D gtxtSSAO	   : register(t25);

struct SSAO_PSIN
{
	float4 posH			: SV_POSITION;
	float3 ToFarPlane	: NORMAL;
	float2 Tex			: TEXCOORD;
};

#define NUM_SSAO_OFFSET 14

cbuffer cbPerFrame : register(b6)
{
	float4x4 gViewToTexSpace; // 투영 행렬 * 텍스쳐 행렬
	float4 gOffsetVectors[NUM_SSAO_OFFSET];
	float4 gFrustumCorners[4];
};

cbuffer ssaobuffer
{
	// 차폐판정
	float gOcclusionRadius = 0.5f;
	float gOcclusionFadeStart = 0.2f;
	float gOcclusionFadeEnd = 2.0f;
	float gSurfaceEpsilon = 0.05f;
};
// 표본점 q가 p를 얼마나 가리는지를 distZ(깊이 차이)로 계산
float OcclusionFunction(float distZ)
{
	float occlusion = 0.0f;

	if (distZ > gSurfaceEpsilon)
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
		// distZ가 gOcclusionFadeStart 에서 gOcclusionFadeEnd로 증가함에 따라
		// 차폐도를 1에서 0으로 선형 감소
		occlusion = saturate((gOcclusionFadeEnd - distZ) / fadeLength);
	}
	else
		occlusion = 1.0f;
	return occlusion;
}

SSAO_PSIN VSSCeneSpaceAmbient(VS_SCENE_INPUT input)
{
	SSAO_PSIN output = (SSAO_PSIN)0;
	output.posH = float4(input.pos, 1.0f);
	output.ToFarPlane = gFrustumCorners[max(sign(input.tex.x) , 0) + max(sign(input.tex.y * 2), 0 )].xyz;
	output.Tex = input.tex;

	return output;
}

float4 PSSCeneSpaceAmbient(SSAO_PSIN input) :SV_Target
{
	// P-- 지금 주벼광 차폐를 계산하고자 하는 픽셀에 해당하는 점
	// n-- p에서의 법선 벡터
	// q-- p 주변의 한 무작위 점(표본점)
	// r-- p를 가릴 가능성이 있는 잠재적 차폐점

	// 이 픽셀의 시야 공간 법선과 z 성분을 가져온다. 지금 렌더링되는 화면 전체 사각형 텍스쳐 좌표는 이미 uv안에 있다.
	int3 uvm = int3(input.posH.xy, 0);	// (u, v, level)
	float4 normal = gtxtNormal.Load(uvm);

	// 완전한 시야공간 위치 (x, y, z) 재구축
	// 우선 p = t * input.ToFarPlane을 만족하는 t를 구한다.
	// p.z = t * input.ToFarPlane.z ==> t = p.z / input.ToFarPlane.z
	float3 p = (normal.w / input.ToFarPlane.z) * input.ToFarPlane;

	// 무작위 벡터를 추출해서 [0, 1] -> [-1, 1] 매핑
	float3 randVec = 2.0f * gtxtRandom.SampleLevel(gSamplerState, 4.0f * input.Tex, 0.0f).rgb - 1.0f;

	float occlusionSum = 0.0f;
	// p 주변의 이웃 표본점들을 n방향 반구에서 추출한다.
	[unroll]
	for (int i = 0; i < NUM_SSAO_OFFSET; ++i)
	{
		// 미리 만들어 둔 상수 오프셋 벡터들은 고르게 분포 되어있다. 
		// (즉, 오프셋의 벡터들은 같은 방향으로 뭉쳐있지 않다.)
		// 이들을 하나의 무작위 벡터 기준으로 반사시키면 무작위 벡터들이 만들어 진다.
		float3 offset = reflect(gOffsetVectors[i].xyz, randVec);

		// 오프셋 벡터가 (p, n)으로 정의된 평면의 뒤쪽을 향하고 있으면 반대로 뒤집는다.
		float flip = sign(dot(offset, normal.xyz));

		// p주변에서 차폐 반지름 이내의 무작위 점 q를 선택한다.
		float3 q = p + flip * gOcclusionRadius * offset;
		//return float4(p, 1);
		// q를 투영해서 투영 텍스쳐 좌표를 구한다.
		float4 ProjQ = mul(float4(q, 1.0f), gViewToTexSpace);
		//float fDepth = (ProjQ.w * gfDepthFar);
		ProjQ /= (ProjQ.w);// *gfDepthFar);
		//return float4(ProjQ);//float4(ProjQ.x / 1280, ProjQ.y / 960, 0, 0);

		// 시점에서 q로의 반직선에서 시점에 가장 가까운 픽셀의 깊이를 구한다.
		// (이것은 q의 깊이는 아니다. q는 그냥 p 근처의 임의 점이며, 장면 물체가 아닌 빈공간에 있는 점일 수 있다.)
		// 가장 가까운 깊이는 깊이 맵에서 추출한다
		float4 rz = gtxtNormal.Load(int3(ProjQ.xy, 0));
		//return rz;
		// 완전한 시야 공간 위치 r = (rx, ry, rz)를 재구축한다.
		// r은 q를 지나는 반직선에 있으므로, r = t * q를 만족하는 t가 존재한다.
		// r.z = t*q.z ==> t = r.z / q.z
		float3 r = (rz / q.z) * q;;;
		//return float4(r, 1);
		/* 
		r이 p를 가리는지 판정.
		   내적 dot(n, normalize(r - p))는 잠재적 차폐점 r이 평면 plane(p, n) 앞쪽으로 얼마나 앞에 있는지를 나타낸다.
		   더 앞에 있을 수록, 차폐도의 가중치를 더 크게 잡는다. 이렇게 하면 r이 시선과 직각인 평면 (p, n)에 있을때 
		   시점 기준의 깊이 값 차이 때문에 r이 p를 가린다고 잘못 판정하는 상황도 방지된다.

		   차폐도는 현재 점 p와 차폐점 r 사이의 거리에 의존한다.
		   r이 p에서 너무 멀리 있으면 p를 가리지 않는 것으로 간주한다.
		*/
		float distZ = p.z - r.z;
		float dp = max(dot(normal.xyz, normalize(r - p)), 0.0f);
		float occlusion = /*dp */ 1 * OcclusionFunction(distZ);
		return occlusion;
		occlusionSum += occlusion;
	}
	occlusionSum /= NUM_SSAO_OFFSET;

	float access = 1.0f - occlusionSum;

	//SSAO가 좀 더 극명한 효과를 내도록 거듭 제곱를 이용해 대비를 강화한다.
	return saturate(pow(access, 4.0f));
}


