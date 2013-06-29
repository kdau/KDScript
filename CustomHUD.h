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
#include <darkhook.h>
#include "BaseScript.h"
#include "scriptvars.h"
#endif // SCR_GENSCRIPTS



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
	virtual void CanvasChanged ();

protected:
	virtual long OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply);

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

	void SetOpacity (int opacity);

	void GetCanvasSize (int& width, int& height);
	void SetPosition (int x, int y);
	void SetSize (int width, int height);

	void FillBackground (int color, int opacity);
	void SetDrawingColor (ulong color);
	void DrawLine (int x1, int y1, int x2, int y2);

	void GetTextSize (const char* text, int& width, int& height);
	void DrawText (const char* text, int x, int y);

	int LoadBitmap (const char* path);
	void GetBitmapSize (int bitmap, int& width, int& height);
	void DrawBitmap (int bitmap, int x, int y, int src_x = 0, int src_y = 0,
		int src_width = -1, int src_height = -1);
	void FreeBitmap (int bitmap);

	bool LocationToScreen (const cScrVec& location, int& x, int& y);
	bool ObjectToScreen (object target, int& x1, int& y1, int& x2, int& y2);

private:
	SService<IDarkOverlaySrv> pDOS;
	object handler;
	int handle;
	bool draw, redraw, redrawing;
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
	virtual long OnDHNotify (sDHNotifyMsg* pMsg, cMultiParm& mpReply);
	virtual long OnContained (sContainedScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnQuestChange (sQuestMsg* pMsg, cMultiParm& mpReply);

private:
	script_int enabled; // bool

	int objective;
	void UpdateObjective ();
	void SetEnabledFromObjective ();
	void GetTextFromObjective (cScrStr& msgstr);

	enum ImageType
	{
		IMAGE_NONE,
		IMAGE_BITMAP,
		IMAGE_ARROW,
		IMAGE_CROSSHAIRS
	} image;
	int bitmap;
	static const int SYMBOL_SIZE;
	void UpdateImage ();

	cAnsiStr text;
	void UpdateText ();

	ulong color;
	void UpdateColor ();

	void UpdateOpacity ();
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDQuestArrow","KDHUDElement",cScr_QuestArrow)
#endif // SCR_GENSCRIPTS



#endif // CUSTOMHUD_H

