/******************************************************************************
 *  KDRoomAmbient.cc
 *
 *  Copyright (C) 2012-2014 Kevin Daughtridge <kevin@kdau.com>
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

#include "KDRoomAmbient.hh"

KDRoomAmbient::KDRoomAmbient (const String& _name, const Object& _host)
	: Script (_name, _host)
{
	listen_message ("PlayerRoomEnter", &KDRoomAmbient::on_player_enter);
	listen_message ("PropertyChange", &KDRoomAmbient::on_property_change);
}

void
KDRoomAmbient::initialize ()
{
	Script::initialize ();
	ObjectProperty::subscribe ("Ambient", host ());
}

void
KDRoomAmbient::deinitialize ()
{
	Script::deinitialize ();
	ObjectProperty::unsubscribe ("Ambient", host ());
}

Message::Result
KDRoomAmbient::on_player_enter (Message&)
{
	set_owning_room ();
	set_ambient ();
	return Message::CONTINUE;
}

Message::Result
KDRoomAmbient::on_property_change (PropertyMessage& message)
{
	// Only proceed for a change in the Room\Ambient property on this room
	// while it owns the environmental ambient and the mission is running.
	if (is_sim () && message.property == Property ("Ambient") &&
	    message.object == host () && is_owning_room ())
		set_ambient ();
	return Message::CONTINUE;
}

AmbientHacked
KDRoomAmbient::get_fnord (bool create)
{
	AmbientHacked fnord ("KDRoomAmbient");
	if (!fnord.exists () && create)
	{
		log (Log::VERBOSE, "Creating the KDRoomAmbient fnord.");
		fnord = Object::create (Object ("Marker"));
		fnord.set_name ("KDRoomAmbient");
		fnord.set_location (Vector ()); // at the origin
	}
	return fnord;
}

bool
KDRoomAmbient::is_owning_room ()
{
	auto fnord = get_fnord (false);
	return fnord.exists () && host () == ScriptParamsLink::get_one_by_data
		(fnord, "Room").get_dest ();
}

void
KDRoomAmbient::set_owning_room ()
{
	auto fnord = get_fnord ();
	for (auto& link : ScriptParamsLink::get_all_by_data (fnord, "Room"))
		link.destroy ();
	ScriptParamsLink::create (fnord, host (), "Room");
}

void
KDRoomAmbient::set_ambient ()
{
	auto fnord = get_fnord ();
	auto room = host_as<Room> ();
	String schema = room.ambient_schema;
	int volume = room.ambient_volume;

	// Only proceed if a schema was specified and something has changed.
	if (schema.empty () || (schema == String (fnord.ambient_schema [0u]) &&
	    volume == fnord.ambient_volume))
		return;

	log (Log::NORMAL, "Playing room environmental ambient %|| "
		"at volume %||.", schema, volume);

	fnord.active.remove (); // remove old AmbientHacked
	fnord.ambient_schema [0u] = schema;
	fnord.ambient_volume = volume;
	fnord.ambient_radius = 2000.0; // reaches everywhere
	fnord.environmental = true;
}

