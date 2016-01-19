//#define LUMCOLOR


static float  MIDDLE_GRAY = 0.72f;
static float  LUM_WHITE = 2.0f;
static const float  BRIGHT_THRESHOLD = 0.5f;


float3 ColorToLum(float3 fColor)
{
	const float3x3 RGBtoXYZ = {
		0.5141364, 0.3238786, 0.16036376,
		0.265068, 0.67023428, 0.06409157,
		0.0241188, 0.1228178, 0.84442666
	};

	//const float3x3 RGBtoXYZ = {
	//	0.4124, 0.3576, 0.1805,
	//	0.2126, 0.7152, 0.7222,
	//	0.0193, 0.1192, 0.9505
	//};

	float3 XYZ = mul(RGBtoXYZ, fColor);

	float3 Yxy;
	Yxy.r = XYZ.g;
	//x = X / (X + Y + Z)
	//y = X / (X + Y + Z)

	float temp = dot(float3(1.0f, 1.0f, 1.0f), XYZ.rgb);
	Yxy.gb = XYZ.rg / temp;

	return Yxy;
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

float3 ToneMappingByLum(float3 fLum)
{
	float3 Yxy = fLum;
	float LumScaled = fLum.r * MIDDLE_GRAY / (fLum.x + 0.001f);
	Yxy.r = (LumScaled * (1.0f + LumScaled / LUM_WHITE)) / (1.0f + LumScaled);
	return Yxy;
}



float4 FilmicToneMapping(float4 LinearColor)
{
	float3 color = max(0, LinearColor - 0.004f);
	return float4((color * (6.2 * color + 0.5)) / (color * (6.2 * color + 1.7) + 0.06), 1);
}


float3 ColorScaled(float3 fColor, float fLum, float middle_gray)
{
	float3 fLumScaled = fColor.rgb * (middle_gray / (fLum + 0.001f));

	//	vColor.rgb *= middle_gray / (fLum + 0.001f);
	fLumScaled *= (1.0f + fLumScaled / LUM_WHITE);
	fLumScaled /= (1.0f + fLumScaled);

	return fLumScaled;
}

float LumScaled(float3 fColor, float fLum, float middle_gray)
{
	//float3 vColor = fColor;
	float LumScaled = fColor.r;

	LumScaled *= middle_gray / (fLum + 0.001f);
	LumScaled *= (1.0f + LumScaled / LUM_WHITE);
	LumScaled /= (1.0f + LumScaled);

	return LumScaled;
}

float3 ColorReinhardCompress(float3 fColor, float fLum, float middle_gray)
{
	float3 fLumScaled = fColor.rgb * middle_gray / (fLum + 0.001f);

	float3 fCompress = (fLumScaled * (1.0f + (fLumScaled / LUM_WHITE*LUM_WHITE)));
	fCompress /= (1.0f + fLumScaled);
	return fCompress;
}


float4 HDRToLDR(float3 HDRColor)
{
	float3 fColor;
	fColor = HDRColor * (1 + (HDRColor / LUM_WHITE * LUM_WHITE));
	return float4(fColor / (1.0f + HDRColor), 1);
}

float CalculateMiddleGray(float fLum)
{
	return LUM_WHITE - (2.0f / (2.0f + log10(fLum + 1.0f)));
}


float4 CalculateToneColor(float4 Color, float fLum, float middle_gray)
{
	float3 fColor = Color;
#ifdef _LUMCOLOR
	fColor.r = LumScaled(fColor, fLum, middle_gray);
	fColor = LumToColor(fColor);
#else
	fColor = ColorScaled(fColor, fLum, middle_gray);
	//	fColor = ColorReinhardCompress(fColor, fLum, middle_gray);
#endif
	fColor = HDRToLDR(fColor);
	return float4(fColor, 1);
}
