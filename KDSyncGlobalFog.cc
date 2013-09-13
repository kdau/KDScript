/******************************************************************************
 *  KDSyncGlobalFog.cc
 *
 *  Copyright (C) 2013 Kevin Daughtridge <kevin@kdau.com>
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

#include "KDSyncGlobalFog.hh"

KDSyncGlobalFog::KDSyncGlobalFog (const String& _name, const Object& _host)
	: Script (_name, _host),
	  transition (*this, &KDSyncGlobalFog::step, "SyncGlobalFog"),
	  PARAMETER (sync_fog_color, true),
	  PARAMETER (sync_fog_dist, true),
	  PARAMETER (sync_fog_disabled, false),
	  PARAMETER (fog_dist_mult, 1.0f),
	  PARAMETER (fog_dist_add, 0.0f),
	  PERSISTENT_ (last_room_zone),
	  PERSISTENT_ (start_color),
	  PERSISTENT_ (end_color),
	  PERSISTENT_ (start_distance),
	  PERSISTENT_ (end_distance)
{
	listen_message ("ObjRoomTransit", &KDSyncGlobalFog::on_room_transit);
	listen_message ("FogZoneChange", &KDSyncGlobalFog::on_fog_zone_change);
}

void
KDSyncGlobalFog::sync (const Color& color, float distance, bool sync_color,
	bool sync_distance)
{
	Fog global = Mission::get_fog (Fog::GLOBAL);

	if (distance > 0.0f)
		distance = distance * fog_dist_mult + fog_dist_add;

	sync_color = sync_color && sync_fog_color;
	sync_distance = (sync_distance && sync_fog_dist && distance >= 0.0f)
		|| global.distance == 0.0f || distance == 0.0f;

	start_color = global.color;
	end_color = sync_color ? color : global.color;

	start_distance = global.distance;
	end_distance = sync_distance ? distance : global.distance;

	log (Log::NORMAL, "Synchronizing global fog to color %|| at distance "
		"%|| over %|| ms.", end_color, end_distance, transition.length);
	transition.start ();
}

bool
KDSyncGlobalFog::step ()
{
	Mission::set_fog (Fog::GLOBAL, {
		transition.interpolate (start_color, end_color),
		Fog::interpolate_distance (true, start_distance, end_distance,
			transition.get_progress (), transition.curve)
	});
	return true;
}

Message::Result
KDSyncGlobalFog::on_room_transit (RoomMessage& message)
{
	if (message.object_type != RoomMessage::PLAYER)
		return Message::HALT; // The starting point is still our host.
	if (message.to_room == Object::NONE)
		return Message::HALT; // This is not a valid transit message.

	Fog::Zone new_zone = message.to_room.fog_zone;
	if (last_room_zone.exists () && last_room_zone == new_zone)
		return Message::HALT; // no change

	Color color;
	float distance = 0.0f;
	bool sync_color;

	if (new_zone == Fog::DISABLED)
	{
		if (!sync_fog_disabled)
			return Message::HALT;
		sync_color = false;
	}
	else if (new_zone > Fog::GLOBAL && new_zone <= Fog::_MAX_ZONE)
	{
		Fog zone_fog = Mission::get_fog (new_zone);
		color = zone_fog.color;
		distance = zone_fog.distance;
		sync_color = true;
	}
	else
		return Message::HALT; // invalid zone

	last_room_zone = new_zone;
	sync (color, distance, sync_color);
	return Message::HALT;
}

Message::Result
KDSyncGlobalFog::on_fog_zone_change (Message& message)
{
	if (!message.has_data (Message::DATA1) ||
	    !message.has_data (Message::DATA2) ||
	    !message.has_data (Message::DATA3))
		return Message::ERROR;

	Fog::Zone changed_zone = message.get_data<Fog::Zone> (Message::DATA1);
	Color color = message.get_data<Color> (Message::DATA2);
	float distance = message.get_data<float> (Message::DATA3);

	if (last_room_zone.exists () && last_room_zone == changed_zone)
		sync (color, distance);

	return Message::HALT;
}

