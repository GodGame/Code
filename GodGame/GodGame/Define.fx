

Texture2D gtxtTexture : register(t0);
SamplerState gSamplerState : register(s0);

Texture2D gtxtDetailTexture : register(t1);
SamplerState gDetailSamplerState : register(s1);

TextureCube gtxtSkyBox : register(t2);
SamplerState gssSkyBox : register(s2);

Texture1D gtxtRandom : register(t9);

//Texture2D gtxtResult   : register(t16);
Texture2D gtxtTxColor  : register(t17);
Texture2D gtxtPos      : register(t18);
Texture2D gtxtDiffuse  : register(t19);	// ���� ��ǻ�� �Ϲ��� �ؽ��� ����
Texture2D gtxtSpecular : register(t20);	// ���� ����ŧ��
Texture2D gtxtNormal   : register(t21);
Texture2DArray gTextureArray : register(t10);


#include "Light.fx"

#define  MRT_NUM	5

//ī�޶� ��ȯ ��İ� ���� ��ȯ ����� ���� ���̴� ������ �����Ѵ�(���� 0�� ���).
cbuffer cbViewProjectionMatrix : register(b0)
{
	matrix gmtxView;
	matrix gmtxViewProjection;

	static float gfCameraFar = 2000.0f;
	static float gfDepthFar = 0.001f;
};

//���� ��ȯ ����� ���� ���̴� ������ �����Ѵ�(���� 1�� ���). 
cbuffer cbWorldMatrix : register(b1)
{
	matrix gmtxWorld : packoffset(c0);
};

// ī�޶� ��ġ�� ���� �������
cbuffer cbCameraPosition : register(b2)
{
	float4 gf3CameraPos : packoffset(c0);
};

cbuffer cbTerrain
{
	static int gWorldCell = 128;
	static int gHegiht = 512;
	static float gCameraMax = 1000.0f;
	static float gCameraMin = 20.0f;

	static float gScaleHeight = 32.0f;
};

cbuffer cbFixed
{
	static float2 gvQuadTexCoord[4] = { float2(1.0f, 1.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f), float2(0.0f, 0.0f) };
};

cbuffer cbDisplacement : register(b3)
{
	float3 gBumpScale;
	float  gnBumpMax;
};


cbuffer cbShadow : register(b5)
{
	matrix gmtxShadowTransform : packoffset(c0);
}



/*(����) register(b0)���� b�� �������Ͱ� ��� ���۸� ���� ���Ǵ� ���� �ǹ��Ѵ�. 0�� ���������� ��ȣ�̸�
���� ���α׷����� ��� ���۸� ����̽� ���ؽ�Ʈ�� ������ ���� ���� ��ȣ�� ��ġ�ϵ��� �ؾ� �Ѵ�.
pd3dDeviceContext->VSSetConstantBuffers(VS_SLOT_WORLD_MATRIX, 1, &m_pd3dcbWorldMatrix);*/
//����-���̴��� ����� ���� ����ü�̴�.

struct PS_MRT_COLOR_OUT
{
	// �ִ� 8�� ����
	float4 color  : SV_Target0;
	float4 zDepth : SV_Target1;
	float4 colorR : SV_Target2;
	float4 colorG : SV_Target3;
	float4 colorB : SV_Target4;
};

struct PS_MRT_OUT
{
	// �ִ� 8�� ����	
	
	float4 vTxColor : SV_Target0;
	float4 vPos		: SV_Target1;
	float4 vDiffuse : SV_Target2;
	float4 vSpec	: SV_Target3;
	float4 vNormal  : SV_Target4;
	//float4 vDepth : SV_Target4;
};

struct VS_INPUT
{
	float3	position	: POSITION;
	//float4 color		: COLOR;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

struct VS_SCENE_INPUT
{
	float3 pos			: POSITION;
	float2 tex			: TEXCOORD;
};


struct PS_SCENE_INPUT
{
	float4 pos			: SV_POSITION;
	float2 tex			: TEXCOORD;
};


struct VS_INSTANCED_COLOR_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR0;
	column_major float4x4 mtxTransform : INSTANCEPOS;
	float4 instanceColor : INSTANCECOLOR;
};

