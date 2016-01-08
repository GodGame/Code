#include "Define.fx"
#include "Shadow.fx"

PS_SCENE_INPUT VSScreen(VS_SCENE_INPUT input)
{
	PS_SCENE_INPUT output;
	output.pos = float4(input.pos, 1);
	output.tex = input.tex;

	return output;
}

float4 InfoScreen(PS_SCENE_INPUT input) : SV_Target
{
	int3 uvm = int3(input.pos.xy, 0);
	float4 color = gtxtTexture.Load(uvm);
	if (color.x > 100.0f)
	{
		color.xyz /= 2000.0f;
	}
	return color;
}

float4 LightScreen(PS_SCENE_INPUT input) : SV_Target
{
	int3 uvm = int3(input.pos.xy, 0);
	float color = gtxtTexture.Load(uvm);
	return float4(color.xxx, 0.0);
}


float4 PSScreen(PS_SCENE_INPUT input) : SV_Target
{
	int3 uvm = int3(input.pos.xy, 0);	// (u, v, level)
	float4 normal = gtxtNormal.Load(uvm);
	float3 pos = gtxtPos.Load(uvm).xyz;
	float4 diffuse = gtxtDiffuse.Load(uvm);
	float4 specular = gtxtSpecular.Load(uvm);
	float4 txColor = GammaToneMapping(gtxtTxColor.Load(uvm));

	float4 color;
	if (diffuse.a == 0.0)
	{
		return lerp(txColor, gFogColor, 0.9f);
	}
	else
	{
		//matrix shadowProj = mul(gmtxWorld, gmtxShadowTransform);
		float4 shadowPos = mul(float4(pos, 1.0f), gmtxShadowTransform);

		float fShadowFactor = 0.3f;
#ifdef SHADOW_PCF
		fShadowFactor = CalcShadowFactorByPCF(shadowPos);
#else
		fShadowFactor = CalcOneShadowFactor(shadowPos, fShadowFactor);
#endif
		color = Lighting(pos, normal, float4(diffuse.rgb, fShadowFactor), specular);
		color *= txColor;
	}
	float distance = length(pos - gvCameraPosition.xyz);

	float fogLerp = saturate((distance - gFogStart) / gFogRange);
	// Blend the fog color and the lit color.
	return lerp(color, gFogColor, fogLerp);
}

/*정점-쉐이더이다. 정점의 위치 벡터를 월드 변환, 카메라 변환, 투영 변환을 순서대로 수행한다. 이제 삼각형의 각 정점은 y-축으로의 회전을 나타내는 행렬에 따라 변환한다. 그러므로 삼각형은 회전하게 된다.*/
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.position = mul(float4(input.position, 1), gmtxWorld);
	output.position = mul(output.position, gmtxViewProjection);
	//output.color = input.color;
	return output;
}

VS_INSTANCED_COLOR_OUTPUT VSInstancedDiffusedColor(VS_INSTANCED_COLOR_INPUT input, uint instanceID : SV_InstanceID)
{
	VS_INSTANCED_COLOR_OUTPUT output = (VS_INSTANCED_COLOR_OUTPUT)0;
	output.position = mul(float4(input.position, 1), input.mtxTransform);
	output.position = mul(output.position, gmtxViewProjection);
	output.color = (instanceID % 5) ? input.color : (input.color * 0.3f + input.instanceColor * 0.7f);
	output.instanceID = instanceID;
	return output;
}


float4 PS(VS_OUTPUT input) : SV_Target
{
	//return input.color;
	return (1.0f, 1.0f, 1.0f, 0.0f);
	//return gcColor;    // Yellow, with Alpha = 1
}

float4 PSInstancedDiffusedColor(VS_INSTANCED_COLOR_OUTPUT input) : SV_Target
{
	return input.color;
}

VS_DIFFUSED_COLOR_OUTPUT VSDiffusedColor(VS_DIFFUSED_COLOR_INPUT input)
{
	VS_DIFFUSED_COLOR_OUTPUT output = (VS_DIFFUSED_COLOR_OUTPUT)0;
	output.position = mul(float4(input.position, 1.0f), mul(gmtxWorld, gmtxViewProjection));
	output.color = input.color;

	return(output);
}

float4 PSDiffusedColor(VS_DIFFUSED_COLOR_OUTPUT input) : SV_Target
{
	return(input.color);
}


VS_INSTANCED_DIFFUSED_COLOR_OUTPUT VSInstancedDiffusedColor(VS_INSTANCED_DIFFUSED_COLOR_INPUT input)
{
	VS_INSTANCED_DIFFUSED_COLOR_OUTPUT output = (VS_INSTANCED_DIFFUSED_COLOR_OUTPUT)0;
	output.position = mul(mul(float4(input.position, 1.0f), input.mtxTransform), gmtxViewProjection);
	output.color = input.color;

	return(output);
}

