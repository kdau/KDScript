/******************************************************************************
 *  CustomHUD.cpp: script to draw custom HUD elements
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

#include "CustomHUD.h"
#include <lg/objstd.h>
#include <ScriptLib.h>
#include "ScriptModule.h"
#include "utils.h"



/* HUDElement */


HUDElement::HUDElement (object _host, unsigned _number)
	: host (_host), number (_number), pDOS (g_pScriptManager), handle (-1),
	  position_type (POS_NONE), offset_x (0), offset_y (0), width (-1),
	  height (-1), color_fg (0x00ffffff), color_bg (-1), color_border (-1),
	  image (), image_width (-1), image_height (-1), symbol (SYM_NONE),
	  text (), play_modes (MODE_DEFAULT), opacity (255), position_x (0),
	  position_y (0), current_play_modes (MODE_NORMAL)
{
	if (number > 63) throw std::invalid_argument ("bad element number");

	InitializeParameters ();
	UpdatePosition ();
	//FIXME Do any other needed setup.

	handle = pDOS->CreateTOverlayItem (position_x, position_y,
		width, height, opacity, true);
	if (handle == -1)
		throw std::runtime_error ("could not allocate overlay");

	//FIXME Schedule redraws as needed.
}


void
HUDElement::Draw ()
{
	//FIXME Redraw as needed.

	if (handle != -1)
		pDOS->DrawTOverlayItem (handle);
}

void
HUDElement::UpdatePosition ()
{
	SService<IEngineSrv> pES (g_pScriptManager);
	int canvas_width = 0, canvas_height = 0;
	pES->GetCanvasSize (canvas_width, canvas_height);

	switch (position_type)
	{
	case POS_NW:
	case POS_W:
	case POS_SW:
		position_x = 0;
		break;
	case POS_N:
	case POS_CENTER:
	case POS_S:
		position_x = (canvas_width - width) / 2;
		break;
	case POS_NE:
	case POS_E:
	case POS_SE:
		position_x = canvas_width - width;
		break;
	default:
		break; // position_x will be handled below
	}

	switch (position_type)
	{
	case POS_NW:
	case POS_N:
	case POS_NE:
		position_y = 0;
		break;
	case POS_W:
	case POS_CENTER:
	case POS_E:
		position_y = (canvas_height - height) / 2;
		break;
	case POS_SW:
	case POS_S:
	case POS_SE:
		position_y = canvas_height - height;
		break;
	case POS_TRACK:
		//FIXME Calculate and break. This needs more frequent calculation.
	case POS_NONE:
	default:
		position_x = position_y = 0;
		break;
	}

	position_x += offset_x;
	position_y += offset_y;

	if (handle != -1)
		pDOS->UpdateTOverlayPosition (handle, position_x, position_y);
}

void
HUDElement::OnModesChanged (PlayMode play_modes)
{
	current_play_modes = play_modes;
	//FIXME Enable/disable/redraw as needed.
}

