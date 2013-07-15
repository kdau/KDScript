/******************************************************************************
 *  KDHUDElement.h: HUDElement, KDHUDElement
 *
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
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

#ifndef KDHUDELEMENT_H
#define KDHUDELEMENT_H

#if !SCR_GENSCRIPTS
#include <lg/interface.h>
#include <lg/scrservices.h>
#include "BaseScript.h"
#include "KDCustomHUD.h"
#endif // !SCR_GENSCRIPTS



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



#endif // KDHUDELEMENT_H

