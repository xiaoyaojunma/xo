#include "pch.h"
#if XO_BUILD_DIRECTX
#include "FillTexShader.h"

xoDXProg_FillTex::xoDXProg_FillTex()
{
	Reset();
}

void xoDXProg_FillTex::Reset()
{
	ResetBase();

}

const char* xoDXProg_FillTex::VertSrc()
{
	return
		"\n"
		"cbuffer PerFrame : register(b0)\n"
		"{\n"
		"	float4x4		mvproj;\n"
		"	float2			vport_hsize;\n"
		"\n"
		"	Texture2D		shader_texture;\n"
		"	SamplerState	sample_type;\n"
		"};\n"
		"\n"
		"cbuffer PerObject : register(b1)\n"
		"{\n"
		"	float4		box;\n"
		"	float4		border;\n"
		"	float4		border_color;\n"
		"	float		radius;\n"
		"};\n"
		"\n"
		"struct VertexType_PTC\n"
		"{\n"
		"	float4 pos		: POSITION;\n"
		"	float2 uv		: TEXCOORD0;\n"
		"	float4 color	: COLOR;\n"
		"};\n"
		"\n"
		"struct VertexType_PTCV4\n"
		"{\n"
		"	float4 pos		: POSITION;\n"
		"	float2 uv		: TEXCOORD1;\n"
		"	float4 color	: COLOR;\n"
		"	float4 v4		: TEXCOORD2;\n"
		"};\n"
		"\n"
		"float fromSRGB_Component(float srgb)\n"
		"{\n"
		"	float sRGB_Low	= 0.0031308;\n"
		"	float sRGB_a	= 0.055;\n"
		"\n"
		"	if (srgb <= 0.04045)\n"
		"		return srgb / 12.92;\n"
		"	else\n"
		"		return pow(abs((srgb + sRGB_a) / (1.0 + sRGB_a)), 2.4);\n"
		"}\n"
		"\n"
		"float4 fromSRGB(float4 c)\n"
		"{\n"
		"	float4 linear_c;\n"
		"	linear_c.r = fromSRGB_Component(c.r);\n"
		"	linear_c.g = fromSRGB_Component(c.g);\n"
		"	linear_c.b = fromSRGB_Component(c.b);\n"
		"	linear_c.a = c.a;\n"
		"	return linear_c;\n"
		"}\n"
		"\n"
		"float4 premultiply(float4 c)\n"
		"{\n"
		"	return float4(c.r * c.a, c.g * c.a, c.b * c.a, c.a);\n"
		"}\n"
		"\n"
		"// SV_Position is in screen space, but in GLSL it is in normalized device space\n"
		"float2 frag_to_screen(float2 unit_pt)\n"
		"{\n"
		"	return unit_pt;\n"
		"}\n"
		"\n"
		"struct VSOutput\n"
		"{\n"
		"	float4 pos		: SV_Position;\n"
		"	float4 color	: COLOR;\n"
		"	float2 texuv0	: TEXCOORD0;\n"
		"};\n"
		"\n"
		"VSOutput main(VertexType_PTC vertex)\n"
		"{\n"
		"	VSOutput output;\n"
		"	output.pos = mul(mvproj, vertex.pos);\n"
		"	output.color = fromSRGB(vertex.color);\n"
		"	output.texuv0 = vertex.uv;\n"
		"	return output;\n"
		"}\n"
;
}

const char* xoDXProg_FillTex::FragSrc()
{
	return
		"\n"
		"cbuffer PerFrame : register(b0)\n"
		"{\n"
		"	float4x4		mvproj;\n"
		"	float2			vport_hsize;\n"
		"\n"
		"	Texture2D		shader_texture;\n"
		"	SamplerState	sample_type;\n"
		"};\n"
		"\n"
		"cbuffer PerObject : register(b1)\n"
		"{\n"
		"	float4		box;\n"
		"	float4		border;\n"
		"	float4		border_color;\n"
		"	float		radius;\n"
		"};\n"
		"\n"
		"struct VertexType_PTC\n"
		"{\n"
		"	float4 pos		: POSITION;\n"
		"	float2 uv		: TEXCOORD0;\n"
		"	float4 color	: COLOR;\n"
		"};\n"
		"\n"
		"struct VertexType_PTCV4\n"
		"{\n"
		"	float4 pos		: POSITION;\n"
		"	float2 uv		: TEXCOORD1;\n"
		"	float4 color	: COLOR;\n"
		"	float4 v4		: TEXCOORD2;\n"
		"};\n"
		"\n"
		"float fromSRGB_Component(float srgb)\n"
		"{\n"
		"	float sRGB_Low	= 0.0031308;\n"
		"	float sRGB_a	= 0.055;\n"
		"\n"
		"	if (srgb <= 0.04045)\n"
		"		return srgb / 12.92;\n"
		"	else\n"
		"		return pow(abs((srgb + sRGB_a) / (1.0 + sRGB_a)), 2.4);\n"
		"}\n"
		"\n"
		"float4 fromSRGB(float4 c)\n"
		"{\n"
		"	float4 linear_c;\n"
		"	linear_c.r = fromSRGB_Component(c.r);\n"
		"	linear_c.g = fromSRGB_Component(c.g);\n"
		"	linear_c.b = fromSRGB_Component(c.b);\n"
		"	linear_c.a = c.a;\n"
		"	return linear_c;\n"
		"}\n"
		"\n"
		"float4 premultiply(float4 c)\n"
		"{\n"
		"	return float4(c.r * c.a, c.g * c.a, c.b * c.a, c.a);\n"
		"}\n"
		"\n"
		"// SV_Position is in screen space, but in GLSL it is in normalized device space\n"
		"float2 frag_to_screen(float2 unit_pt)\n"
		"{\n"
		"	return unit_pt;\n"
		"}\n"
		"\n"
		"struct VSOutput\n"
		"{\n"
		"	float4 pos		: SV_Position;\n"
		"	float4 color	: COLOR;\n"
		"	float2 texuv0	: TEXCOORD0;\n"
		"};\n"
		"\n"
		"float4 main(VSOutput input) : SV_Target\n"
		"{\n"
		"	float4 col;\n"
		"	col = input.color * shader_texture.Sample(sample_type, input.texuv0);\n"
		"    return col;\n"
		"}\n"
;
}

const char* xoDXProg_FillTex::Name()
{
	return "FillTex";
}


bool xoDXProg_FillTex::LoadVariablePositions()
{
	int nfail = 0;

	if (nfail != 0)
		XOTRACE("Failed to bind %d variables of shader FillTex\n", nfail);

	return nfail == 0;
}

uint32 xoDXProg_FillTex::PlatformMask()
{
	return xoPlatform_All;
}

xoVertexType xoDXProg_FillTex::VertexType()
{
	return xoVertexType_PTC;
}

#endif // XO_BUILD_DIRECTX

