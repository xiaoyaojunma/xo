#include "pch.h"
#include "nuRenderGL.h"
#include "../Image/nuImage.h"
#include "nuTextureAtlas.h"
#include "../Text/nuGlyphCache.h"

nuRenderGL::nuRenderGL()
{
	AllProgs[0] = &PRect;
	AllProgs[1] = &PFill;
	AllProgs[2] = &PFillTex;
	AllProgs[3] = &PTextRGB;
	AllProgs[4] = &PTextWhole;
	AllProgs[5] = &PCurve;
	static_assert(NumProgs == 6, "Add your new shader here");
	Reset();
}

nuRenderGL::~nuRenderGL()
{
}

void nuRenderGL::Reset()
{
	for ( int i = 0; i < NumProgs; i++ )
		AllProgs[i]->Reset();
}

bool nuRenderGL::CreateShaders()
{
	Check();

	if ( SingleTex2D == 0 )
		glGenTextures( 1, &SingleTex2D );
	if ( SingleTexAtlas2D == 0 )
		glGenTextures( 1, &SingleTexAtlas2D );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, SingleTex2D );

	Check();

	for ( int i = 0; i < NumProgs; i++ )
	{
		if ( AllProgs[i]->UseOnThisPlatform() )
		{
			if ( !LoadProgram( *AllProgs[i] ) ) return false;
			if ( !AllProgs[i]->LoadVariablePositions() ) return false;
		}
	}

	/*
	// old manual
	if ( !LoadProgram( PRect, pRectVert, pRectFrag ) ) return false;
	if ( !LoadProgram( PFill, pFillVert, pFillFrag ) ) return false;
	if ( !LoadProgram( PFillTex, pFillTexVert, pFillTexFrag ) ) return false;
	if ( !LoadProgram( PTextRGB, pTextRGBVert, textRGBFrag.Z ) ) return false;
	if ( !LoadProgram( PTextWhole, pTextWholeVert, pTextWholeFrag ) ) return false;
	//if ( !LoadProgram( PCurve, pCurveVert, pCurveFrag ) ) return false;

	//glUseProgram( PRect.Prog );
	VarRectBox = glGetUniformLocation( PRect.Prog, "box" );
	VarRectRadius = glGetUniformLocation( PRect.Prog, "radius" );
	VarRectVPortHSize = glGetUniformLocation( PRect.Prog, "vport_hsize" );
	VarRectMVProj = glGetUniformLocation( PRect.Prog, "mvproj" );
	VarRectVPos = glGetAttribLocation( PRect.Prog, "vpos" );
	VarRectVColor = glGetAttribLocation( PRect.Prog, "vcolor" );
	//VarCurveTex0 = glGetAttribLocation( PCurve.Prog, "vtex0" );

	//glUseProgram( PFill.Prog );
	VarFillMVProj = glGetUniformLocation( PFill.Prog, "mvproj" );
	VarFillVPos = glGetAttribLocation( PFill.Prog, "vpos" );
	VarFillVColor = glGetAttribLocation( PFill.Prog, "vcolor" );

	//glUseProgram( PFillTex.Prog );
	VarFillTexMVProj = glGetUniformLocation( PFillTex.Prog, "mvproj" );
	VarFillTexVPos = glGetAttribLocation( PFillTex.Prog, "vpos" );
	VarFillTexVColor = glGetAttribLocation( PFillTex.Prog, "vcolor" );
	VarFillTexVUV = glGetAttribLocation( PFillTex.Prog, "vtexuv0" );
	VarFillTex0 = glGetUniformLocation( PFillTex.Prog, "tex0" );

	VarTextRGBMVProj = glGetUniformLocation( PTextRGB.Prog, "mvproj" );
	VarTextRGBVPos = glGetAttribLocation( PTextRGB.Prog, "vpos" );
	VarTextRGBVColor = glGetAttribLocation( PTextRGB.Prog, "vcolor" );
	VarTextRGBVUV = glGetAttribLocation( PTextRGB.Prog, "vtexuv0" );
	VarTextRGBVClamp = glGetAttribLocation( PTextRGB.Prog, "vtexClamp" );
	VarTextRGBTex0 = glGetUniformLocation( PTextRGB.Prog, "tex0" );
#if NU_WIN_DESKTOP
	glBindFragDataLocation( PTextRGB.Prog, 0, "outputColor0" );
	glBindFragDataLocation( PTextRGB.Prog, 1, "outputColor1" );
	Check();
	GLint c0 = glGetFragDataLocation( PTextRGB.Prog, "outputColor0" );
	GLint c1 = glGetFragDataLocation( PTextRGB.Prog, "outputColor1" );
	GLint c2 = glGetFragDataLocation( PTextRGB.Prog, "outputColor2" );
	Check();
#endif

	VarTextWholeMVProj = glGetUniformLocation( PTextWhole.Prog, "mvproj" );
	VarTextWholeVPos = glGetAttribLocation( PTextWhole.Prog, "vpos" );
	VarTextWholeVColor = glGetAttribLocation( PTextWhole.Prog, "vcolor" );
	VarTextWholeVUV = glGetAttribLocation( PTextWhole.Prog, "vtexuv0" );
	VarTextWholeTex0 = glGetUniformLocation( PTextWhole.Prog, "tex0" );

	NUTRACE( "VarFillMVProj = %d\n", VarFillMVProj );
	NUTRACE( "VarFillVPos = %d\n", VarFillVPos );
	NUTRACE( "VarFillVColor = %d\n", VarFillVColor );

	NUTRACE( "VarFillTexMVProj = %d\n", VarFillTexMVProj );
	NUTRACE( "VarFillTexVPos = %d\n", VarFillTexVPos );
	NUTRACE( "VarFillTexVColor = %d\n", VarFillTexVColor );
	NUTRACE( "VarFillTexVUV = %d\n", VarFillTexVUV );
	NUTRACE( "VarFillTex0 = %d\n", VarFillTex0 );

	NUTRACE( "VarTextRGBMVProj = %d\n", VarTextRGBMVProj );
	NUTRACE( "VarTextRGBVPos = %d\n", VarTextRGBVPos );
	NUTRACE( "VarTextRGBVColor = %d\n", VarTextRGBVColor );
	NUTRACE( "VarTextRGBVUV = %d\n", VarTextRGBVUV );
	NUTRACE( "VarTextRGBVClamp = %d\n", VarTextRGBVClamp );
	NUTRACE( "VarTextRGBTex0 = %d\n", VarTextRGBTex0 );
	*/

	//glUseProgram( 0 );
	
	Check();

	return true;
}

