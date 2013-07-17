/******************************************************************************
 *  KDToolSight.h
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

#ifndef KDTOOLSIGHT_H
#define KDTOOLSIGHT_H

#if !SCR_GENSCRIPTS
#include "KDHUDElement.h"
#endif // !SCR_GENSCRIPTS

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

	bool selected;

	Symbol symbol;
	HUDBitmapPtr bitmap;
	ulong color;
	CanvasPoint offset;
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDToolSight","KDHUDElement",cScr_ToolSight)
#endif // SCR_GENSCRIPTS

#endif // KDTOOLSIGHT_H

