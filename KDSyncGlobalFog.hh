/******************************************************************************
 *  KDSyncGlobalFog.hh
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

#ifndef KDSYNCGLOBALFOG_HH
#define KDSYNCGLOBALFOG_HH

#include "KDTransitionTrap.hh"

class KDSyncGlobalFog : public KDTransitionTrap
{
public:
	KDSyncGlobalFog (const String& name, const Object& host);

private:
	void sync (const Color& color, float distance, bool sync_color = true,
		bool sync_distance = true);

	virtual bool increment ();

	Message::Result on_room_transit (RoomMessage&);
	Message::Result on_fog_zone_change (GenericMessage&);

	Parameter<bool> sync_fog_color, sync_fog_dist, sync_fog_disabled;
	Parameter<float> fog_dist_mult, fog_dist_add;

	Persistent<Fog::Zone> last_room_zone;
	Persistent<Color> start_color, end_color;
	Persistent<float> start_distance, end_distance;
};

#endif // KDSYNCGLOBALFOG_HH

