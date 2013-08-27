/******************************************************************************
 *  KDRenewable.cc
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

#include "KDRenewable.hh"

KDRenewable::KDRenewable (const String& _name, const Object& _host)
	: Script (_name, _host),
	  PARAMETER_ (frequency, "renewable_frequency", 180000ul),
	  PARAMETER_ (threshold, "renewable_threshold", 0),
	  PARAMETER_ (physical, "renewable_physical", false)
{
	listen_message ("PostSim", &KDRenewable::on_post_sim);
	listen_timer ("Renew", &KDRenewable::on_renew);
}

Message::Result
KDRenewable::on_post_sim (GenericMessage&)
{
	// This non-standard use of the Script->Timing property is kept for
	// backwards compatibility with miss16.osm's RenewableResource.
	Time delay = frequency;
	if (host ().script_timing.exists () && !frequency.exists ())
		delay = 1000ul * Time (host ().script_timing);

	if (delay != 0ul)
	{
		TimerMessage ("Renew").send (host (), host ());
		start_timer ("Renew", delay, true);
	}

	return Message::CONTINUE;
}

Message::Result
KDRenewable::on_renew (TimerMessage&)
{
	Player player;

	// Check for a previously created instance.
	Link previous = Link::get_one ("Owns", host ());
	if (previous != Link::NONE)
	{
		if (player.has_touched (previous.get_dest ()))
			previous.destroy ();
		else
			return Message::HALT;
	}

	// Identify the resource archetype and stack count threshold.
	Object archetype;
	size_t my_threshold = 0u;
	for (auto& script_param : ScriptParamsLink::get_all (host ()))
	{
		CIString data = String (script_param.data).data ();
		if (data == "Renewable")
		{
			archetype = script_param.get_dest ();
			my_threshold = threshold;
			break;
		}

		// For backwards compatibility, look in the link data.
		char* end = nullptr;
		my_threshold = strtoul (data.data (), &end, 10);
		if (end != data.data ())
		{
			archetype = script_param.get_dest ();
			break;
		}
	}
	if (archetype == Object::NONE)
		return Message::HALT;

	// Transmogrify the archetype for the inventory check.
	Object inv_type = archetype;
	Links transmute = Link::get_all ("Transmute", archetype, Object::ANY,
		Link::Inheritance::SOURCE);
	if (!transmute.empty ())
		inv_type = transmute.front ().get_dest ();
	else if (archetype == Object ("EarthCrystal"))
		inv_type = Object ("EarthArrow");
	else if (archetype == Object ("WaterCrystal"))
		inv_type = Object ("water");
	else if (archetype == Object ("FireCrystal"))
		inv_type = Object ("firearr");
	else if (archetype == Object ("AirCrystal"))
		inv_type = Object ("GasArrow");

	// Check the inventory count.
	size_t inv_count = 0;
	for (auto& content : player.get_inventory ())
		if (content.object.inherits_from (inv_type))
			inv_count += content.object.stack_count;
	if (inv_count >= my_threshold)
		return Message::HALT;

	// Create new instance.
	Physical instance = Object::create (archetype);
	instance.set_position (Vector (), Vector (), host ());
	Link::create ("Owns", host (), instance);

	// Remove the instance's physics, if required.
	if (!physical)
		instance.remove_physics ();

	return Message::CONTINUE;
}