float4 PSInstancedDiffusedColor(VS_INSTANCED_DIFFUSED_COLOR_OUTPUT input) : SV_Target
{
	return(input.color);
}

//조명의 영향을 계산하기 위한 정점의 법선 벡터와 정점의 위치를 계산하는 정점 쉐이더 함수이다.
VS_LIGHTING_COLOR_OUTPUT VSLightingColor(VS_LIGHTING_COLOR_INPUT input)
{
	VS_LIGHTING_COLOR_OUTPUT output = (VS_LIGHTING_COLOR_OUTPUT)0;
	//조명의 영향을 계산하기 위하여 월드좌표계에서 정점의 위치와 법선 벡터를 구한다.
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
	output.position = mul(float4(output.positionW, 1.0f), gmtxViewProjection);

	return(output);
}
// 각 픽셀에 대하여 조명의 영향을 반영한 색상을 계산하기 위한 픽셀 쉐이더 함수이다.
float4 PSLightingColor(VS_LIGHTING_COLOR_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);

	return(cIllumination);
}

VS_INSTANCED_LIGHTING_COLOR_OUTPUT VSInstancedLightingColor(VS_INSTANCED_LIGHTING_COLOR_INPUT input)
{
	VS_INSTANCED_LIGHTING_COLOR_OUTPUT output = (VS_INSTANCED_LIGHTING_COLOR_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)input.mtxTransform);
	output.positionW = mul(float4(input.position, 1.0f), input.mtxTransform).xyz;
	output.position = mul(float4(output.positionW, 1.0f), gmtxViewProjection);

	return(output);
}

float4 PSInstancedLightingColor(VS_INSTANCED_LIGHTING_COLOR_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);

	return(cIllumination);
}

VS_TEXTURED_COLOR_OUTPUT VSTexturedColor(VS_TEXTURED_COLOR_INPUT input)
{
	VS_TEXTURED_COLOR_OUTPUT output = (VS_TEXTURED_COLOR_OUTPUT)0;
	output.position = mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxViewProjection);
	output.texCoord = input.texCoord;
	output.posW = mul(mul(input.position, (float3x3)gmtxWorld), gmtxViewProjection);
//	float4x4 shadowProj = mul(gmtxWorld, gmtxShadowTransform);
//	output.shadowPos = mul(float4(output.posW, 1.0f), shadowProj);
	return(output);
}

PS_MRT_OUT PSTexturedColor(VS_TEXTURED_COLOR_OUTPUT input)
{
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.texCoord);
//	input.shadowPos.xyz /= input.shadowPos.w;
//	float fsDepth = gtxtShadowMap.Sample(gssShadowMap, input.shadowPos.xy).r;
//	float fShadowFactor = 0.1f;
//	if (input.shadowPos.z <= (fsDepth + gfBias))
//		fShadowFactor = 1.0f;

	PS_MRT_OUT output;
	output.vNormal = float4(1, 1, 1, input.position.w * gfDepthFar);
	output.vPos = float4(input.posW, 1.0f);
	output.vDiffuse = float4(gMaterial.m_cDiffuse.xyz, 0.0f/*fShadowFactor*/);
	output.vSpec = gMaterial.m_cSpecular;
	output.vTxColor = cColor;

	return(output);
}

// 스카이박스 큐브맵 전용 -----------------------------

VS_SKYBOX_CUBEMAP_OUTPUT VSSkyBoxTexturedColor(VS_SKYBOX_CUBEMAP_INPUT input)
{
	VS_SKYBOX_CUBEMAP_OUTPUT output = (VS_SKYBOX_CUBEMAP_OUTPUT)0;
	output.position = mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxViewProjection);
	output.positionL = input.position;

	return(output);
}

PS_MRT_OUT PSSkyBoxTexturedColor(VS_SKYBOX_CUBEMAP_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtSkyBox.Sample(gssSkyBox, input.positionL);
	//float4 cColor = gtxtTexture.Sample(gSamplerState, input.positionL);

	PS_MRT_OUT output;
	output.vNormal = float4(0, 0, 0, 0);
	output.vPos = float4(input.positionL, 1);
	output.vDiffuse = float4(1, 1, 1, 0);// cColor;
	output.vSpec = gMaterial.m_cSpecular;
	output.vTxColor = cColor;
	return(output);
}



VS_INSTANCED_TEXTURED_COLOR_OUTPUT VSInstancedTexturedColor(VS_INSTANCED_TEXTURED_COLOR_INPUT input)
{
	VS_INSTANCED_TEXTURED_COLOR_OUTPUT output = (VS_INSTANCED_TEXTURED_COLOR_OUTPUT)0;
	output.position = mul(mul(float4(input.position, 1.0f), input.mtxTransform), gmtxViewProjection);
	output.texCoord = input.texCoord;

	return(output);
}

