/******************************************************************************
 *  KDTransitionTrap.hh
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

#ifndef KDTRANSITIONTRAP_HH
#define KDTRANSITIONTRAP_HH

#include <Thief/Thief.hh>
using namespace Thief;

class KDTransitionTrap : public TrapTrigger
{
protected:
	KDTransitionTrap (const String& name, const Object& host);

	void start ();

	bool incomplete () const;
	float get_progress () const;

	template <typename T> THIEF_INTERPOLATE_RESULT (T)
	interpolate (const T& from, const T& to) const;

	template <typename T>
	T interpolate (const Persistent<T>& from, const Persistent<T>& to) const;

	Parameter<Time> transition;
	Parameter<Curve> curve;

private:
	virtual bool prepare (bool on);
	virtual bool increment () = 0;

	virtual Message::Result on_trap (bool on, Message&);
	Message::Result on_increment (TimerMessage&);
	void do_increment ();

	static const Time INCREMENT_TIME;

	Persistent<Timer> timer;
	Persistent<Time> time_remaining;
};

template <typename T>
inline THIEF_INTERPOLATE_RESULT (T)
KDTransitionTrap::interpolate (const T& from, const T& to) const
{
	return Thief::interpolate (from, to, get_progress (), curve);
}

template <typename T>
inline T
KDTransitionTrap::interpolate (const Persistent<T>& from,
	const Persistent<T>& to) const
{
	return Thief::interpolate (T (from), T (to), get_progress (), curve);
}

#endif // KDTRANSITIONTRAP_HH

