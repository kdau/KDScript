/******************************************************************************
 *  Other.h: miscellaneous useful scripts
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

#ifndef OTHER_H
#define OTHER_H

#if !SCR_GENSCRIPTS
#include "BaseScript.h"
#endif // SCR_GENSCRIPTS



/**
 * Script: KDShortText
 * Inherits: BaseScript
 * Property: Book\Text - The name of the message in short.str to display.
 * Parameter: text (string) - The name of the message from short.str to display.
 * Parameter: color (color) - The color to display the message in.
 * Parameter: time (time) - How long to display the message, in milliseconds.
 * Parameter: on_frob (bool) - Whether to display the message when frobbed.
 * Parameter: on_focus (bool) - Whether to display the message when highlighted.
 *
 * When triggered, displays text in an on-screen message of the specified color
 * for the specified duration. The Book\Text property, if present, gives the
 * name of a message in the strings\short.str file. (If not, the text parameter
 * is consulted instead.) This allows for the cleaner updating and translation
 * of short readables without requiring a separate .str file for each.
 *
 * If the color parameter is not specified, the message is displayed in white.
 * If the time parameter is not specified, the message is displayed for a time
 * calculated based on its length.
 *
 * If on_frob is true or not specified, and the WorldAction of the object's
 * FrobInfo includes Script, the message will be displayed when the object is
 * frobbed. If on_focus is true or not specified, and the WorldAction of the
 * object's FrobInfo includes FocusScript, the message will be displayed when
 * the object is highlighted (focused/pointed to).
 */
#if !SCR_GENSCRIPTS
class cScr_ShortText : public virtual cBaseScript
{
public:
	cScr_ShortText (const char* pszName, int iHostObjId);

protected:
	virtual long OnFrobWorldEnd (sFrobMsg* pMsg, cMultiParm& mpReply);
	virtual long OnWorldSelect (sScrMsg* pMsg, cMultiParm& mpReply);

private:
	void DisplayMessage ();
};
#else // SCR_GENSCRIPTS
GEN_FACTORY("KDShortText","BaseScript",cScr_ShortText)
#endif // SCR_GENSCRIPTS



#endif // OTHER_H

