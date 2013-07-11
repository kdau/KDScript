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
#include <vector>
#include <lg/interface.h>
#include <lg/scrservices.h>
#include "BaseScript.h"
#include "scriptvars.h"
#include "utils.h"
#endif // !SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
struct CanvasSize;

struct CanvasPoint
{
	int x, y;
	CanvasPoint (int x = 0, int y = 0);
	explicit CanvasPoint (const CanvasSize& area);
	bool operator == (const CanvasPoint& rhs) const;
	bool operator != (const CanvasPoint& rhs) const;
	CanvasPoint operator + (const CanvasPoint& rhs) const;
	CanvasPoint operator - (const CanvasPoint& rhs) const;
	CanvasPoint operator * (int rhs) const;
	CanvasPoint operator / (int rhs) const;
};
extern const CanvasPoint ORIGIN;
extern const CanvasPoint OFFSCREEN;

struct CanvasSize
{
	int w, h;
	CanvasSize (int w = 0, int h = 0);
	bool operator == (const CanvasSize& rhs) const;
	bool operator != (const CanvasSize& rhs) const;
};

struct CanvasRect
{
	int x, y, w, h;
	CanvasRect (int x = 0, int y = 0, int w = 0, int h = 0);
	bool operator == (const CanvasRect& rhs) const;
	bool operator != (const CanvasRect& rhs) const;
	CanvasRect operator + (const CanvasPoint& rhs) const;
	CanvasRect operator - (const CanvasPoint& rhs) const;
};
extern const CanvasRect NOCLIP;
extern const CanvasRect OFFSCREEN_R;

#endif // !SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class HUDElement;
typedef std::list<HUDElement*> HUDElements;

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
class HUDElement
{
public:
	HUDElement (object host);
	virtual ~HUDElement ();

	void DrawStage1 ();
	void DrawStage2 ();
	virtual void EnterGameMode ();

protected:
	bool Initialize ();
	void Deinitialize ();
	object GetHost ();

	bool GetParamBool (const char* param, bool default_value);
	ulong GetParamColor (const char* param, ulong default_value);
	float GetParamFloat (const char* param, float default_value);
	int GetParamInt (const char* param, int default_value);
	object GetParamObject (const char* param, object default_value);
	cAnsiStr GetParamString (const char* param, const char* default_value);

	void SetParamBool (const char* param, bool value);
	// no support for setting a color parameter
	void SetParamFloat (const char* param, float value);
	void SetParamInt (const char* param, int value);
	// no support for setting an object parameter
	void SetParamString (const char* param, const char* value);

	virtual bool Prepare ();
	virtual void Redraw () = 0;
	bool NeedsRedraw ();
	void ScheduleRedraw ();

	bool IsOverlay ();
	bool CreateOverlay ();
	void DestroyOverlay ();
	void SetOpacity (int opacity);

	CanvasSize GetCanvasSize ();
	void SetPosition (CanvasPoint position);
	CanvasSize GetSize ();
	void SetSize (CanvasSize size);
	void SetScale (int scale);

	void SetDrawingColor (ulong color);
	void SetDrawingOffset (CanvasPoint offset = ORIGIN);

	void FillBackground (int color, int opacity);
	void FillArea (CanvasRect area = NOCLIP);
	void DrawBox (CanvasRect area = NOCLIP);
	void DrawLine (CanvasPoint from, CanvasPoint to);

	CanvasSize GetTextSize (const char* text);
	void DrawText (const char* text, CanvasPoint position = ORIGIN,
		bool shadowed = false);

	enum { INVALID_BITMAP = -1 };
	bool IsValidBitmap (int bitmap);
	int LoadBitmap (const char* path);
	void LoadBitmaps (const char* path, std::vector<int>& bitmaps);
	CanvasSize GetBitmapSize (int bitmap);
	void DrawBitmap (int bitmap, CanvasPoint position = ORIGIN,
		CanvasRect clip = NOCLIP);
	void FreeBitmap (int bitmap);

	CanvasPoint LocationToCanvas (const cScrVec& location);
	CanvasRect ObjectToCanvas (object target);
	CanvasPoint ObjectCentroidToCanvas (object target);