struct VS_INSTANCED_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
	//�ý��� ���� ������ ���� ���̴��� ���޵Ǵ� ��ü �ν��Ͻ��� ID�� �ȼ� ���̴��� �����Ѵ�.
	float4 instanceID : INDEX;
};

//������ ������ ���� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_DIFFUSED_COLOR_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR0;
};

//������ ������ ���� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_DIFFUSED_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

//�ν��Ͻ��� �ϸ鼭 ������ ������ ���� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_INSTANCED_DIFFUSED_COLOR_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR0;
	float4x4 mtxTransform : INSTANCEPOS;
};

struct VS_INSTANCED_DIFFUSED_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

//������ ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_LIGHTING_COLOR_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

//������ ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_LIGHTING_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	//������ǥ�迡�� ������ ��ġ�� ���� ���͸� ��Ÿ����.
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
};

//�ν��Ͻ��� �ϸ鼭 ������ ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_INSTANCED_LIGHTING_COLOR_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float4x4 mtxTransform : INSTANCEPOS;
};

//�ν��Ͻ��� �ϸ鼭 ������ ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_INSTANCED_LIGHTING_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
};

//�ؽ��ĸ� ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_TEXTURED_COLOR_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
};

//�ؽ��ĸ� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_TEXTURED_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	//float4 shadowPos : SHADOW;
	float3 posW		: POSITION;
	float2 texCoord : TEXCOORD0;
};

// ��ī�̹ڽ� ���� ���̴�
struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3	position : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL : POSITION;
	float4	position : SV_POSITION;
};


//�ν��Ͻ��� �ϸ鼭 �ؽ��ĸ� ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_INSTANCED_TEXTURED_COLOR_INPUT
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD0;
	float4x4 mtxTransform : INSTANCEPOS;
};

//�ν��Ͻ��� �ϸ鼭 �ؽ��ĸ� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_INSTANCED_TEXTURED_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};
//-------------------------------------------------------------------------------------------------------------------------
//������ �ؽ��ĸ� ����ϴ� ��� ���� ���̴��� �Է°� ����� ���� ����ü�̴�.
struct VS_DETAIL_TEXTURED_COLOR_INPUT
{
	float3 position : POSITION;
	float2 texCoordBase : TEXCOORD0;
	float2 texCoordDetail : TEXCOORD1;
};

struct VS_DETAIL_TEXTURED_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texCoordBase : TEXCOORD0;
	float2 texCoordDetail : TEXCOORD1;
};

//-------------------------------------------------------------------------------------------------------------------------
//�ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_TEXTURED_LIGHTING_COLOR_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
};

//�ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_TEXTURED_LIGHTING_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	//float4 shadowPos : SHADOW;
	float3 normalW : NORMAL;
	float2 texCoord : TEXCOORD0;
};
//--------------------------------------------------------------------------------------------------------------------
struct VS_DETAIL_TEXTURED_LIGHTING_COLOR_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoordBase : TEXCOORD0;
	float2 texCoordDetail : TEXCOORD1;
};

//������ �ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_DETAIL_TEXTURED_LIGHTING_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 texCoordBase : TEXCOORD0;
	float2 texCoordDetail : TEXCOORD1;
};

struct VS_SPLAT_TEXTURED_LIGHTING_COLOR_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoordBase  : TEXCOORD0;
	float2 texCoordAlpha : TEXCOORD1;
};

struct VS_SPLAT_TEXTURED_LIGHTING_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	//float4 shadowPos : SHADOW;
	float3 normalW : NORMAL;
	float2 texCoordBase  : TEXCOORD0;
	float2 texCoordAlpha : TEXCOORD1;
};

//--------------------------------------------------------------------------------------------------------------------
//�ν��Ͻ�, �ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_INSTANCED_TEXTURED_LIGHTING_COLOR_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD0;
	float4x4 mtxTransform : INSTANCEPOS;
};

//�ν��Ͻ�, �ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_INSTANCED_TEXTURED_LIGHTING_COLOR_OUTPUT
{
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float3 normalW : NORMAL;
	float2 texCoord : TEXCOORD0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

// ������� ����ü�̴� --------------------------------------------------------------------------------------------------
struct VS_BILLBOARD_INPUT
{
	float3 pos : POSITION;
	float2 sizeW : SIZE;
	float4 posW : INSTANCEPOS;
};

struct VS_BILLBOARD_OUTPUT
{
	float3 centerW : POSITION;
	float2 sizeW : SIZE;
};

struct GS_BILLBOARD_OUTPUT
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float2 texCoord : TEXCOORD;
	uint primID : SV_PrimitiveID;
};



