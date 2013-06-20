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
#include <memory>
#include <vector>
#include <lg/interface.h>
#include <lg/scrservices.h>
#include "BaseScript.h"
#include "scriptvars.h"
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class HUDElement
{
public:
	HUDElement (object host, unsigned number);
	HUDElement (const HUDElement&) = delete;
	virtual ~HUDElement () {}

	void Draw ();
	void UpdatePosition ();

	enum PlayMode
	{
		MODE_NONE = 0,
		MODE_NORMAL = 1,
		MODE_WEAPON = 2,
		MODE_CAMERA = 4,
		MODE_TRACK_OFFSCREEN = 8,
		MODE_DEFAULT = MODE_NORMAL | MODE_WEAPON,
		MODE_ALL = MODE_NORMAL | MODE_WEAPON | MODE_CAMERA |
			MODE_TRACK_OFFSCREEN
	};
	void OnModesChanged (PlayMode play_modes);

private:
	void InitializeParameters ();

	object host;
	unsigned number;

	SService<IDarkOverlaySrv> pDOS;
	int handle;

	// configuration

	enum PositionType
	{
		POS_NONE,
		POS_NW,
		POS_N,
		POS_NE,
		POS_E,
		POS_SE,
		POS_S,
		POS_SW,
		POS_W,
		POS_CENTER,
		POS_TRACK
	} position_type;

	static constexpr int EDGE_OFFSET = 20;
	int offset_x, offset_y;

	int width, height;

	long color_fg, color_bg, color_border;

	std::string image;
	int image_width, image_height;

	enum Symbol
	{
		SYM_NONE,
		SYM_ARROW,
		SYM_CROSS
	} symbol;

	std::string text;

	int play_modes;

	unsigned opacity;

	// current state

	int position_x, position_y;
	int current_play_modes;
};
typedef std::shared_ptr<HUDElement> HUDElementPtr;
typedef std::vector<HUDElementPtr> HUDElements;
#endif // !SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class HUDHandler : public IDarkOverlayHandler
{
public:
	HUDHandler (object host);
	HUDHandler (const HUDHandler&) = delete;
	virtual ~HUDHandler () {}

	STDMETHOD_(void, DrawHUD) () {}
	STDMETHOD_(void, DrawTOverlay) ();
	STDMETHOD_(void, OnUIEnterMode) ();
	//FIXME Detect play mode changes and notify elements.

private:

	object host;
	HUDElements elements;
};
#endif // !SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class cScr_CustomHUD : public virtual cBaseScript
{
public:
	cScr_CustomHUD (const char* pszName, int iHostObjId);

protected:
	virtual long OnBeginScript (sScrMsg* pMsg, cMultiParm& mpReply);
	virtual long OnEndScript (sScrMsg* pMsg, cMultiParm& mpReply);

private:
	std::shared_ptr<HUDHandler> handler;
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDCustomHUD","BaseScript",cScr_CustomHUD)
#endif // SCR_GENSCRIPTS



#endif // CUSTOMHUD_H

