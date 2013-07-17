/******************************************************************************
 *  KDQuestArrow.h
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

#ifndef KDQUESTARROW_H
#define KDQUESTARROW_H

#if !SCR_GENSCRIPTS
#include "KDHUDElement.h"
#include "scriptvars.h"
#endif // !SCR_GENSCRIPTS

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

#endif // KDQUESTARROW_H