struct VS_BILLBOARD_CUBE_INPUT
{
	float3 pos : POSITION;
	float sizeW : SIZE;
};

struct VS_BILLBOARD_CUBE_OUTPUT
{
	float3 centerW : POSITION;
	float sizeW : SIZE;
};

struct GS_BILLBOARD_CUBE_OUTPUT
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float2 texCoord : TEXCOORD;
	uint primID : SV_PrimitiveID;
};

struct VS_INSTANCE_CUBE_INPUT
{
	//	float3 pos : POSITION;
	float sizeW : SIZE;
	//float4x4 mtxW : INSTANCEPOS;
	float4 posW : INSTANCEPOS;
};

struct VS_INSTANCE_CUBE_OUTPUT
{
	float sizeW : SIZE;
	float3 centerW : POSITION;
	//float3 pos : POSITION

};

struct GS_INSTANCE_OUTPUT
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float2 texCoord : TEXCOORD;
	//uint primID : SV_PrimitiveID;
};

struct VS_INSTANCE_SPHERE_INPUT
{
	float4 info	: INFO;
	float4 posW : INSTANCEPOS;
};

struct VS_INSTANCE_SPHERE_OUTPUT
{
	float4 info : INFO;
	float3 centerW : POSITION;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

struct FLOAT3_POS
{
	float3 pos : POSITION;
};

struct HCS_EDGE4_IN2
{
	float fTessEdges[4]   : SV_TessFactor;
	float fTessInsides[2] : SV_InsideTessFactor;
};


struct DS_BEZIER_OUTPUT
{
	float4 pos : SV_POSITION;
};

float4 BernsteinCoefficient(float t)
{
	float tlnv = 1.0f - t;
	return float4(tlnv * tlnv * tlnv, 3.0f * t * tlnv * tlnv, 3.0f * t * t * tlnv, t* t* t);
}


struct FLOAT3_POS_FLOAT2_TEX
{
	float3 pos : POSITION;
	float2 tex : TEXCOORD0;
};


float3 CubicBezierSum(OutputPatch<FLOAT3_POS_FLOAT2_TEX, 16> patch, float4 u, float4 v)
{
	float3 sum = float3(0.0f, 0.0f, 0.0f);
	sum  = v.x * (u.x * patch[0].pos + u.y * patch[1].pos + u.z * patch[2].pos + u.w * patch[3].pos);
	sum += v.y * (u.x * patch[4].pos + u.y * patch[5].pos + u.z * patch[6].pos + u.w * patch[7].pos);
	sum += v.z * (u.x * patch[8].pos + u.y * patch[9].pos + u.z * patch[10].pos + u.w * patch[11].pos);
	sum += v.w * (u.x * patch[12].pos + u.y * patch[13].pos + u.z * patch[14].pos + u.w * patch[15].pos);
	return sum;
}

///////////////////////////////////////////////////

float CalculateTessFactor(float3 p)
{
	float fDistToCamera = distance(p, gf3CameraPos);
	float s = saturate((fDistToCamera - gCameraMin) / (gCameraMax - gCameraMin));
	return pow(2, lerp(8.0f, 2.0f, s));
}

#define PSNORMAL

struct DETAIL_TERRAIN
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float2 tex  : TEXCOORD0;
	float2 texDetail : TEXCOORD1;
#ifdef DSNORMAL
	float3 normalW: NORMAL;
#endif
};

//////////////////////////////////////////////////////////

struct MODEL_NORMALMAP
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float2 tex : TEXCOORD;
};

struct WORLD_NORMALMAP
{
	float3 posW : POSITION;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float2 tex : TEXCOORD;
};

struct PS_WORLD_NORMALMAP
{
	float4 pos : SV_POSITION;
	float3 posW : POSITION;
	//float4 shadowPos : SHADOW;
	float3 normalW : NORMAL;
	float3 tangentW : TANGENT;
	float2 tex : TEXCOORD;
};

struct HCS_EDGE3_IN1
{
	float fTessEdges[3]   : SV_TessFactor;
	float fTessInsides[1] : SV_InsideTessFactor;
};
