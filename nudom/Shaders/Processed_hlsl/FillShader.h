#pragma once

#include "../../Render/nuRenderDX_Defs.h"

class nuDXProg_Fill : public nuDXProg
{
public:
	nuDXProg_Fill();
	virtual void			Reset();
	virtual const char*		VertSrc();
	virtual const char*		FragSrc();
	virtual const char*		Name();
	virtual bool			LoadVariablePositions();	// Performs glGet[Uniform|Attrib]Location for all variables. Returns true if all variables are found.
	virtual uint32			PlatformMask();				// Combination of nuPlatform bits.
	virtual nuVertexType	VertexType();				// Only meaningful on DirectX

};

