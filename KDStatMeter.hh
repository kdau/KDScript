/******************************************************************************
 *  KDStatMeter.h
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

#ifndef KDSTATMETER_H
#define KDSTATMETER_H

#if !SCR_GENSCRIPTS
#include "KDHUDElement.h"
#include "scriptvars.h"
#endif // !SCR_GENSCRIPTS

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

#endif // KDSTATMETER_H