float4 PSInstancedTexturedColor(VS_INSTANCED_TEXTURED_COLOR_OUTPUT input) : SV_Target
{
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.texCoord);

	return(cColor);
}

//---------------------------------------------------------------------------------------------------------------------

VS_DETAIL_TEXTURED_COLOR_OUTPUT  VSDetailTexturedColor(VS_DETAIL_TEXTURED_COLOR_INPUT input)
{
	VS_DETAIL_TEXTURED_COLOR_OUTPUT output = (VS_DETAIL_TEXTURED_COLOR_OUTPUT)0;
	output.position = mul(mul(float4(input.position, 1.0f), gmtxWorld), gmtxViewProjection);
	output.texCoordBase = input.texCoordBase;
	output.texCoordDetail = input.texCoordDetail;

	return(output);
}

float4 PSDetailTexturedColor(VS_DETAIL_TEXTURED_COLOR_OUTPUT input) : SV_Target
{
	float4 cBaseTexColor = gtxtTexture.Sample(gSamplerState, input.texCoordBase);
	float4 cDetailTexColor = gtxtDetailTexture.Sample(gDetailSamplerState, input.texCoordDetail);
	float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

	return(cColor);
}

//---------------------------------------------------------------------------------------------------------------------


VS_TEXTURED_LIGHTING_COLOR_OUTPUT VSTexturedLightingColor(VS_TEXTURED_LIGHTING_COLOR_INPUT input)
{
	VS_TEXTURED_LIGHTING_COLOR_OUTPUT output = (VS_TEXTURED_LIGHTING_COLOR_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
	output.position = mul(float4(output.positionW, 1.0f), gmtxViewProjection);
	output.texCoord = input.texCoord;

	//matrix shadowProj = mul(gmtxWorld, gmtxShadowTransform);
	//output.shadowPos = mul(float4(input.position, 1.0f), shadowProj);

	return(output);
}

PS_MRT_OUT PSTexturedLightingColor(VS_TEXTURED_LIGHTING_COLOR_OUTPUT input)
{
	input.normalW = normalize(input.normalW);
	//float4 cIllumination = Lighting(input.positionW, input.normalW);
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.texCoord);// *cIllumination;

	//float fShadowFactor = 0.3f;
	//fShadowFactor = CalcOneShadowFactor(gsShadow, gtxtShadowMap, input.shadowPos, fShadowFactor);

	PS_MRT_OUT output;
	output.vNormal = float4(input.normalW, input.position.w * gfDepthFar);
	output.vPos = float4(input.positionW, 1.0f);
	output.vDiffuse = float4(gMaterial.m_cDiffuse.rgb, 1);
	output.vSpec = gMaterial.m_cSpecular;
	output.vTxColor = cColor;

	return(output);
}



//---------------------------------------------------------------------------------------------------------------------
VS_DETAIL_TEXTURED_LIGHTING_COLOR_OUTPUT VSDetailTexturedLightingColor(VS_DETAIL_TEXTURED_LIGHTING_COLOR_INPUT input)
{
	VS_DETAIL_TEXTURED_LIGHTING_COLOR_OUTPUT output = (VS_DETAIL_TEXTURED_LIGHTING_COLOR_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
	output.position = mul(float4(output.positionW, 1.0f), gmtxViewProjection);
	output.texCoordBase = input.texCoordBase;
	output.texCoordDetail = input.texCoordDetail;

	return(output);
}

float4 PSDetailTexturedLightingColor(VS_DETAIL_TEXTURED_LIGHTING_COLOR_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);
	float4 cBaseTexColor = gtxtTexture.Sample(gSamplerState, input.texCoordBase);
	float4 cDetailTexColor = gtxtDetailTexture.Sample(gDetailSamplerState, input.texCoordDetail);
	float4 cColor = saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

	return(cColor*cIllumination);
}

