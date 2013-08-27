/******************************************************************************
 *  KDCarrier.hh
 *
 *  Copyright (C) 2012-2013 Kevin Daughtridge <kevin@kdau.com>
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

#ifndef KDCARRIER_HH
#define KDCARRIER_HH

#include <Thief/Thief.hh>
using namespace Thief;

class KDCarrier : public Script
{
public:
	KDCarrier (const String& name, const Object& host);

private:
	virtual void initialize ();

	Message::Result on_sim (SimMessage&);
	Message::Result on_create (GenericMessage&);
	void do_create_attachments ();
	Parameter<bool> create_attachments;

	Message::Result on_ai_mode_change (AIModeChangeMessage&);
	Message::Result on_ignore_potion (GenericMessage&);
	Persistent<bool> detected_braindeath;

	Message::Result on_slain (SlayMessage&);
	Message::Result on_property_change (PropertyChangeMessage&);
	Persistent<bool> detected_slaying;

	Message::Result on_alertness (AIAlertnessMessage&);

	void notify_carried (const char* message, bool delay = false,
		int data = 0);
};

#endif // KDCARRIER_HH

