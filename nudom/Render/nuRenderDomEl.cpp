#include "pch.h"
#include "nuRenderDomEl.h"
#include "nuRenderStack.h"

nuRenderDomEl::nuRenderDomEl( nuInternalID id, nuTag tag ) : InternalID(id), Tag(tag)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

nuRenderDomNode::nuRenderDomNode( nuInternalID id, nuTag tag, nuPool* pool ) : nuRenderDomEl(id, tag)
{
	SetPool( pool );
}

void nuRenderDomNode::Discard()
{
	InternalID = 0;
	Children.clear();
}

void nuRenderDomNode::SetStyle( nuRenderStack& stack )
{
	auto bgColor = stack.Get( nuCatBackground );
	auto bgImage = stack.Get( nuCatBackgroundImage );
	if ( !bgColor.IsNull() ) Style.BackgroundColor = bgColor.GetColor();
	if ( !bgImage.IsNull() ) Style.BackgroundImageID = bgImage.GetStringID();
}

void nuRenderDomNode::SetPool( nuPool* pool )
{
	Children.Pool = pool;
}

nuBox nuRenderDomNode::BorderBox() const
{
	nuBox box = Pos;
	box.Left -= Style.BorderSize.Left;
	box.Top -= Style.BorderSize.Top;
	box.Right += Style.BorderSize.Right;
	box.Bottom += Style.BorderSize.Bottom;
	return box;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

nuRenderDomText::nuRenderDomText( nuInternalID id, nuPool* pool ) : nuRenderDomEl( id, nuTagText )
{
	Text.Pool = pool;
	FontID = nuFontIDNull;
	Char = 0;
	FontSizePx = 0;
	Flags = 0;
}

void nuRenderDomText::SetStyle( nuRenderStack& stack )
{
	Color = stack.Get( nuCatColor ).GetColor();
}
