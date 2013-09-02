/******************************************************************************
 *  KDShortText.cc
 *
 *  Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
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

#include "KDShortText.hh"

KDShortText::KDShortText (const String& _name, const Object& _host)
	: Script (_name, _host),
	  PARAMETER (text),
	  PARAMETER (text_color, Color (255, 255, 255)),
	  PARAMETER (text_time),
	  PARAMETER (text_on_focus, true),
	  PARAMETER (text_on_frob, true)
{
	listen_message ("WorldSelect", &KDShortText::on_focus);
	listen_message ("FrobWorldEnd", &KDShortText::on_frob);
}

Message::Result
KDShortText::on_focus (GenericMessage&)
{
	if (text_on_focus)
		show_text ();
	return Message::CONTINUE;
}

Message::Result
KDShortText::on_frob (FrobMessage&)
{
	if (text_on_frob)
		show_text ();
	return Message::CONTINUE;
}

void
KDShortText::show_text ()
{
	String msgid = host_as<Readable> ().book_name;
	if (msgid.empty ()) msgid = text;

	String msgstr = Mission::get_text ("strings", "short", msgid);
	if (!msgstr.empty ())
		Mission::show_text (msgstr, text_time, text_color);
}