// 스플랫용 
VS_SPLAT_TEXTURED_LIGHTING_COLOR_OUTPUT VSSplatTexturedLightingColor(VS_SPLAT_TEXTURED_LIGHTING_COLOR_INPUT input)
{
	
	VS_SPLAT_TEXTURED_LIGHTING_COLOR_OUTPUT output = (VS_SPLAT_TEXTURED_LIGHTING_COLOR_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.positionW = mul(float4(input.position, 1.0f), gmtxWorld).xyz;
	output.position = mul(float4(output.positionW, 1.0f), gmtxViewProjection);
	output.texCoordBase  = input.texCoordBase;
	output.texCoordAlpha = input.texCoordAlpha;

	//matrix shadowProj = mul(gmtxWorld, gmtxShadowTransform);
	//output.shadowPos = mul(float4(input.position, 1.0f), shadowProj);

	return(output);
}

PS_MRT_OUT PSSplatTexturedLightingColor(VS_SPLAT_TEXTURED_LIGHTING_COLOR_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);
	//float4 cIllumination = Lighting(input.positionW, input.normalW);
	float4 cAlphaColor = gtxtDetailTexture.Sample(gDetailSamplerState, input.texCoordAlpha);
	if (cAlphaColor.a != 0.0f)
		discard;
	
	float4 cTexColor = gtxtTexture.Sample(gSamplerState, input.texCoordBase);
	//float4 cEntityColor = gtxtTexture.Sample(gDetailSamplerState, input.texCoordAlpha);

	//float fShadowFactor = 0.3f;
	//fShadowFactor =	CalcOneShadowFactor(gsShadow, gtxtShadowMap, input.shadowPos, fShadowFactor);
	
	PS_MRT_OUT output;

	output.vNormal = float4(input.normalW, input.position.w * gfDepthFar);
	output.vPos = float4(input.positionW, 1.0f);
	output.vDiffuse = float4(gMaterial.m_cDiffuse.rgb, 1);
	output.vSpec = gMaterial.m_cSpecular;
	output.vTxColor = cTexColor;// *cEntityColor;

	return output;
}

//---------------------------------------------------------------------------------------------------------------------
VS_INSTANCED_TEXTURED_LIGHTING_COLOR_OUTPUT VSInstancedTexturedLightingColor(VS_INSTANCED_TEXTURED_LIGHTING_COLOR_INPUT input)
{
	VS_INSTANCED_TEXTURED_LIGHTING_COLOR_OUTPUT output = (VS_INSTANCED_TEXTURED_LIGHTING_COLOR_OUTPUT)0;
	output.normalW = mul(input.normal, (float3x3)input.mtxTransform);
	output.positionW = mul(float4(input.position, 1.0f), input.mtxTransform).xyz;
	output.position = mul(float4(output.positionW, 1.0f), gmtxViewProjection);
	output.texCoord = input.texCoord;

	return(output);
}

float4 PSInstancedTexturedLightingColor(VS_INSTANCED_TEXTURED_LIGHTING_COLOR_OUTPUT input) : SV_Target
{
	input.normalW = normalize(input.normalW);
	float4 cIllumination = Lighting(input.positionW, input.normalW);
	float4 cColor = gtxtTexture.Sample(gSamplerState, input.texCoord) * cIllumination;

	return(cColor);
}


// 빌보드용--------------------------------------------------------------------------------------------------------------
VS_BILLBOARD_OUTPUT VSBillboard(VS_BILLBOARD_INPUT input)
{
	VS_BILLBOARD_OUTPUT output;
	float3 test = float3(1006, 200, 308);
	//output.centerW.xyz = input.posW.xyz;// +input.pos;
	output.centerW.x = input.posW.x;
	output.centerW.y = input.posW.y;
	output.centerW.z = input.posW.z;
	output.sizeW = input.sizeW;
	return output;
}

[maxvertexcount(4)]
void GSBillboard(point VS_BILLBOARD_OUTPUT input[1], 
	uint primID : SV_PrimitiveID, 
	inout TriangleStream<GS_BILLBOARD_OUTPUT> triStream)
{
	float3 vUp = float3(0.0f, 1.0f, 0.0f);
	float3 vLook = gf3CameraPos.xyz - input[0].centerW;
	vLook.y = 0.0f;
	vLook = normalize(vLook);
	float3 vRight = cross(vUp, vLook);
	
	float fHalfW = 0.5f * input[0].sizeW.x;
	float fHalfH = 0.5f * input[0].sizeW.y;

	float3 vWidth = fHalfW * vRight;
	float3 vHeight = fHalfH * vUp;
	
	float4 pVertices[4];
	pVertices[0] = float4(input[0].centerW + fHalfW * vRight - fHalfH * vUp, 1.0f);
	pVertices[1] = float4(input[0].centerW + fHalfW * vRight + fHalfH * vUp, 1.0f);
	pVertices[2] = float4(input[0].centerW - fHalfW * vRight - fHalfH * vUp, 1.0f);
	pVertices[3] = float4(input[0].centerW - fHalfW * vRight + fHalfH * vUp, 1.0f);

	float2 pTexCoords[4] = { float2(0.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 1.0f), float2(1.0f, 0.0f) };

	GS_BILLBOARD_OUTPUT output;
	for (int i = 0; i < 4; ++i) 
	{
		output.posW = pVertices[i].xyz;
		output.posH = mul(pVertices[i], gmtxViewProjection);
		output.normalW = vLook;
		output.texCoord = pTexCoords[i];
		output.primID = primID;
		triStream.Append(output);
	}
}

