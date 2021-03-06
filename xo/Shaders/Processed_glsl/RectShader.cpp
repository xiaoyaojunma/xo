#include "pch.h"
#if XO_BUILD_OPENGL
#include "RectShader.h"

xoGLProg_Rect::xoGLProg_Rect()
{
	Reset();
}

void xoGLProg_Rect::Reset()
{
	ResetBase();
	v_mvproj = -1;
	v_vpos = -1;
	v_vcolor = -1;
	v_radius = -1;
	v_box = -1;
	v_border = -1;
	v_border_color = -1;
	v_vport_hsize = -1;
}

const char* xoGLProg_Rect::VertSrc()
{
	return
		"uniform		mat4	mvproj;\n"
		"attribute	vec4	vpos;\n"
		"attribute	vec4	vcolor;\n"
		"varying		vec4	pos;\n"
		"varying		vec4	color;\n"
		"void main()\n"
		"{\n"
		"	pos = mvproj * vpos;\n"
		"	gl_Position = pos;\n"
		"	color = fromSRGB(vcolor);\n"
		"}\n"
;
}

const char* xoGLProg_Rect::FragSrc()
{
	return
		"varying vec4	pos;\n"
		"varying vec4	color;\n"
		"uniform float	radius;\n"
		"uniform vec4	box;\n"
		"uniform vec4	border;\n"
		"uniform vec4	border_color;\n"
		"uniform vec2	vport_hsize;\n"
		"\n"
		"vec2 to_screen(vec2 unit_pt)\n"
		"{\n"
		"	return (vec2(unit_pt.x, -unit_pt.y) + vec2(1,1)) * vport_hsize;\n"
		"}\n"
		"\n"
		"#define LEFT x\n"
		"#define TOP y\n"
		"#define RIGHT z\n"
		"#define BOTTOM w\n"
		"\n"
		"void main()\n"
		"{\n"
		"	vec2 screenxy = to_screen(pos.xy);\n"
		"	float radius_in = max(border.x, radius);\n"
		"	float radius_out = radius;\n"
		"	vec4 out_box = box + vec4(radius, radius, -radius, -radius);\n"
		"	vec4 in_box = box + vec4(max(border.LEFT, radius), max(border.TOP, radius), -max(border.RIGHT, radius), -max(border.BOTTOM, radius));\n"
		"\n"
		"	vec2 cent_in = screenxy;\n"
		"	vec2 cent_out = screenxy;\n"
		"	cent_in.x = clamp(cent_in.x, in_box.LEFT, in_box.RIGHT);\n"
		"	cent_in.y = clamp(cent_in.y, in_box.TOP, in_box.BOTTOM);\n"
		"	cent_out.x = clamp(cent_out.x, out_box.LEFT, out_box.RIGHT);\n"
		"	cent_out.y = clamp(cent_out.y, out_box.TOP, out_box.BOTTOM);\n"
		"\n"
		"	// If you draw the pixels out on paper, and take cognisance of the fact that\n"
		"	// our samples are at pixel centers, then this -0.5 offset makes perfect sense.\n"
		"	// This offset is correct regardless of whether you're blending linearly or in gamma space.\n"
		"	// UPDATE: This is more subtle than it seems. By using a 0.5 offset here, and an additional 0.5 offset\n"
		"	// that is fed into the shader's \"radius\" uniform, we effectively get rectangles to be sharp\n"
		"	// when they are aligned to an integer grid. I haven't thought this through carefully enough,\n"
		"	// but it does feel right.\n"
		"	float dist_out = length(screenxy - cent_out) - 0.5;\n"
		"\n"
		"	float dist_in = length(screenxy - cent_in) - (radius_in - border.x);\n"
		"\n"
		"	vec4 outcolor = color;\n"
		"\n"
		"	float borderWidthX = max(border.x, border.z);\n"
		"	float borderWidthY = max(border.y, border.w);\n"
		"	float borderWidth = max(borderWidthX, borderWidthY);\n"
		"	float borderMix = clamp(dist_in, 0.0, 1.0);\n"
		"	if (borderWidth > 0.5)\n"
		"		outcolor = mix(outcolor, border_color, borderMix);\n"
		"\n"
		"	outcolor.a *= clamp(radius_out - dist_out, 0.0, 1.0);\n"
		"	outcolor = premultiply(outcolor);\n"
		"\n"
		"#ifdef XO_SRGB_FRAMEBUFFER\n"
		"	gl_FragColor = outcolor;\n"
		"#else\n"
		"	float igamma = 1.0/2.2;\n"
		"	gl_FragColor.rgb = pow(outcolor.rgb, vec3(igamma, igamma, igamma));\n"
		"	gl_FragColor.a = outcolor.a;\n"
		"#endif\n"
		"}\n"
;
}

const char* xoGLProg_Rect::Name()
{
	return "Rect";
}


bool xoGLProg_Rect::LoadVariablePositions()
{
	int nfail = 0;

	nfail += (v_mvproj = glGetUniformLocation( Prog, "mvproj" )) == -1;
	nfail += (v_vpos = glGetAttribLocation( Prog, "vpos" )) == -1;
	nfail += (v_vcolor = glGetAttribLocation( Prog, "vcolor" )) == -1;
	nfail += (v_radius = glGetUniformLocation( Prog, "radius" )) == -1;
	nfail += (v_box = glGetUniformLocation( Prog, "box" )) == -1;
	nfail += (v_border = glGetUniformLocation( Prog, "border" )) == -1;
	nfail += (v_border_color = glGetUniformLocation( Prog, "border_color" )) == -1;
	nfail += (v_vport_hsize = glGetUniformLocation( Prog, "vport_hsize" )) == -1;
	if (nfail != 0)
		XOTRACE("Failed to bind %d variables of shader Rect\n", nfail);

	return nfail == 0;
}

uint32 xoGLProg_Rect::PlatformMask()
{
	return xoPlatform_All;
}

xoVertexType xoGLProg_Rect::VertexType()
{
	return xoVertexType_NULL;
}

#endif // XO_BUILD_OPENGL