	enum Symbol
	{
		SYMBOL_NONE,
		SYMBOL_ARROW,
		SYMBOL_CROSSHAIRS,
		SYMBOL_RETICULE,
		SYMBOL_SQUARE
	};
	enum Direction
	{
		DIRN_NONE,
		DIRN_LEFT,
		DIRN_RIGHT,
	};
	void DrawSymbol (Symbol symbol, CanvasSize size,
		CanvasPoint position = ORIGIN, Direction direction = DIRN_NONE,
		bool shadowed = false);
	CanvasPoint GetSymbolCenter (Symbol symbol, CanvasSize size,
		Direction direction = DIRN_NONE);
	Symbol InterpretSymbol (const char* symbol, bool directional = false);

private:
	SService<IDarkOverlaySrv> pDOS;
	object host, handler;
	int overlay;
	bool draw, redraw, drawing;
	CanvasPoint last_position;
	CanvasSize last_size;
	int last_scale, last_opacity;
	ulong last_color;
	CanvasPoint drawing_offset;
};
#endif // !SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_HUDElement : public virtual cBaseScript, public HUDElement
{
public:
	cScr_HUDElement (const char* pszName, int iHostObjId);

protected:
	virtual long OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnMessage (sScrMsg* pMsg, cMultiParm& mpReply);

	bool SubscribeProperty (const char* property);
	virtual void OnPropertyChanged (const char* property);

	virtual void Redraw ();
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDHUDElement","BaseScript",cScr_HUDElement)
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_QuestArrow : public cScr_HUDElement
{
public:
	cScr_QuestArrow (const char* pszName, int iHostObjId);

protected:
	virtual bool Prepare ();
	virtual void Redraw ();

	virtual long OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnMessage (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual void OnPropertyChanged (const char* property);
	virtual long OnContained (sContainedScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnQuestChange (sQuestMsg* pMsg, cMultiParm& mpReply);

private:
	static const CanvasSize SYMBOL_SIZE;
	static const int PADDING;

	script_int enabled; // bool
	bool obscured;

	enum { OBJECTIVE_NONE = -1 };
	int objective;
	void UpdateObjective ();
	void SetEnabledFromObjective ();
#if (_DARKGAME == 2)
	void GetTextFromObjective (cScrStr& msgstr);
#endif

	Symbol symbol;
	Direction symbol_dirn;
	int bitmap;
	CanvasPoint image_pos;
	void UpdateImage ();

	cAnsiStr text;
	CanvasPoint text_pos;
	void UpdateText ();

	ulong color;
	bool shadow;
	void UpdateColor ();
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDQuestArrow","KDHUDElement",cScr_QuestArrow)
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_StatMeter : public cScr_HUDElement
{
public:
	cScr_StatMeter (const char* pszName, int iHostObjId);

protected:
	virtual bool Prepare ();
	virtual void Redraw ();

	virtual long OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnSim (sSimMsg* pMsg, cMultiParm& mpReply);
	virtual long OnMessage (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual void OnPropertyChanged (const char* property);

private:
	void UpdateImage ();
	void FreeBitmaps ();

	void UpdateObject ();

	static const int MARGIN;

	script_int enabled; // bool

	enum Style
	{
		STYLE_PROGRESS,
		STYLE_UNITS,
		STYLE_GEM
	} style;

	enum Position
	{
		POS_CENTER,
		POS_NORTH,
		POS_NE,
		POS_EAST,
		POS_SE,
		POS_SOUTH,
		POS_SW,
		POS_WEST,
		POS_NW
	} position;
	CanvasPoint offset;

	enum Orient
	{
		ORIENT_HORIZ,
		ORIENT_VERT
	} orient;

	Symbol symbol;
	std::vector<int> bitmaps;

	CanvasSize size;
	int spacing;

	cAnsiStr qvar, prop_name, prop_field;
	enum Component { COMP_NONE, COMP_X, COMP_Y, COMP_Z } prop_comp;

	object prop_object;
	bool post_sim_fix;

	float min, max; int low, high;

	ulong color_bg, color_low, color_med, color_high;

	float value, value_pct;
	int value_int;
	enum ValueTier
	{
		VALUE_LOW,
		VALUE_MED,
		VALUE_HIGH
	} value_tier;
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDStatMeter","KDHUDElement",cScr_StatMeter)
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_ToolSight : public cScr_HUDElement
{
public:
	cScr_ToolSight (const char* pszName, int iHostObjId);

protected:
	virtual bool Prepare ();
	virtual void Redraw ();

	virtual long OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply);
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

	CanvasPoint offset;
	void UpdateOffset ();
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDToolSight","KDHUDElement",cScr_ToolSight)
#endif // SCR_GENSCRIPTS



#endif // CUSTOMHUD_H