void nuRenderGL::DeleteShaders()
{
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	if ( SingleTex2D != 0 )
		glDeleteTextures( 1, &SingleTex2D );
	SingleTex2D = 0;
	if ( SingleTexAtlas2D != 0 )
		glDeleteTextures( 1, &SingleTexAtlas2D );

	glUseProgram( 0 );
	for ( int i = 0; i < NumProgs; i++ )
	{
		DeleteProgram( *AllProgs[i] );
	}
	// DeleteProgram( PFill );
	// DeleteProgram( PFillTex );
	// DeleteProgram( PTextRGB );
	// DeleteProgram( PTextWhole );
	// DeleteProgram( PRect );
	// DeleteProgram( PCurve );
}

void nuRenderGL::SurfaceLost()
{
	Reset();
	CreateShaders();
}

#ifndef GL_SRC1_COLOR
#define GL_SRC1_COLOR                                      0x88F9
#endif
#ifndef GL_ONE_MINUS_SRC1_COLOR
#define GL_ONE_MINUS_SRC1_COLOR                            0x88FA
#endif
#ifndef GL_ONE_MINUS_SRC1_ALPHA
#define GL_ONE_MINUS_SRC1_ALPHA                            0x88FB
#endif

void nuRenderGL::ActivateProgram( nuGLProg& p )
{
	if ( ActiveProgram == &p ) return;
	ActiveProgram = &p;
	NUASSERT( p.Prog != 0 );
	glUseProgram( p.Prog );
	if ( ActiveProgram == &PTextRGB )
	{
		// outputColor0 = vec4(color.r, color.g, color.b, avgA);
		// outputColor1 = vec4(aR, aG, aB, avgA);
		glBlendFuncSeparate( GL_SRC1_COLOR, GL_ONE_MINUS_SRC1_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	}
	else
	{
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );	// this is for non-premultiplied
		//glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );			// this is premultiplied
	}
	Check();
}

