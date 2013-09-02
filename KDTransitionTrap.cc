/******************************************************************************
 *  KDTransitionTrap.cc
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

#include "KDTransitionTrap.hh"

const Time
KDTransitionTrap::INCREMENT_TIME = 50ul;

KDTransitionTrap::KDTransitionTrap (const String& _name, const Object& _host)
	: TrapTrigger (_name, _host),
	  PARAMETER (transition),
	  PARAMETER (curve, Curve::LINEAR),
	  PERSISTENT_ (timer),
	  PERSISTENT_ (time_remaining)
{
	listen_timer ("Increment", &KDTransitionTrap::on_increment);
}

void
KDTransitionTrap::start ()
{
	if (timer.exists ()) // stop any previous transition
	{
		timer->cancel ();
		timer.remove ();
	}

	time_remaining = transition;
	do_increment ();
}

bool
KDTransitionTrap::incomplete () const
{
	return time_remaining.exists () && time_remaining != 0ul;
}

float
KDTransitionTrap::get_progress () const
{
	if (!time_remaining.exists ())
		return 0.0f;
	else if (transition == 0ul || time_remaining == 0ul)
		return 1.0f;
	else
	{
		float _transition = float (Time (transition)),
			_remaining = float (Time (time_remaining));
		return (_transition - _remaining) / _transition;
	}
}

bool
KDTransitionTrap::prepare (bool)
{
	// Trap behavior requires an override of this method.
	return false;
}

Message::Result
KDTransitionTrap::on_trap (bool on, Message&)
{
	if (prepare (on))
	{
		start ();
		return Message::CONTINUE;
	}
	else
		return Message::HALT;
}

Message::Result
KDTransitionTrap::on_increment (TimerMessage& message)
{
	if (message.has_data (Message::DATA1) &&
	    message.get_data<String> (Message::DATA1) == name ())
	{
		do_increment ();
		return Message::CONTINUE;
	}
	else // for a different transition hosted by this object
		return Message::HALT;
}

void
KDTransitionTrap::do_increment ()
{
	if (increment () &&
	    time_remaining.exists () && Time (time_remaining) > 0ul)
	{
		time_remaining = std::max (0l,
			long (Time (time_remaining)) - long (INCREMENT_TIME));
		timer = start_timer ("Increment", INCREMENT_TIME, false, name ());
	}
	else
	{
		timer.remove ();
		time_remaining.remove ();
	}
}