void
HUDElement::InitializeParameters ()
{
	SService<IQuestSrv> pQS (g_pScriptManager);
	char param_name[128] = { 0 };
#define set_param_name(name) snprintf (param_name, 128, "hud%02u_" name, number)

	set_param_name ("position");
	char* _pos_type = GetObjectParamString (host, param_name, NULL);
	if (!_pos_type)
		throw std::runtime_error ("no element for this number");
	else if (!stricmp (_pos_type, "nw"))
	{
		position_type = POS_NW;
		offset_x = offset_y = EDGE_OFFSET;
	}
	else if (!stricmp (_pos_type, "n"))
	{	position_type = POS_N;
		offset_y = EDGE_OFFSET;
	}
	else if (!stricmp (_pos_type, "ne"))
	{
		position_type = POS_NE;
		offset_x = -EDGE_OFFSET; offset_y = EDGE_OFFSET;
	}
	else if (!stricmp (_pos_type, "e"))
	{
		position_type = POS_E;
		offset_x = -EDGE_OFFSET;
	}
	else if (!stricmp (_pos_type, "se"))
	{
		position_type = POS_SE;
		offset_x = offset_y = -EDGE_OFFSET;
	}
	else if (!stricmp (_pos_type, "s"))
	{
		position_type = POS_S;
		offset_y = -EDGE_OFFSET;
	}
	else if (!stricmp (_pos_type, "sw"))
	{
		position_type = POS_SW;
		offset_x = EDGE_OFFSET; offset_y = -EDGE_OFFSET;
	}
	else if (!stricmp (_pos_type, "w"))
	{
		position_type = POS_W;
		offset_x = EDGE_OFFSET;
	}
	else if (!stricmp (_pos_type, "center"))
		position_type = POS_CENTER;
	else if (!stricmp (_pos_type, "track"))
		position_type = POS_TRACK;
	else
		throw std::runtime_error ("bad element position type");
	g_pMalloc->Free (_pos_type);

	set_param_name ("offset_x");
	offset_x = GetObjectParamInt (host, param_name, offset_x);

	set_param_name ("offset_y");
	offset_y = GetObjectParamInt (host, param_name, offset_y);

	set_param_name ("width");
	int _width = GetObjectParamInt (host, param_name, -1);
	if (_width > 0) width = _width;

	set_param_name ("height");
	int _height = GetObjectParamInt (host, param_name, -1);
	if (_height > 0) height = _height;

	set_param_name ("color_fg");
	color_fg = GetObjectParamColor (host, param_name, color_fg);

	set_param_name ("color_bg");
	color_bg = GetObjectParamInt (host, param_name, color_bg);

	set_param_name ("color_border");
	if (char* _color_border = GetObjectParamString (host, param_name, NULL))
	{
		color_border = strtocolor (_color_border);
		g_pMalloc->Free (_color_border);
	}

	set_param_name ("image");
	if (char* _image = GetObjectParamString (host, param_name, NULL))
	{
		image = _image;
		g_pMalloc->Free (_image);

		//FIXME Interpret and prepare.
	}

	set_param_name ("symbol");
	char* _symbol = GetObjectParamString (host, param_name, NULL);
	if (_symbol)
	{
		if (!stricmp (_symbol, "none"))
			symbol = SYM_NONE;
		else if (!stricmp (_symbol, "arrow"))
			symbol = SYM_ARROW;
		else if (!stricmp (_symbol, "cross"))
			symbol = SYM_CROSS;
		else
			g_pfnMPrintf ("CustomHUD::Element [%d.%d]: ignoring invalid element symbol `%s'",
				host, number, _symbol);
		g_pMalloc->Free (_symbol);
	}

	set_param_name ("text");
	if (char* _text = GetObjectParamString (host, param_name, NULL))
	{
		text = _text;
		g_pMalloc->Free (_text);

		//FIXME Interpret and prepare.
	}

	set_param_name ("initial_state");
	int initial_state = GetObjectParamInt (host, param_name, 1);
	if (initial_state >= 0 && initial_state <= 2)
	{
		set_param_name ("state");
		pQS->Set (param_name, initial_state, kQuestDataMission);
	}

	set_param_name ("play_modes");
	int _play_modes = GetObjectParamInt (host, param_name, -1);
	if (_play_modes >= MODE_NONE && _play_modes <= MODE_ALL)
		play_modes = _play_modes;

	set_param_name ("opacity");
	int _opacity = GetObjectParamInt (host, param_name, -1);
	if (_opacity >= 0 && _opacity <= 255)
		opacity = _opacity;
}



/* HUDHandler */

HUDHandler::HUDHandler (object _host)
	: host (_host)
{
	for (unsigned number = 0; number < 64; ++number)
		try
		{
			elements.push_back
				(std::make_shared<HUDElement> (host, number));
		}
		catch (...) {} //FIXME Log?
}

void
HUDHandler::DrawTOverlay ()
{
	for (auto element : elements)
		element->Draw ();
}

void
HUDHandler::OnUIEnterMode ()
{
	for (auto element : elements)
		element->UpdatePosition ();
}



/* CustomHUD */

cScr_CustomHUD::cScr_CustomHUD (const char* pszName, int iHostObjId)
	: cBaseScript (pszName, iHostObjId), handler ()
{}

long
cScr_CustomHUD::OnBeginScript (sScrMsg*, cMultiParm&)
{
	handler = std::make_shared<HUDHandler> (ObjId ());

	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	pDOS->SetHandler (handler.get ());

	return 0;
}

long
cScr_CustomHUD::OnEndScript (sScrMsg*, cMultiParm&)
{
	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	pDOS->SetHandler (NULL);

	handler.reset ();

	return 0;
}

