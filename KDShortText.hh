/******************************************************************************
 *  KDShortText.hh
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

#ifndef KDSHORTTEXT_HH
#define KDSHORTTEXT_HH

#include <Thief/Thief.hh>
using namespace Thief;

class KDShortText : public Script
{
public:
	KDShortText (const String& name, const Object& host);

private:
	Message::Result on_focus (Message&);
	Message::Result on_frob (FrobMessage&);

	void show_text ();

	Parameter<String> text;
	Parameter<Color> text_color;
	Parameter<Time> text_time;
	Parameter<bool> text_on_focus, text_on_frob;
};

#endif // KDSHORTTEXT_HH