float4 PSBillboard(GS_BILLBOARD_OUTPUT input) : SV_Target
{
	float4 cIllumination = Lighting(input.posW, input.normalW);
	float3 uvw = float3(input.texCoord, (input.primID % 4));
	//float4 cTexture = gTextureArray.Sample(gSamplerState, uvw);
	//float4 cColor = cIllumination * cTexture;
	//cColor.a = cTexture.a;
	float4 cColor = float4(1.0f, 1.0f, 1.0f, 0.0f);
	return (cColor);
}



VS_BILLBOARD_CUBE_OUTPUT VSCubeBillboard(VS_BILLBOARD_CUBE_INPUT input)
{
	VS_BILLBOARD_CUBE_OUTPUT output;
	//float3 test = float3(1006, 200, 308);
		//output.centerW.xyz = input.posW.xyz;// +input.pos;
	output.centerW = input.pos;
	//output.centerW.x = input.posW.x;
	//output.centerW.y = input.posW.y;
	//output.centerW.z = input.posW.z;
	output.sizeW = input.sizeW;
	return output;
}

VS_INSTANCE_CUBE_OUTPUT VSPointCubeInstance(VS_INSTANCE_CUBE_INPUT input)
{
	VS_INSTANCE_CUBE_OUTPUT output;
	//output.positionW = mul(float4(input.position, 1.0f), input.mtxTransform).xyz;
	//output.centerW = mul(float4(input.pos, 1.0f), input.mtxW).xyz;
	output.centerW = input.posW;
	output.sizeW = input.sizeW;
	return output;
}

