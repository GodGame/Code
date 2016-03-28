#include "fx/Light.fx"
#include "fx/Common.fx"
// 빌보드용 구조체이다--------------------------------------------------------------------------------------------------
struct VS_BILLBOARD_INPUT
{
	float3 pos                         : POSITION;
	float2 sizeW                       : SIZE;
	float4 instanceW                   : INSTANCE;
};

struct VS_BILLBOARD_OUTPUT
{
	float4 centerW                     : POSITION;
	float2 sizeW                       : SIZE;
};

struct GS_BILLBOARD_ARRAY_OUTPUT
{
	float4 posH                        : SV_POSITION;
	float3 posW                        : POSITION;
	float3 normalW                     : NORMAL;
	float2 texCoord                    : TEXCOORD;
	uint primID                        : SV_PrimitiveID;
};

struct GS_BILLBOARD_COLOR_OUTPUT
{
	float4 posH                        : SV_POSITION;
	float3 posW                        : POSITION;
	float4 color                       : COLOR;
	float2 texCoord                    : TEXCOORD;
};

struct VS_BILLBOARD_CUBE_INPUT
{
	float3 pos                         : POSITION;
	float sizeW						   : SIZE;
};

struct VS_BILLBOARD_CUBE_OUTPUT
{
	float3 centerW                     : POSITION;
	float sizeW : SIZE;
};

struct GS_BILLBOARD_CUBE_OUTPUT
{
	float4 posH                        : SV_POSITION;
	float3 posW                        : POSITION;
	float3 normalW                     : NORMAL;
	float2 texCoord                    : TEXCOORD;
	uint primID                        : SV_PrimitiveID;
};

struct VS_INSTANCE_CUBE_INPUT
{
	float sizeW : SIZE;
	//float4x4 mtxW                    : INSTANCEPOS;
	float4 posW                        : INSTANCEPOS;
};

struct VS_INSTANCE_CUBE_OUTPUT
{
	float sizeW						   : SIZE;
	float3 centerW                     : POSITION;
	//float3 pos                       : POSITION
};

struct GS_INSTANCE_OUTPUT
{
	float4 posH                        : SV_POSITION;
	float3 posW                        : POSITION;
	float3 normalW                     : NORMAL;
	float2 texCoord                    : TEXCOORD;
	//uint primID                      : SV_PrimitiveID;
};

struct VS_INSTANCE_SPHERE_INPUT
{
	float4 info	                       : INFO;
	float4 posW                        : INSTANCEPOS;
};

struct VS_INSTANCE_SPHERE_OUTPUT
{
	float4 info                        : INFO;
	float3 centerW                     : POSITION;
};
////////////////////////////////////////////////////////////////////////////////////////////////////


// 빌보드용--------------------------------------------------------------------------------------------------------------
VS_BILLBOARD_OUTPUT VSBillboard(VS_BILLBOARD_INPUT input, uint instID : SV_InstanceID)
{
	VS_BILLBOARD_OUTPUT output;
	//output.centerW.xyz = input.posW.xyz;// +input.pos;
	output.centerW = input.instanceW;
	//output.centerW.w = instID;
	output.sizeW = input.sizeW;
	return output;
}

[maxvertexcount(4)]
void GSBillboard(point VS_BILLBOARD_OUTPUT input[1],
	//uint primID : SV_InstanceID,
	inout TriangleStream<GS_BILLBOARD_ARRAY_OUTPUT> triStream)
{
	float3 vUp    = float3(0.0f, 1.0f, 0.0f);
	float3 vLook  = gf3CameraPos.xyz - input[0].centerW;
	vLook.y       = 0.0f;
	vLook         = normalize(vLook);
	float3 vRight = cross(vUp, vLook);

	float fHalfW = 0.5f * input[0].sizeW.x;
	float fHalfH = 0.5f * input[0].sizeW.y;

	float3 vWidth = fHalfW * vRight;
	float3 vHeight = fHalfH * vUp;
	float3 PosW = input[0].centerW.xyz;

	float4 pVertices[4];
	pVertices[0] = float4(PosW + vWidth - vHeight, 1.0f);
	pVertices[1] = float4(PosW + vWidth + vHeight, 1.0f);
	pVertices[2] = float4(PosW - vWidth - vHeight, 1.0f);
	pVertices[3] = float4(PosW - vWidth + vHeight, 1.0f);

	GS_BILLBOARD_ARRAY_OUTPUT output;
	for (int i = 0; i < 4; ++i)
	{
		output.posW     = pVertices[i].xyz;
		output.posH     = mul(pVertices[i], gmtxViewProjection);
		output.normalW  = vLook;
		output.texCoord = gvQuadTexCoord[i];
		output.primID   = input[0].centerW.w;
		triStream.Append(output);
	}
}

