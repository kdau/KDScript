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
#include <memory>
#include <vector>
#include <unordered_map>
#include <lg/interface.h>
#include <lg/scrservices.h>
#include "BaseScript.h"
#include "scriptvars.h"
#include "utils.h"
#endif // !SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
struct CanvasPoint
{
	int x, y;

	CanvasPoint (int x = 0, int y = 0);

	bool Valid () const;
	bool operator == (const CanvasPoint& rhs) const;
	bool operator != (const CanvasPoint& rhs) const;

	CanvasPoint operator + (const CanvasPoint& rhs) const;
	CanvasPoint operator - (const CanvasPoint& rhs) const;
	CanvasPoint operator * (int rhs) const;
	CanvasPoint operator / (int rhs) const;

	CanvasPoint& operator += (const CanvasPoint& rhs);
	CanvasPoint& operator -= (const CanvasPoint& rhs);

	static const CanvasPoint ORIGIN;
	static const CanvasPoint OFFSCREEN;
};
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
struct CanvasSize
{
	int w, h;

	CanvasSize (int w = 0, int h = 0);

	bool Valid () const;
	bool operator == (const CanvasSize& rhs) const;
	bool operator != (const CanvasSize& rhs) const;
};
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
struct CanvasRect : public CanvasPoint, public CanvasSize
{
	CanvasRect (int x = 0, int y = 0, int w = 0, int h = 0);
	CanvasRect (CanvasPoint position, CanvasSize size);
	explicit CanvasRect (CanvasSize size); // at ORIGIN

	bool Valid () const;
	bool operator == (const CanvasRect& rhs) const;
	bool operator != (const CanvasRect& rhs) const;

	CanvasRect operator + (const CanvasPoint& rhs) const;
	CanvasRect operator - (const CanvasPoint& rhs) const;

	static const CanvasRect NOCLIP;
	static const CanvasRect OFFSCREEN;
};
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class HUDBitmap
{
public:
	virtual ~HUDBitmap ();

	CanvasSize GetSize () const;

	std::size_t GetFrames () const;
	static const std::size_t STATIC;

protected:
	friend class CustomHUD;
	friend class HUDElement;

	HUDBitmap (const char* path, bool animation);

	void Draw (std::size_t frame, CanvasPoint position,
		CanvasRect clip = CanvasRect::NOCLIP);

private:
	static const int INVALID_HANDLE;
	std::vector<int> frames;
};
typedef std::shared_ptr<HUDBitmap> HUDBitmapPtr;
typedef std::unordered_map<std::string, std::weak_ptr<HUDBitmap>> HUDBitmaps;
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class HUDElement;
typedef std::list<HUDElement*> HUDElements;

class CustomHUD;
typedef std::shared_ptr<CustomHUD> CustomHUDPtr;

class CustomHUD : public IDarkOverlayHandler
{
public:
	virtual ~CustomHUD ();

	static CustomHUDPtr Get ();

	STDMETHOD_(void, DrawHUD) ();
	STDMETHOD_(void, DrawTOverlay) ();
	STDMETHOD_(void, OnUIEnterMode) ();

	bool RegisterElement (HUDElement& element);
	void UnregisterElement (HUDElement& element);

	HUDBitmapPtr LoadBitmap (const char* path, bool animation);

protected:
	CustomHUD ();

	bool LockMutex ();
	bool UnlockMutex ();

private:
	HANDLE mutex;

	HUDElements elements;
	HUDBitmaps bitmaps;
};
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
	virtual bool Initialize ();
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
	virtual void Redraw ();
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
	void SetScale (float scale);

	void SetDrawingColor (ulong color);
	void SetDrawingOffset (CanvasPoint offset = CanvasPoint::ORIGIN);

	void FillBackground (int color, int opacity);
	void FillArea (CanvasRect area = CanvasRect::NOCLIP);
	void DrawBox (CanvasRect area = CanvasRect::NOCLIP);
	void DrawLine (CanvasPoint from, CanvasPoint to);

	CanvasSize GetTextSize (const char* text);
	void DrawText (const char* text,
		CanvasPoint position = CanvasPoint::ORIGIN,
		bool shadowed = false);

	HUDBitmapPtr LoadBitmap (const char* path, bool animation = false);
	void DrawBitmap (HUDBitmapPtr& bitmap, std::size_t frame,
		CanvasPoint position = CanvasPoint::ORIGIN,
		CanvasRect clip = CanvasRect::NOCLIP);

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
		CanvasPoint position = CanvasPoint::ORIGIN,
		Direction direction = DIRN_NONE,
		bool shadowed = false);
	CanvasPoint GetSymbolCenter (Symbol symbol, CanvasSize size,
		Direction direction = DIRN_NONE);
	Symbol InterpretSymbol (const char* symbol, bool directional = false);

private:
	void Offset (CanvasPoint& point);
	void Offset (CanvasRect& area);

	__attribute__((format (printf,2,3)))
		void _DebugPrintf (const char* format, ...);

	SService<IDarkOverlaySrv> pDOS;
	CustomHUDPtr hud;
	object host;

	bool draw, redraw, drawing;

	int overlay;
	int opacity;

	CanvasPoint position;
	CanvasSize size;
	float scale;

	ulong drawing_color;
	CanvasPoint drawing_offset;
};
#endif // !SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_HUDElement : public virtual cBaseScript, public HUDElement
{
public:
	cScr_HUDElement (const char* pszName, int iHostObjId);

protected:
	virtual void OnAnyMessage (sScrMsg* pMsg);
	virtual long OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnMessage (sScrMsg* pMsg, cMultiParm& mpReply);

	bool SubscribeProperty (const char* property);
	virtual void OnPropertyChanged (const char* property);
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
	virtual bool Initialize ();
	virtual bool Prepare ();
	virtual void Redraw ();

	virtual long OnMessage (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual void OnPropertyChanged (const char* property);
	virtual long OnContained (sContainedScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnSlain (sSlayMsg* pMsg, cMultiParm& mpReply);
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
	HUDBitmapPtr bitmap;
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
	virtual bool Initialize ();
	virtual bool Prepare ();
	virtual void Redraw ();

	virtual long OnSim (sSimMsg* pMsg, cMultiParm& mpReply);
	virtual long OnMessage (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual void OnPropertyChanged (const char* property);

private:
	void UpdateImage ();
	void UpdateText ();
	void UpdateObject ();

	static const int MARGIN;

	script_int enabled; // bool

	enum Style
	{
		STYLE_PROGRESS,
		STYLE_UNITS,
		STYLE_GEM
	} style;

	Symbol symbol;
	HUDBitmapPtr bitmap;
	CanvasPoint meter_pos;
	int spacing;

	cAnsiStr text;
	CanvasPoint text_pos;

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

	CanvasSize request_size;

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
	virtual bool Initialize ();
	virtual bool Prepare ();
	virtual void Redraw ();

	virtual long OnInvSelect (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnInvFocus (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnInvDeSelect (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnInvDeFocus (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual void OnPropertyChanged (const char* property);

private:
	void UpdateImage ();

	static const CanvasSize SYMBOL_SIZE;

	script_int enabled; // bool

	Symbol symbol;
	HUDBitmapPtr bitmap;
	ulong color;
	CanvasPoint offset;
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDToolSight","KDHUDElement",cScr_ToolSight)
#endif // SCR_GENSCRIPTS



#endif // CUSTOMHUD_H

