#include "pch.h"
#include "xoRenderDomEl.h"
#include "xoRenderStack.h"

xoRenderDomEl::xoRenderDomEl(xoInternalID id, xoTag tag) : InternalID(id), Tag(tag)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

xoRenderDomNode::xoRenderDomNode(xoInternalID id, xoTag tag, xoPool* pool) : xoRenderDomEl(id, tag)
{
	SetPool(pool);
}

void xoRenderDomNode::Discard()
{
	InternalID = 0;
	Children.clear();
}

void xoRenderDomNode::SetStyle(xoRenderStack& stack)
{
	Style.BackgroundImageID = stack.Get(xoCatBackgroundImage).GetStringID();
	Style.BackgroundColor = stack.Get(xoCatBackground).GetColor();
	Style.BorderColor = stack.Get(xoCatBorderColor_Left).GetColor();
	Style.HasHoverStyle = stack.HasHoverStyle();
	Style.HasFocusStyle = stack.HasFocusStyle();
}

void xoRenderDomNode::SetPool(xoPool* pool)
{
	Children.Pool = pool;
}

xoBox xoRenderDomNode::BorderBox() const
{
	xoBox box = Pos;
	box.Left -= Style.Padding.Left + Style.BorderSize.Left;
	box.Top -= Style.Padding.Top + Style.BorderSize.Top;
	box.Right += Style.Padding.Right + Style.BorderSize.Right;
	box.Bottom += Style.Padding.Bottom + Style.BorderSize.Bottom;
	return box;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

xoRenderDomText::xoRenderDomText(xoInternalID id, xoPool* pool) : xoRenderDomEl(id, xoTagText)
{
	Text.Pool = pool;
	FontID = xoFontIDNull;
	FontSizePx = 0;
	Flags = 0;
}

void xoRenderDomText::SetStyle(xoRenderStack& stack)
{
	Color = stack.Get(xoCatColor).GetColor();
}