[maxvertexcount(4)]
void GSBillboardColor(point VS_BILLBOARD_OUTPUT input[1],
	//uint primID : SV_InstanceID,
	inout TriangleStream<GS_BILLBOARD_COLOR_OUTPUT> triStream)
{
	float3 vUp    = float3(0.0f, 1.0f, 0.0f);
	float3 vLook  = gf3CameraPos.xyz - input[0].centerW;
	vLook.y       = 0.0f;
	vLook         = normalize(vLook);
	float3 vRight = cross(vUp, vLook);

	float fHalfW   = 0.5f * input[0].sizeW.x;
	float fHalfH   = 0.5f * input[0].sizeW.y;

	float3 vWidth  = fHalfW * vRight;
	float3 vHeight = fHalfH * vUp;
	float3 PosW    = input[0].centerW.xyz;

	float4 pVertices[4];
	pVertices[0] = float4(PosW + vWidth - vHeight, 1.0f);
	pVertices[1] = float4(PosW + vWidth + vHeight, 1.0f);
	pVertices[2] = float4(PosW - vWidth - vHeight, 1.0f);
	pVertices[3] = float4(PosW - vWidth + vHeight, 1.0f);

	PosW = gf3CameraPos.xyz - PosW;

	GS_BILLBOARD_COLOR_OUTPUT output;
	for (int i = 0; i < 4; ++i)
	{
		output.posH     = mul(pVertices[i], gmtxViewProjection);
		output.posW     = PosW;
		output.color    = gcColors[uint(input[0].centerW.w)];
		output.texCoord = gvQuadTexCoord[i];
		triStream.Append(output);
	}
}

PS_MRT_OUT PSBillboardTextureArray(GS_BILLBOARD_ARRAY_OUTPUT input) : SV_Target
{
	float3 uvw = float3(input.texCoord, (input.primID % 4));
	float4 cColor = gTextureArray.Sample(gSamplerState, uvw);
	if (cColor.a < 0.1) discard;

	PS_MRT_OUT output;

	output.vNormal  = float4(input.normalW, 0.0f/*input.posW.w * gfDepthFar*/);
	output.vPos     = float4(input.posW, 1.0f);
	output.vDiffuse = float4(gMaterial.m_cDiffuse.rgb, 1);
	output.vSpec    = float4(gMaterial.m_cSpecular.rgb, 0);
	output.vTxColor = cColor;// gtxtTexture.Sample(gSamplerState, input.texCoord);

	return output;
}

PS_MRT_OUT PSBillboardColor(GS_BILLBOARD_COLOR_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.texCoord);
	if (cColor.a < 0.1) discard;

	PS_MRT_OUT output;

	output.vNormal  = vZero;
	output.vPos     = float4(input.posW, 1.0f);
	output.vDiffuse = vZero;
	output.vSpec    = vZero;
	output.vTxColor = cColor * input.color * 10;

	return output;
}

PS_MRT_OUT PSBillboard(GS_BILLBOARD_ARRAY_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.texCoord);
	if (cColor.a <= 0.15) discard;

	PS_MRT_OUT output;

	output.vNormal  = float4(input.normalW, 0.0f);
	output.vPos     = float4(input.posW, 1.0f);
	output.vDiffuse = float4(gMaterial.m_cDiffuse.rgb, 1);
	output.vSpec    = float4(gMaterial.m_cSpecular.rgb, 0);
	output.vTxColor = cColor;

	return output;
}

VS_BILLBOARD_CUBE_OUTPUT VSCubeBillboard(VS_BILLBOARD_CUBE_INPUT input)
{
	VS_BILLBOARD_CUBE_OUTPUT output;
	//float3 test = float3(1006, 200, 308);
	//output.centerW.xyz = input.posW.xyz;// +input.pos;
	output.centerW = input.pos;
	output.sizeW = input.sizeW;
	return output;
}