void nuRenderGL::Ortho( nuMat4f &imat, double left, double right, double bottom, double top, double znear, double zfar )
{
	nuMat4f m;
	m.Zero();
	double A = 2 / (right - left);
	double B = 2 / (top - bottom);
	double C = -2 / (zfar - znear);
	double tx = -(right + left) / (right - left);
	double ty = -(top + bottom) / (top - bottom);
	double tz = -(zfar + znear) / (zfar - znear);
	m.m(0,0) = (float) A;
	m.m(1,1) = (float) B;
	m.m(2,2) = (float) C;
	m.m(3,3) = 1;
	m.m(0,3) = (float) tx;
	m.m(1,3) = (float) ty;
	m.m(2,3) = (float) tz;
	imat = imat * m;
}

void nuRenderGL::PreRender( int fbwidth, int fbheight )
{
	Check();

	NUTRACE_RENDER( "PreRender %d %d\n", fbwidth, fbheight );
	Check();

	FBWidth = fbwidth;
	FBHeight = fbheight;
	glViewport( 0, 0, fbwidth, fbheight );

	//glMatrixMode( GL_PROJECTION );
	//glLoadIdentity();
	//glOrtho( 0, fbwidth, fbheight, 0, 0, 1 );

	//glMatrixMode( GL_MODELVIEW );
	//glLoadIdentity();

	// Make our clear color a very noticeable purple, so you know when you've screwed up the root node
	glClearColor( 0.7f, 0.0, 0.7f, 0 );

	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	if ( nuGlobal()->EnableSRGBFramebuffer )
		glEnable( GL_FRAMEBUFFER_SRGB );

	NUTRACE_RENDER( "PreRender 2\n" );
	Check();

	// Enable CULL_FACE because it will make sure that we are consistent about vertex orientation
	//glEnable( GL_CULL_FACE );
	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	NUTRACE_RENDER( "PreRender 3\n" );
	Check();

	nuMat4f mvproj;
	mvproj.Identity();
	Ortho( mvproj, 0, fbwidth, fbheight, 0, 0, 1 );
	// GLES doesn't support TRANSPOSE = TRUE
	mvproj = mvproj.Transposed();

	//ActivateProgram( PRect );
	//glUniform2f( VarRectVPortHSize, FBWidth / 2.0f, FBHeight / 2.0f );
	//glUniformMatrix4fv( VarRectMVProj, 1, false, &mvproj.row[0].x );
	ActivateProgram( PRect );
	glUniform2f( PRect.v_vport_hsize, FBWidth / 2.0f, FBHeight / 2.0f );
	glUniformMatrix4fv( PRect.v_mvproj, 1, false, &mvproj.row[0].x );
	Check();

	ActivateProgram( PFill );

	NUTRACE_RENDER( "PreRender 4 (%d)\n", VarFillMVProj );
	Check();

	glUniformMatrix4fv( PFill.v_mvproj, 1, false, &mvproj.row[0].x );

	NUTRACE_RENDER( "PreRender 5\n" );
	Check();

	ActivateProgram( PFillTex );

	NUTRACE_RENDER( "PreRender 6 (%d)\n", PFillTex.v_mvproj );
	Check();

	glUniformMatrix4fv( PFillTex.v_mvproj, 1, false, &mvproj.row[0].x );

	ActivateProgram( PTextRGB );

	NUTRACE_RENDER( "PreRender 7 (%d)\n", PTextRGB.v_mvproj );
	Check();

	glUniformMatrix4fv( PTextRGB.v_mvproj, 1, false, &mvproj.row[0].x );

	ActivateProgram( PTextWhole );

	NUTRACE_RENDER( "PreRender 8 (%d)\n", PTextWhole.v_mvproj );
	Check();

	glUniformMatrix4fv( PTextWhole.v_mvproj, 1, false, &mvproj.row[0].x );

	NUTRACE_RENDER( "PreRender done\n" );

	//glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	//glEnableClientState( GL_COLOR_ARRAY );
}

void nuRenderGL::PostRenderCleanup()
{
	//glDisableVertexAttribArray( VarRectVPos );
	//glDisableVertexAttribArray( VarRectVColor );
	//glDisableVertexAttribArray( VarFillVPos );
	glUseProgram( 0 );
	ActiveProgram = NULL;
}

