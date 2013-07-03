/******************************************************************************
 *  CustomHUD.h: script to draw custom HUD elements
 *
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
 *  Adapted in part from Public Scripts
 *  Copyright (C) 2005-2011 Tom N Harris <telliamed@whoopdedo.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#ifndef CUSTOMHUD_H
#define CUSTOMHUD_H

#if !SCR_GENSCRIPTS
#include <list>
#include <lg/interface.h>
#include <lg/scrservices.h>
#include "BaseScript.h"
#include "scriptvars.h"
#endif // !SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
struct CanvasPoint
{
	int x, y;
	CanvasPoint (int x = 0, int y = 0);
	bool operator == (const CanvasPoint& rhs) const;
	bool operator != (const CanvasPoint& rhs) const;
	CanvasPoint operator + (const CanvasPoint& rhs) const;
	CanvasPoint operator - (const CanvasPoint& rhs) const;
	CanvasPoint operator * (int rhs) const;
};
struct CanvasSize
{
	int w, h;
	CanvasSize (int w = 0, int h = 0);
	bool operator == (const CanvasSize& rhs) const;
};
struct CanvasRect
{
	int x, y, w, h;
	CanvasRect (int x = 0, int y = 0, int w = 0, int h = 0);
	bool operator == (const CanvasRect& rhs) const;
};
static const CanvasPoint ORIGIN = { 0, 0 };
static const CanvasPoint OFFSCREEN = { -1, -1 };
static const CanvasRect NOCLIP = { 0, 0, -1, -1 };
static const CanvasRect OFFSCREEN_R = { -1, -1, -1, -1 };
#endif // !SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_HUDElement;
typedef std::list<cScr_HUDElement*> HUDElements;

class cScr_CustomHUD : public virtual cBaseScript, public IDarkOverlayHandler
{
public:
	cScr_CustomHUD (const char* pszName, int iHostObjId);

	STDMETHOD_(void, DrawHUD) ();
	STDMETHOD_(void, DrawTOverlay) ();
	STDMETHOD_(void, OnUIEnterMode) ();

protected:
	virtual long OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnMessage (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply);

private:
	HUDElements elements;
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDCustomHUD","BaseScript",cScr_CustomHUD)
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_HUDElement : public virtual cBaseScript
{
public:
	cScr_HUDElement (const char* pszName, int iHostObjId);

	void DrawStage1 ();
	void DrawStage2 ();
	virtual void EnterGameMode ();

protected:
	virtual long OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnMessage (sScrMsg* pMsg, cMultiParm& mpReply);

	bool SubscribeProperty (const char* property);
	virtual void OnPropertyChanged (const char* property);

	bool GetParamBool (const char* param, bool default_value);
	ulong GetParamColor (const char* param, ulong default_value);
	float GetParamFloat (const char* param, float default_value);
	int GetParamInt (const char* param, int default_value);
	char* GetParamString (const char* param, const char* default_value);

	void SetParamBool (const char* param, bool value);
	// no support for setting a color parameter
	void SetParamFloat (const char* param, float value);
	void SetParamInt (const char* param, int value);
	void SetParamString (const char* param, const char* value);

	virtual bool Prepare ();
	virtual void Redraw () = 0;
	bool NeedsRedraw ();
	void ScheduleRedraw ();

	void SetIsOverlay (bool is_overlay);
	void SetOpacity (int opacity);

	CanvasSize GetCanvasSize ();
	void SetPosition (CanvasPoint position);
	void SetSize (CanvasSize size);
	void SetScale (int scale);

	void SetDrawingColor (ulong color);
	void SetDrawingOffset (CanvasPoint offset = ORIGIN);

	void FillBackground (int color, int opacity);
	void DrawLine (CanvasPoint from, CanvasPoint to);

	CanvasSize GetTextSize (const char* text);
	void DrawText (const char* text, CanvasPoint position = ORIGIN);

	int LoadBitmap (const char* path);
	CanvasSize GetBitmapSize (int bitmap);
	void DrawBitmap (int bitmap, CanvasPoint position = ORIGIN,
		CanvasRect clip = NOCLIP);
	void FreeBitmap (int bitmap);

	CanvasPoint LocationToCanvas (const cScrVec& location);
	CanvasRect ObjectToCanvas (object target);

	enum Symbol
	{
		SYMBOL_NONE,
		SYMBOL_ARROW,
		SYMBOL_CROSSHAIRS,
		SYMBOL_RETICULE
	};
	enum Direction
	{
		DIRN_NONE,
		DIRN_LEFT,
		DIRN_RIGHT,
	};
	void DrawSymbol (Symbol symbol, CanvasSize size,
		CanvasPoint position = ORIGIN, Direction direction = DIRN_NONE);
	CanvasPoint GetSymbolCenter (Symbol symbol, CanvasSize size,
		Direction direction = DIRN_NONE);
	Symbol InterpretSymbol (const char* symbol);

private:
	SService<IDarkOverlaySrv> pDOS;
	object handler;
	int handle;
	bool draw, redraw, drawing;
	CanvasPoint last_position;
	CanvasSize last_size;
	int last_scale, last_opacity;
	CanvasPoint drawing_offset;
};
#endif // !SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_QuestArrow : public virtual cScr_HUDElement
{
public:
	cScr_QuestArrow (const char* pszName, int iHostObjId);

protected:
	virtual bool Prepare ();
	virtual void Redraw ();

	virtual long OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnMessage (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual void OnPropertyChanged (const char* property);
	virtual long OnContained (sContainedScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnQuestChange (sQuestMsg* pMsg, cMultiParm& mpReply);

private:
	static const CanvasSize SYMBOL_SIZE;
	static const int PADDING;

	script_int enabled; // bool

	int objective;
	void UpdateObjective ();
	void SetEnabledFromObjective ();
	void GetTextFromObjective (cScrStr& msgstr);

	Symbol symbol;
	Direction symbol_dirn;
	int bitmap;
	CanvasPoint image_pos;
	void UpdateImage ();

	cAnsiStr text;
	CanvasPoint text_pos;
	void UpdateText ();

	ulong color;
	void UpdateColor ();
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDQuestArrow","KDHUDElement",cScr_QuestArrow)
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_ToolSight : public virtual cScr_HUDElement
{
public:
	cScr_ToolSight (const char* pszName, int iHostObjId);

protected:
	virtual bool Prepare ();
	virtual void Redraw ();

	virtual long OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual void OnPropertyChanged (const char* property);
	virtual long OnInvSelect (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnInvFocus (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnInvDeSelect (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnInvDeFocus (sScrMsg* pMsg, cMultiParm& mpReply);

private:
	static const CanvasSize SYMBOL_SIZE;

	script_int enabled; // bool

	Symbol symbol;
	int bitmap;
	void UpdateImage ();

	ulong color;
	void UpdateColor ();
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDToolSight","KDHUDElement",cScr_ToolSight)
#endif // SCR_GENSCRIPTS



#endif // CUSTOMHUD_H