float4 PSPointInstance(GS_INSTANCE_OUTPUT input) : SV_Target
{
	float4 cIllumination = Lighting(input.posW, input.normalW);
	//float3 uvw = float3(input.texCoord, (input.primID % 4));
	//float4 cTexture = gTextureArray.Sample(gSamplerState, uvw);
	float4 cTexture = gtxtTexture.Sample(gSamplerState, input.texCoord);
	float4 cColor = cIllumination * cTexture;
	cColor.a = cTexture.a;
	//float4 cColor = float4(1.0f, 1.0f, 1.0f, 0.0f);
	return (cColor);
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

		float2 pTexCoords[4] = { float2(0.0f, 0.0f), float2(1.0f, 0.0f), float2(0.0f, 1.0f), float2(1.0f, 1.0f) };

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

FLOAT3_POS_FLOAT2_TEX VSBezier(FLOAT3_POS_FLOAT2_TEX input)
{
	FLOAT3_POS_FLOAT2_TEX output;
	output.pos = input.pos;
	output.tex = input.tex;
	return output;
}

HCS_EDGE4_IN2 HSBezierCS(InputPatch<FLOAT3_POS_FLOAT2_TEX, 16> input, uint nPatchID : SV_PrimitiveID)
{
	HCS_EDGE4_IN2 output;
	output.fTessEdges[0] = 15;
	output.fTessEdges[1] = 15;
	output.fTessEdges[2] = 15;
	output.fTessEdges[3] = 15;

	output.fTessInsides[0] = 15;
	output.fTessInsides[1] = 15;
	return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("HSBezierCS")]
[maxtessfactor(64.0f)]
FLOAT3_POS_FLOAT2_TEX HSBezier(InputPatch<FLOAT3_POS_FLOAT2_TEX, 16> input, uint i : SV_OutputControlPointID, uint nID : SV_PrimitiveID)
{
	FLOAT3_POS_FLOAT2_TEX output;
	output.pos = input[i].pos;	
	output.tex = input[i].tex;
	return output;
}

// uv : 15 * 15 호출 (u, v)수만큼 
// 셰이더는 테셀레이터가 생성한 각 정점마다 호출
[domain("quad")]
DS_BEZIER_OUTPUT DSBezier(HCS_EDGE4_IN2 input, float2 uv : SV_DomainLocation, OutputPatch<FLOAT3_POS_FLOAT2_TEX, 16> patch)
{
	DS_BEZIER_OUTPUT output;

	float4 uB = BernsteinCoefficient(uv.x);
	float4 vB = BernsteinCoefficient(uv.y);
	float3 position = CubicBezierSum(patch, uB, vB);

	//matrix mtxWorldViewProjection = mul(gmtxWorld, gmtxViewProjection);
	output.pos = mul(mul(float4(position, 1.0f), gmtxWorld), gmtxViewProjection);

	return output;
}


float4 PSBezier(DS_BEZIER_OUTPUT input) : SV_Target
{
	//return input.color;
	return (1.0f, 1.0f, 1.0f, 1.0f);
	//return gcColor;    // Yellow, with Alpha = 1
}

////////////////////////////////////////////////////////////////////////////////////////
FLOAT3_POS_FLOAT2_TEX VSTerrain(FLOAT3_POS_FLOAT2_TEX input)
{
	FLOAT3_POS_FLOAT2_TEX output;
	output.pos = input.pos;
	// 레벨을 자동 선택 안한다 마지막 파라미터 0
	//output.pos.y = gtxtTexture.SampleLevel(gSamplerState, input.tex, 0).r;
	output.tex = input.tex;
	return output;
}

HCS_EDGE4_IN2 CameraHCS(InputPatch<FLOAT3_POS_FLOAT2_TEX, 4> input, uint nPatchID : SV_PrimitiveID)
{
	HCS_EDGE4_IN2 output;

	float3 edges[4];
	edges[0] = 0.5f * (input[0].pos + input[2].pos);
	edges[1] = 0.5f * (input[0].pos + input[1].pos);
	edges[2] = 0.5f * (input[1].pos + input[3].pos);
	edges[3] = 0.5f * (input[2].pos + input[3].pos);
	float3 center = 0.25 * (input[0].pos + input[1].pos + input[2].pos + input[3].pos);

	output.fTessEdges[0] = CalculateTessFactor(edges[0]);
	output.fTessEdges[1] = CalculateTessFactor(edges[1]);
	output.fTessEdges[2] = CalculateTessFactor(edges[2]);
	output.fTessEdges[3] = CalculateTessFactor(edges[3]);

	output.fTessInsides[0] = CalculateTessFactor(center);
	output.fTessInsides[1] = output.fTessInsides[0];
	
	return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CameraHCS")]
[maxtessfactor(64.0f)]
FLOAT3_POS_FLOAT2_TEX HSTerrain(InputPatch<FLOAT3_POS_FLOAT2_TEX, 4> input,
	uint i : SV_OutputControlPointID)
{
	FLOAT3_POS_FLOAT2_TEX output;
	output.pos = input[i].pos;
	output.tex = input[i].tex;
	return output;
}


[domain("quad")]
DETAIL_TERRAIN DSTerrain(HCS_EDGE4_IN2 input, float2 uv : SV_DomainLocation,
	OutputPatch<FLOAT3_POS_FLOAT2_TEX, 4> quad)
{
	DETAIL_TERRAIN output;

	// uv 조절?
	// 애초에 터레인 좌표값을 월드 좌표 값으로 넣으므로 월드 변환은 필요가 없다.
	output.posW = lerp(lerp(quad[0].pos, quad[1].pos, uv.x), lerp(quad[2].pos, quad[3].pos, uv.x), uv.y);
	output.tex  = lerp(lerp(quad[0].tex, quad[1].tex, uv.x), lerp(quad[2].tex, quad[3].tex, uv.x), uv.y);
	output.texDetail = output.tex * gWorldCell;

	output.posW.y = gtxtTexture.SampleLevel(gSamplerState, output.tex, 0).r * gHegiht;
	output.posH = mul(float4(output.posW, 1.0f), gmtxViewProjection);
#ifdef DSNORMAL
	output.normalW =output.posW;
#endif
	return output;
}

PS_MRT_OUT PSTerrain(DETAIL_TERRAIN input)
{
#ifdef PSNORMAL
	static float TexelU = 1.0f;
	static float TexelV = 1.0f;

	static int left  = 0;
	static int right = 1;
	static int bot   = 2;
	static int top   = 3;

	float2 Tex[4];
	Tex[left]  = input.tex + float2(-TexelU, 0.0f);
	Tex[right] = input.tex + float2(TexelU, 0.0f);
	Tex[bot]   = input.tex + float2(0.0f, TexelV);
	Tex[top]   = input.tex + float2(0.0f, -TexelV);

	float  Height[4];
	Height[0] = gtxtTexture.SampleLevel(gSamplerState, Tex[0], 0).r;
	Height[1] = gtxtTexture.SampleLevel(gSamplerState, Tex[1], 0).r;
	Height[2] = gtxtTexture.SampleLevel(gSamplerState, Tex[2], 0).r;
	Height[3] = gtxtTexture.SampleLevel(gSamplerState, Tex[3], 0).r;

	float3 tangent = normalize(float3(2.0f * gWorldCell, Height[right] - Height[left], 0.0f));
	float3 bitangent = normalize(float3(0.0f, Height[bot] - Height[top], -2.0f * gWorldCell));
	float3 normalW = cross(tangent, bitangent);

#else
#ifdef DSNORMAL
	float3 normalW = normalize(input.normalW);
#endif
#endif
	//float4 cIllumination = Lighting(input.posW, normalW);
	float4 cTexture = gtxtDetailTexture.Sample(gDetailSamplerState, input.texDetail);
	float4 cEntire = gtxtDetailTexture.Sample(gSamplerState, input.tex);

	PS_MRT_OUT output;
	output.vNormal = float4(normalW, 1.0);
	output.vPos = float4(input.posW, 1.0);
	output.vDiffuse = float4(gMaterial.m_cDiffuse.xyz, 1);
	output.vSpec = gMaterial.m_cSpecular;
	output.vTxColor = (cTexture * cEntire);
	return (output);
}
////////////////////////////////////////////////////////////////////////////////////////
PS_WORLD_NORMALMAP VSNormalMap(MODEL_NORMALMAP input)
{
	PS_WORLD_NORMALMAP output;

	// 변위매핑
	//float offset = gtxtTexture.SampleLevel(gSamplerState, input.tex, 0).a;
	//input.pos += input.normal * 100.0f * (offset - 1.0f);

	output.pos = mul(float4(input.pos, 1.0f), gmtxWorld);
	output.posW = output.pos.xyz;
	output.tangentW = mul(input.tangent, (float3x3)gmtxWorld);
	output.normalW  = mul(input.normal,  (float3x3)gmtxWorld);
	output.pos = mul(output.pos, gmtxViewProjection);
	output.tex = input.tex;

	return output;
}

PS_MRT_OUT PSNormalMap(PS_WORLD_NORMALMAP input) 
{
	float3 N = normalize(input.normalW);
	float3 T = normalize(input.tangentW - dot(input.tangentW, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);

	float4 displacementInfo = gtxtTexture.SampleLevel(gSamplerState, input.tex, 0);
	float3 normal = (2.0f * displacementInfo.rgb) - 1.0f; //.rgb;
	normal = mul(normal, TBN);

	//float offset = displacementInfo.a;//gtxtTexture.SampleLevel(gSamplerState, input.tex, 0).a;
	input.posW -= normal * (gBumpScale.y * 2) * (1.0 - displacementInfo.a);
	
	float4 color = gtxtDetailTexture.Sample(gDetailSamplerState, input.tex);
	//	return (1, 1, 1, 0);

	//input.shadowPos.xyz /= input.shadowPos.w;
	//float fsDepth = gtxtShadowMap.Sample(gssShadowMap, input.shadowPos.xy).r;
	//float fShadowFactor = 0.3f;
	//if (input.shadowPos.z <= (fsDepth + gfBias))
	//	fShadowFactor = 1.0f;

	PS_MRT_OUT output;
	output.vNormal = float4(normal, 1.0);
	output.vPos = float4(input.posW, 1.0);
	output.vDiffuse = float4(gMaterial.m_cDiffuse.xyz, 1.0f/*fShadowFactor*/);
	output.vSpec = gMaterial.m_cSpecular;
	output.vTxColor = color;
	return output;
}

WORLD_NORMALMAP VSDisplacement(MODEL_NORMALMAP input)
{
	WORLD_NORMALMAP output;
	output.posW     = mul(input.pos,   gmtxWorld).xyz;
	output.tangentW = mul(input.tangent, (float3x3)gmtxWorld);
	output.normalW  = mul(input.normal,  (float3x3)gmtxWorld);
	output.tex      = input.tex;

	//////////////////////////////////////////////////////////////
	//float fDistToCamera = distance(output.posW, gf3CameraPos);
	//float fTessFactor = saturate((fDistToCamera - gCameraMin) / (gCameraMax - gCameraMin));
	//output.fTessFactor = 2 + fTessFactor * (20 - 2);

	return output;
}


HCS_EDGE3_IN1 TriCameraHCS(InputPatch<WORLD_NORMALMAP, 3> input, uint nPatchID : SV_PrimitiveID)
{
	HCS_EDGE3_IN1 output;

	float3 edges[3];
	edges[0] = 0.5f * (input[1].posW + input[2].posW);
	edges[1] = 0.5f * (input[2].posW + input[0].posW);
	edges[2] = 0.5f * (input[0].posW + input[1].posW);
	float3 center = 0.33 * (input[0].posW + input[1].posW + input[2].posW);

	output.fTessEdges[0] = CalculateTessFactor(edges[0]);
	output.fTessEdges[1] = CalculateTessFactor(edges[1]);
	output.fTessEdges[2] = CalculateTessFactor(edges[2]);
	
	output.fTessInsides[0] = CalculateTessFactor(center);

	//output.fTessEdges[0] = 0.5f * (input[1].fTessFactor + input[2].fTessFactor);
	//output.fTessEdges[1] = 0.5f * (input[2].fTessFactor + input[0].fTessFactor);
	//output.fTessEdges[2] = 0.5f * (input[0].fTessFactor + input[1].fTessFactor);
	//output.fTessInsides[0] = output.fTessEdges[0];

	return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("TriCameraHCS")]
[maxtessfactor(64.0f)]
WORLD_NORMALMAP HSDisplacement(InputPatch<WORLD_NORMALMAP, 3> input, uint i : SV_OutputControlPointID, uint nPID : SV_PrimitiveID)
{
	WORLD_NORMALMAP output;
	output.posW		= input[i].posW;
	output.normalW  = input[i].normalW;
	output.tangentW = input[i].tangentW;
	output.tex		= input[i].tex;

	return output;
}

[domain("tri")]
PS_WORLD_NORMALMAP DSDisplacement(HCS_EDGE3_IN1 input, float3 uv : SV_DomainLocation, OutputPatch<WORLD_NORMALMAP, 3> tri)
{
	PS_WORLD_NORMALMAP output;

	output.posW		= uv.x * tri[0].posW	 + uv.y * tri[1].posW		+ uv.z * tri[2].posW;
	output.normalW	= uv.x * tri[0].normalW  + uv.y * tri[1].normalW	+ uv.z * tri[2].normalW;
	output.tangentW = uv.x * tri[0].tangentW + uv.y * tri[1].tangentW	+ uv.z * tri[2].tangentW;
	output.tex		= uv.x * tri[0].tex		 + uv.y * tri[1].tex		+ uv.z * tri[2].tex;
	output.normalW  = normalize(output.normalW);

	//matrix shadowProj = mul(gmtxWorld, gmtxShadowTransform);
	//output.shadowPos = mul(float4(output.posW, 1.0f), shadowProj);

	float fHeight = gtxtTexture.SampleLevel(gSamplerState, output.tex,0 ).a;
	output.posW -= (gScaleHeight * (1.0f - fHeight )) * output.normalW;
	output.pos = mul(float4(output.posW, 1.0f), gmtxViewProjection);

	return output;
}



WORLD_NORMALMAP VSBump(MODEL_NORMALMAP input)
{
	WORLD_NORMALMAP output;
	output.posW = mul(float4(input.pos, 1), gmtxWorld).xyz;
	output.tangentW = mul(input.tangent, (float3x3)gmtxWorld);
	output.normalW = mul(input.normal, (float3x3)gmtxWorld);
	output.tex = input.tex;

	return output;
}


HCS_EDGE4_IN2 HCSBump(InputPatch<WORLD_NORMALMAP, 4> input, uint nPatchID : SV_PrimitiveID)
{
	HCS_EDGE4_IN2 output;

	float3 edges[4];
	edges[0] = 0.5f * (input[0].posW + input[2].posW);
	edges[1] = 0.5f * (input[0].posW + input[1].posW);
	edges[2] = 0.5f * (input[1].posW + input[3].posW);
	edges[3] = 0.5f * (input[2].posW + input[3].posW);
	float3 center = 0.25 * (input[0].posW + input[1].posW + input[2].posW + input[3].posW);


	output.fTessEdges[0] = CalculateTessFactor(edges[0]);
	output.fTessEdges[1] = CalculateTessFactor(edges[1]);
	output.fTessEdges[2] = CalculateTessFactor(edges[2]);
	output.fTessEdges[3] = CalculateTessFactor(edges[3]);

	output.fTessInsides[0] = CalculateTessFactor(center);
	output.fTessInsides[1] = output.fTessInsides[0];

	return output;
}


[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HCSBump")]
[maxtessfactor(64.0f)]
WORLD_NORMALMAP HSBump(InputPatch<WORLD_NORMALMAP, 4> input, uint i : SV_OutputControlPointID, uint nPID : SV_PrimitiveID)
{
	WORLD_NORMALMAP output;
	output.posW		 = input[i].posW;
	output.normalW   = input[i].normalW;
	output.tangentW  = input[i].tangentW;
	output.tex       = input[i].tex;

	return output;
}


[domain("quad")]
PS_WORLD_NORMALMAP DSBump(HCS_EDGE4_IN2 input, float2 uv : SV_DomainLocation, OutputPatch<WORLD_NORMALMAP, 4> quad)
{
	PS_WORLD_NORMALMAP output;

	output.posW      = lerp(lerp(quad[0].posW, quad[1].posW, uv.x), lerp(quad[2].posW, quad[3].posW, uv.x), uv.y);
	output.tex       = lerp(lerp(quad[0].tex, quad[1].tex, uv.x), lerp(quad[2].tex, quad[3].tex, uv.x), uv.y);
	output.normalW   = lerp(lerp(quad[0].normalW, quad[1].normalW, uv.x), lerp(quad[2].normalW, quad[3].normalW, uv.x), uv.y);
	output.tangentW  = lerp(lerp(quad[0].tangentW, quad[1].tangentW, uv.x), lerp(quad[2].tangentW, quad[3].tangentW, uv.x), uv.y);
	output.normalW   = normalize(output.normalW);

	//matrix shadowProj = mul(gmtxWorld, gmtxShadowTransform);
	//output.shadowPos = mul(float4(output.posW, 1.0f), shadowProj);

	float fHeight = gtxtTexture.SampleLevel(gSamplerState, output.tex, 0).a;
	output.posW += (gBumpScale.y * (fHeight - 1.0f)) * output.normalW;
	output.pos    = mul(float4(output.posW, 1.0f), gmtxViewProjection);

	return output;
}