void nuRenderGL::DrawQuad( const void* v )
{
	NUTRACE_RENDER( "DrawQuad\n" );

	int stride = sizeof(nuVx_PTC);
	const byte* vbyte = (const byte*) v;

	GLint varvpos = 0;
	GLint varvcol = 0;
	GLint varvtex0 = 0;
	GLint varvtexClamp = 0;
	GLint vartex0 = 0;
	if ( ActiveProgram == &PRect )
	{
		varvpos = PRect.v_vpos;
		varvcol = PRect.v_vcolor;
	}
	else if ( ActiveProgram == &PFill )
	{
		varvpos = PFill.v_vpos;
		varvcol = PFill.v_vcolor;
	}
	else if ( ActiveProgram == &PFillTex )
	{
		varvpos = PFillTex.v_vpos;
		varvcol = PFillTex.v_vcolor;
		varvtex0 = PFillTex.v_vtexuv0;
		vartex0 = PFillTex.v_tex0;
	}
	else if ( ActiveProgram == &PTextRGB )
	{
		stride = sizeof(nuVx_PTCV4);
		varvpos = PTextRGB.v_vpos;
		varvcol = PTextRGB.v_vcolor;
		varvtex0 = PTextRGB.v_vtexuv0;
		varvtexClamp = PTextRGB.v_vtexClamp;
		vartex0 = PTextRGB.v_tex0;
	}
	else if ( ActiveProgram == &PTextWhole )
	{
		varvpos = PTextWhole.v_vpos;
		varvcol = PTextWhole.v_vcolor;
		varvtex0 = PTextWhole.v_vtexuv0;
		vartex0 = PTextWhole.v_tex0;
	}

	// We assume here that nuVx_PTC and nuVx_PTCV4 share the same base layout
	glVertexAttribPointer( varvpos, 3, GL_FLOAT, false, stride, vbyte );
	glEnableVertexAttribArray( varvpos );
	
	glVertexAttribPointer( varvcol, 4, GL_UNSIGNED_BYTE, true, stride, vbyte + offsetof(nuVx_PTC, Color) );
	glEnableVertexAttribArray( varvcol );

	if ( varvtex0 != 0 )
	{
		glUniform1i( vartex0, 0 );
		glVertexAttribPointer( varvtex0, 2, GL_FLOAT, true, stride, vbyte + offsetof(nuVx_PTC, UV) );
		glEnableVertexAttribArray( varvtex0 );
	}
	if ( varvtexClamp != 0 )
	{
		glVertexAttribPointer( varvtexClamp, 4, GL_FLOAT, true, stride, vbyte + offsetof(nuVx_PTCV4, V4) );
		glEnableVertexAttribArray( varvtexClamp );
	}
	//glVertexPointer( 3, GL_FLOAT, stride, vbyte );
	//glEnableClientState( GL_VERTEX_ARRAY );

	//glTexCoordPointer( 2, GL_FLOAT, stride, vbyte + offsetof(nuVx_PTC, UV) );
	//glColorPointer( GL_BGRA, GL_UNSIGNED_BYTE, stride, vbyte + offsetof(nuVx_PTC, Color) );
	uint16 indices[6];
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 3;
	glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
	//glDrawArrays( GL_TRIANGLES, 0, 3 );

	NUTRACE_RENDER( "DrawQuad done\n" );

	/*
	glBegin( GL_TRIANGLES );
	glColor4f( 1, 0, 0, 1 );
	glVertex3f( 0, 0, 0 );
	glVertex3f( 0, 100, 0 );
	glVertex3f( 100, 0, 0 );
	glEnd();
	*/
	Check();
}

void nuRenderGL::DrawTriangles( int nvert, const void* v, const uint16* indices )
{
	int stride = sizeof(nuVx_PTC);
	const byte* vbyte = (const byte*) v;
	//glVertexPointer( 3, GL_FLOAT, stride, vbyte );
	//glTexCoordPointer( 2, GL_FLOAT, stride, vbyte + offsetof(nuVx_PTC, UV) );
	//glColorPointer( GL_BGRA, GL_UNSIGNED_BYTE, stride, vbyte + offsetof(nuVx_PTC, Color) );
	glDrawElements( GL_TRIANGLES, nvert, GL_UNSIGNED_SHORT, indices );
	Check();
}

void nuRenderGL::LoadTexture( const nuImage* img )
{
	if ( SingleTex2D == 0 )
		glGenTextures( 1, &SingleTex2D );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, SingleTex2D );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, img->GetWidth(), img->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img->GetData() );
	glGenerateMipmap( GL_TEXTURE_2D );
}

void nuRenderGL::LoadTextureAtlas( const nuTextureAtlas* atlas )
{
	if ( SingleTexAtlas2D == 0 )
		glGenTextures( 1, &SingleTexAtlas2D );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, SingleTexAtlas2D );