VS_INSTANCE_CUBE_OUTPUT VSPointCubeInstance(VS_INSTANCE_CUBE_INPUT input)
{
	VS_INSTANCE_CUBE_OUTPUT output;
	//output.positionW = mul(float4(input.position, 1.0f), input.mtxTransform).xyz;
	//output.centerW   = mul(float4(input.pos, 1.0f), input.mtxW).xyz;
	output.centerW     = input.posW;
	output.sizeW       = input.sizeW;
	return output;
}

PS_MRT_OUT PSPointInstance(GS_INSTANCE_OUTPUT input) : SV_Target
{
	PS_MRT_OUT output;

	output.vNormal  = float4(input.normalW, 0.0f/*input.posW.w * gfDepthFar*/);
	output.vPos     = float4(input.posW, 1.0f);
	output.vDiffuse = float4(gMaterial.m_cDiffuse.rgb, 1);
	output.vSpec    = gMaterial.m_cSpecular;
	output.vTxColor = gtxtTexture.Sample(gSamplerState, input.texCoord);

return (output);
}

[maxvertexcount(36)]
void GSPointCubeInstance(point VS_INSTANCE_CUBE_OUTPUT input[1],
	uint primID : SV_PrimitiveID,
	inout TriangleStream<GS_INSTANCE_OUTPUT> triStream)
{
	float fSize = input[0].sizeW;
	float fx, fy, fz;
	fx = fy = fz = fSize;
	float3 Point = input[0].centerW.xyz;

	static float2 pTexCoords[4] = { float2(0.0f, 0.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f), float2(1.0f, 1.0f) };

	float4  f4Vertices[36];
	float3  f3Normal[36];
	float2  f2TexCoords[36];
	float3  pNormal;
	int index = 0;

	// 뒤쪽면
	pNormal = float3(-fx, +fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[0];
	pNormal = float3(+fx, +fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[1];
	pNormal = float3(+fx, -fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[3];

	pNormal = float3(-fx, +fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[0];
	pNormal = float3(+fx, -fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[3];
	pNormal = float3(-fx, -fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[2];

	// 윗면
	pNormal = float3(-fx, +fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[0];
	pNormal = float3(+fx, +fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[1];
	pNormal = float3(+fx, +fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[3];

	pNormal = float3(-fx, +fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[0];
	pNormal = float3(+fx, +fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[3];
	pNormal = float3(-fx, +fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[2];

	// 앞면
	pNormal = float3(-fx, -fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[0];
	pNormal = float3(+fx, -fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[1];
	pNormal = float3(+fx, +fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[3];

	pNormal = float3(-fx, -fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[0];
	pNormal = float3(+fx, +fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[3];
	pNormal = float3(-fx, +fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[2];

	// 아랫면
	pNormal = float3(-fx, -fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[0];
	pNormal = float3(+fx, -fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[1];
	pNormal = float3(+fx, -fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[3];

	pNormal = float3(-fx, -fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[0];
	pNormal = float3(+fx, -fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[3];
	pNormal = float3(-fx, -fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[2];

	// 왼쪽면
	pNormal = float3(-fx, +fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[0];
	pNormal = float3(-fx, +fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[1];
	pNormal = float3(-fx, -fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[3];

	pNormal = float3(-fx, +fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[0];
	pNormal = float3(-fx, -fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[3];
	pNormal = float3(-fx, -fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[2];

	// 우측면
	pNormal = float3(+fx, +fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[0];
	pNormal = float3(+fx, +fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[1];
	pNormal = float3(+fx, -fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[3];

	pNormal = float3(+fx, +fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[0];
	pNormal = float3(+fx, -fy, +fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[3];
	pNormal = float3(+fx, -fy, -fz); f4Vertices[index] = float4(pNormal + Point, 1.0f);
	f3Normal[index] = normalize(pNormal); f2TexCoords[index++] = pTexCoords[2];

	GS_INSTANCE_OUTPUT output;

	[unroll]
	for (int i = 1; i <= 36; ++i)
	{
		index = i - 1;
		output.posW = f4Vertices[index];
		output.posH = mul(f4Vertices[index], gmtxViewProjection);
		output.normalW = f3Normal[index];
		output.texCoord = f2TexCoords[index];
		//output.primID = primID;
		triStream.Append(output);

		if (i % 3 == 0)
			triStream.RestartStrip();
	}
}

VS_INSTANCE_SPHERE_OUTPUT VSPointSphereInstance(VS_INSTANCE_SPHERE_INPUT input)
{
	VS_INSTANCE_SPHERE_OUTPUT output;

	output.centerW = input.posW.xyz;
	output.info = input.info;
	return output;
}

[maxvertexcount(6)]
void GSPointSphereInstance(point VS_INSTANCE_SPHERE_OUTPUT input[1],
	uint primID : SV_PrimitiveID,
	inout TriangleStream<GS_INSTANCE_OUTPUT> triStream)
{
	int i = input[0].info.x;
	int j = input[0].info.y;
	float fRadius = input[0].info.w;
	int nStacks, nSlices;
	nStacks = nSlices = input[0].info.z;

	float3 Point = input[0].centerW.xyz;

	float4  f4Vertices[3];
	float3  f3Normal[3];
	float2  f2TexCoords[3];
	float3  pNormal;

	float rStacks = float(1.0 / float(nStacks));
	float rSlices = float(1.0 / float(nSlices));

	GS_INSTANCE_OUTPUT output;

	float theta_i, theta_ii, phi_j, phi_jj, fRadius_j, fRadius_jj, y_j, y_jj;

	phi_j = float(3.14 / float(nStacks)) * j;
	phi_jj = float(3.14 / float(nStacks)) * (j + 1);
	fRadius_j = fRadius * sin(phi_j);
	fRadius_jj = fRadius * sin(phi_jj);
	y_j = fRadius * cos(phi_j);
	y_jj = fRadius * cos(phi_jj);

	theta_i = float(6.283184 / float(nSlices)) * float(i);	//2*PI
	theta_ii = float(6.283184 / float(nSlices)) * float(i + 1);

	pNormal = float3(fRadius_j*cos(theta_i), y_j, -fRadius_j*sin(theta_i));
	f4Vertices[0] = float4(pNormal + Point, 1);
	f2TexCoords[0] = float2(float(i) * rSlices, float(j) * rStacks);
	f3Normal[0] = normalize(pNormal);
	pNormal = float3(fRadius_jj*cos(theta_i), y_jj, -fRadius_jj*sin(theta_i));
	f4Vertices[1] = float4(pNormal + Point, 1);
	f2TexCoords[1] = float2(float(i) * rSlices, float(j + 1) * rStacks);
	f3Normal[1] = normalize(pNormal);
	pNormal = float3(fRadius_j*cos(theta_ii), y_j, -fRadius_j*sin(theta_ii));
	f4Vertices[2] = float4(pNormal + Point, 1);
	f2TexCoords[2] = float2(float(i + 1) * rSlices, float(j) * rStacks);
	f3Normal[2] = normalize(pNormal);

	[unroll]
	for (int k = 0; k < 3; ++k)
	{
		output.posW = f4Vertices[k];
		output.posH = mul(f4Vertices[k], gmtxViewProjection);
		output.normalW = f3Normal[k];
		output.texCoord = f2TexCoords[k];
		triStream.Append(output);
	}
	triStream.RestartStrip();

	pNormal = float3(fRadius_jj*cos(theta_i), y_jj, -fRadius_jj*sin(theta_i));
	f4Vertices[0] = float4(pNormal + Point, 1);
	f2TexCoords[0] = float2(float(i) * rSlices, float(j + 1) * rStacks);
	f3Normal[0] = normalize(pNormal);
	pNormal = float3(fRadius_jj*cos(theta_ii), y_jj, -fRadius_jj*sin(theta_ii));
	f4Vertices[1] = float4(pNormal + Point, 1);
	f2TexCoords[1] = float2(float(i + 1) * rSlices, float(j + 1) * rStacks);
	f3Normal[1] = normalize(pNormal);
	pNormal = float3(fRadius_j*cos(theta_ii), y_j, -fRadius_j*sin(theta_ii));
	f4Vertices[2] = float4(pNormal + Point, 1);
	f2TexCoords[2] = float2(float(i + 1) * rSlices, float(j) * rStacks);
	f3Normal[2] = normalize(pNormal);

	[unroll]
	for (int k = 0; k < 3; ++k)
	{
		output.posW = f4Vertices[k];
		output.posH = mul(f4Vertices[k], gmtxViewProjection);
		output.normalW = f3Normal[k];
		output.texCoord = f2TexCoords[k];
		triStream.Append(output);
	}
	triStream.RestartStrip();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////