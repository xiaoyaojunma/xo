#include "pch.h"
#if XO_BUILD_OPENGL
#include "TextWholeShader.h"

xoGLProg_TextWhole::xoGLProg_TextWhole()
{
	Reset();
}

void xoGLProg_TextWhole::Reset()
{
	ResetBase();
	v_mvproj = -1;
	v_vpos = -1;
	v_vcolor = -1;
	v_vtexuv0 = -1;
	v_tex0 = -1;
}

const char* xoGLProg_TextWhole::VertSrc()
{
	return
		"uniform		mat4	mvproj;\n"
		"attribute	vec4	vpos;\n"
		"attribute	vec4	vcolor;\n"
		"attribute	vec2	vtexuv0;\n"
		"varying		vec4	color;\n"
		"varying		vec2	texuv0;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = mvproj * vpos;\n"
		"	texuv0 = vtexuv0;\n"
		"	color = fromSRGB(vcolor);\n"
		"}\n"
;
}

const char* xoGLProg_TextWhole::FragSrc()
{
	return
		"uniform sampler2D	tex0;\n"
		"varying vec4		color;\n"
		"varying vec2		texuv0;\n"
		"void main()\n"
		"{\n"
		"	vec4 texCol = texture2D(tex0, texuv0.st);\n"
		"	gl_FragColor = premultiply(color) * texCol.r;\n"
		"}\n"
		"\n"
;
}

const char* xoGLProg_TextWhole::Name()
{
	return "TextWhole";
}


bool xoGLProg_TextWhole::LoadVariablePositions()
{
	int nfail = 0;

	nfail += (v_mvproj = glGetUniformLocation( Prog, "mvproj" )) == -1;
	nfail += (v_vpos = glGetAttribLocation( Prog, "vpos" )) == -1;
	nfail += (v_vcolor = glGetAttribLocation( Prog, "vcolor" )) == -1;
	nfail += (v_vtexuv0 = glGetAttribLocation( Prog, "vtexuv0" )) == -1;
	nfail += (v_tex0 = glGetUniformLocation( Prog, "tex0" )) == -1;
	if (nfail != 0)
		XOTRACE("Failed to bind %d variables of shader TextWhole\n", nfail);

	return nfail == 0;
}

uint32 xoGLProg_TextWhole::PlatformMask()
{
	return xoPlatform_All;
}

xoVertexType xoGLProg_TextWhole::VertexType()
{
	return xoVertexType_NULL;
}

#endif // XO_BUILD_OPENGL