#if NU_WIN_DESKTOP
	//int internalFormat = atlas->GetBytesPerTexel() == 1 ? GL_SLUMINANCE8 : GL_RGB;
	int internalFormat = atlas->GetBytesPerTexel() == 1 ? GL_LUMINANCE : GL_RGB;
	int format = atlas->GetBytesPerTexel() == 1 ? GL_LUMINANCE : GL_RGB;
#else
	int internalFormat = atlas->GetBytesPerTexel() == 1 ? GL_LUMINANCE : GL_RGB;
	int format = atlas->GetBytesPerTexel() == 1 ? GL_LUMINANCE : GL_RGB;
#endif
	glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, atlas->GetWidth(), atlas->GetHeight(), 0, format, GL_UNSIGNED_BYTE, atlas->DataAt(0,0) );
	// all assuming this is for a glyph atlas
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	// Clamping should have no effect for RGB text, since we clamp inside our fragment shader.
	// Also, when rendering 'whole pixel' glyphs, we shouldn't need clamping either, because
	// our UV coordinates are exact, and we always have a 1:1 texel:pixel ratio.
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	// not necessary for text where we sample NN
	//glGenerateMipmap( GL_TEXTURE_2D );
}

void nuRenderGL::DeleteProgram( nuGLProg& prog )
{
	if ( prog.Prog ) glDeleteShader( prog.Prog );
	if ( prog.Vert ) glDeleteShader( prog.Vert );
	if ( prog.Frag ) glDeleteShader( prog.Frag );
	prog = nuGLProg();
}

bool nuRenderGL::LoadProgram( nuGLProg& prog )
{
	return LoadProgram( prog.Vert, prog.Frag, prog.Prog, prog.VertSrc(), prog.FragSrc() );
}

bool nuRenderGL::LoadProgram( nuGLProg& prog, const char* vsrc, const char* fsrc )
{
	return LoadProgram( prog.Vert, prog.Frag, prog.Prog, vsrc, fsrc );
}

bool nuRenderGL::LoadProgram( GLuint& vshade, GLuint& fshade, GLuint& prog, const char* vsrc, const char* fsrc )
{
	NUASSERT(glGetError() == GL_NO_ERROR);

	if ( !LoadShader( GL_VERTEX_SHADER, vshade, vsrc ) ) return false;
	if ( !LoadShader( GL_FRAGMENT_SHADER, fshade, fsrc ) ) return false;

	prog = glCreateProgram();

	glAttachShader( prog, vshade );
	glAttachShader( prog, fshade );
	glLinkProgram( prog );

	int ilen;
	const int maxBuff = 8000;
	GLchar ibuff[maxBuff];

	GLint linkStat;
	glGetProgramiv( prog, GL_LINK_STATUS, &linkStat );
	glGetProgramInfoLog( prog, maxBuff, &ilen, ibuff );
	NUTRACE( ibuff );
	if ( ibuff[0] != 0 ) NUTRACE( "\n" );
	if ( linkStat == 0 ) 
		return false;

	return glGetError() == GL_NO_ERROR;
}

bool nuRenderGL::LoadShader( GLenum shaderType, GLuint& shader, const char* raw_src )
{
	NUASSERT(glGetError() == GL_NO_ERROR);

	shader = glCreateShader( shaderType );

	nuString processed(raw_src);
	processed.ReplaceAll( "NU_GLYPH_ATLAS_SIZE", fmt("%v", nuGlyphAtlasSize).Z );

	GLchar* vstring[1];
	vstring[0] = (GLchar*) processed.Z;

	glShaderSource( shader, 1, (const GLchar**) vstring, NULL );
	
	int ilen;
	const int maxBuff = 8000;
	GLchar ibuff[maxBuff];

	GLint compileStat;
	glCompileShader( shader );
	glGetShaderiv( shader, GL_COMPILE_STATUS, &compileStat );
	glGetShaderInfoLog( shader, maxBuff, &ilen, ibuff );
	//glGetInfoLogARB( shader, maxBuff, &ilen, ibuff );
	NUTRACE( ibuff );
	if ( compileStat == 0 ) 
		return false;

	return glGetError() == GL_NO_ERROR;
}

void nuRenderGL::Check()
{
	int e = glGetError();
	if ( e != GL_NO_ERROR )
	{
		NUTRACE( "glError = %d\n", e );
	}
	//NUASSERT( glGetError() == GL_NO_ERROR );
}

