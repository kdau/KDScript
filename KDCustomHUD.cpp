/******************************************************************************
 *  KDCustomHUD.cpp: CanvasPoint, CanvasSize, CanvasRect, HUDBitmap, CustomHUD
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

#include "KDCustomHUD.h"
#include <cstdlib>
#include <ScriptLib.h>
#include "utils.h"
#include "KDHUDElement.h"

#include <windows.h>
#undef DrawText // ugh, Windows...
#undef GetClassName // ugh, Windows...
#undef LoadBitmap // ugh, Windows...



/* CanvasPoint */

const CanvasPoint
CanvasPoint::ORIGIN = { 0, 0 };

const CanvasPoint
CanvasPoint::OFFSCREEN = { -1, -1 };

CanvasPoint::CanvasPoint (int _x, int _y)
	: x (_x), y (_y)
{}

bool
CanvasPoint::Valid () const
{
	// This does not (yet) check whether the point is within the canvas max.
	return x >= 0 && y >= 0;
}

bool
CanvasPoint::operator == (const CanvasPoint& rhs) const
{
	return x == rhs.x && y == rhs.y;
}

bool
CanvasPoint::operator != (const CanvasPoint& rhs) const
{
	return x != rhs.x || y != rhs.y;
}

CanvasPoint
CanvasPoint::operator + (const CanvasPoint& rhs) const
{
	return CanvasPoint (x + rhs.x, y + rhs.y);
}

CanvasPoint
CanvasPoint::operator - (const CanvasPoint& rhs) const
{
	return CanvasPoint (x - rhs.x, y - rhs.y);
}

CanvasPoint
CanvasPoint::operator * (int rhs) const
{
	return CanvasPoint (x * rhs, y * rhs);
}

CanvasPoint
CanvasPoint::operator / (int rhs) const
{
	return CanvasPoint (x / rhs, y / rhs);
}

CanvasPoint&
CanvasPoint::operator += (const CanvasPoint& rhs)
{
	x += rhs.x;
	y += rhs.y;
	return *this;
}

CanvasPoint&
CanvasPoint::operator -= (const CanvasPoint& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}



/* CanvasSize */

CanvasSize::CanvasSize (int _w, int _h)
	: w (_w), h (_h)
{}

bool
CanvasSize::Valid () const
{
	// An empty size still counts as valid.
	return w >= 0 && h >= 0;
}

bool
CanvasSize::operator == (const CanvasSize& rhs) const
{
	return w == rhs.w && h == rhs.h;
}

bool
CanvasSize::operator != (const CanvasSize& rhs) const
{
	return w != rhs.w || h != rhs.h;
}



/* CanvasRect */

const CanvasRect
CanvasRect::NOCLIP = { 0, 0, -1, -1 };

const CanvasRect
CanvasRect::OFFSCREEN = { -1, -1, -1, -1 };

CanvasRect::CanvasRect (int _x, int _y, int _w, int _h)
	: CanvasPoint (_x, _y), CanvasSize (_w, _h)
{}

CanvasRect::CanvasRect (CanvasPoint position, CanvasSize size)
	: CanvasPoint (position), CanvasSize (size)
{}

CanvasRect::CanvasRect (CanvasSize size)
	: CanvasPoint (), CanvasSize (size)
{}

bool
CanvasRect::Valid () const
{
	// A rect may legitimately extend off the screen. This does not (yet)
	// check that at least part of the rect is onscreen.
	return (*this == NOCLIP) || (w >= 0 && h >= 0);
}

bool
CanvasRect::operator == (const CanvasRect& rhs) const
{
	return x == rhs.x && y == rhs.y && w == rhs.w && h == rhs.h;
}

bool
CanvasRect::operator != (const CanvasRect& rhs) const
{
	return x != rhs.x || y != rhs.y || w != rhs.w || h != rhs.h;
}

CanvasRect
CanvasRect::operator + (const CanvasPoint& rhs) const
{
	return CanvasRect (x + rhs.x, y + rhs.y, w, h);
}

CanvasRect
CanvasRect::operator - (const CanvasPoint& rhs) const
{
	return CanvasRect (x - rhs.x, y - rhs.y, w, h);
}



/* HUDBitmap */

const std::size_t
HUDBitmap::STATIC = 0;

const int
HUDBitmap::INVALID_HANDLE = -1;

HUDBitmap::HUDBitmap (const char* path, bool animation)
{
	if (!path) throw std::invalid_argument ("no path specified");

	char dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT], file[_MAX_FNAME];
	_splitpath (path, NULL, dir, fname, ext);
	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);

	for (std::size_t frame = 0; frame < (animation ? 128 : 1); ++frame)
	{
		if (frame == STATIC)
			snprintf (file, _MAX_FNAME, "%s%s", fname, ext);
		else
			snprintf (file, _MAX_FNAME, "%s_%d%s",
				fname, frame, ext);

		int handle = pDOS->GetBitmap (file, dir);
		if (handle != INVALID_HANDLE)
			frames.push_back (handle);
		else
			break;
	}

	if (frames.empty ())
		throw std::runtime_error ("bitmap file invalid or not found");
}

HUDBitmap::~HUDBitmap ()
{
	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	for (auto frame : frames)
		pDOS->FlushBitmap (frame);
}

CanvasSize
HUDBitmap::GetSize () const
{
	CanvasSize result;
	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	pDOS->GetBitmapSize (frames.front (), result.w, result.h);
	return result;
}

std::size_t
HUDBitmap::GetFrames () const
{
	return frames.size ();
}

void
HUDBitmap::Draw (std::size_t frame, CanvasPoint position, CanvasRect clip)
{
	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	if (clip == CanvasRect::NOCLIP)
		pDOS->DrawBitmap (frames.at (frame), position.x, position.y);
	else
	{
		CanvasSize size = GetSize ();
		if (clip.w == CanvasRect::NOCLIP.w) clip.w = size.w - clip.x;
		if (clip.h == CanvasRect::NOCLIP.h) clip.h = size.h - clip.y;
		pDOS->DrawSubBitmap (frames.at (frame), position.x, position.y,
			clip.x, clip.y, clip.w, clip.h);
	}
}



/* CustomHUD */

CustomHUD::CustomHUD ()
	: mutex (CreateMutex (NULL, FALSE, NULL))
{
	if (mutex == NULL)
		DebugPrintf ("Error: CustomHUD couldn't create a mutex.");

	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	pDOS->SetHandler (this);
}

CustomHUD::~CustomHUD ()
{
	SService<IDarkOverlaySrv> pDOS (g_pScriptManager);
	pDOS->SetHandler (NULL);
}


CustomHUDPtr
CustomHUD::Get ()
{
	static HANDLE get_mutex = CreateMutex (NULL, FALSE, NULL);
	if (get_mutex == NULL ||
	    WaitForSingleObject (get_mutex, INFINITE) != WAIT_OBJECT_0)
	{
		DebugPrintf ("Error: CustomHUD::Get couldn't lock its mutex.");
		return CustomHUDPtr ();
	}

	static std::weak_ptr<CustomHUD> single;
	CustomHUDPtr result = single.lock ();
	if (!result)
	{
		result = CustomHUDPtr (new CustomHUD ());
		single = result;
	}

	ReleaseMutex (get_mutex);
	return result;
}

void
CustomHUD::DrawHUD ()
{
	for (auto element : elements)
		element->DrawStage1 ();
}

void
CustomHUD::DrawTOverlay ()
{
	for (auto element : elements)
		element->DrawStage2 ();
}

void
CustomHUD::OnUIEnterMode ()
{
	for (auto element : elements)
		element->EnterGameMode ();
}

bool
CustomHUD::RegisterElement (HUDElement& element)
{
	if (!LockMutex ()) return false;
	elements.push_back (&element);
	UnlockMutex ();
	return true;
}

void
CustomHUD::UnregisterElement (HUDElement& element)
{
	if (!LockMutex ()) return;
	elements.remove (&element);
	UnlockMutex ();
}

HUDBitmapPtr
CustomHUD::LoadBitmap (const char* path, bool animation)
{
	HUDBitmapPtr result;
	if (!path || !LockMutex ()) return result;

	// try an existing bitmap
	HUDBitmaps::iterator existing = bitmaps.find (path);
	if (existing != bitmaps.end ())
	{
		result = existing->second.lock ();
		if (result)
		{
			UnlockMutex ();
			return result;
		}
		else
			bitmaps.erase (existing);
	}

	try
	{
		// load a new one
		result = HUDBitmapPtr (new HUDBitmap (path, animation));
		bitmaps.insert (std::make_pair (path, result));
	}
	catch (std::exception& e)
	{
		::DebugPrintf ("Warning: Could not load bitmap at `%s': %s.",
			path, e.what ());
	}

	UnlockMutex ();
	return result;
}

bool
CustomHUD::LockMutex ()
{
	if (mutex == NULL) return false;
	bool result = WaitForSingleObject (mutex, INFINITE) == WAIT_OBJECT_0;
	if (!result)
		DebugPrintf ("Error: CustomHUD couldn't lock its mutex.");
	return result;
}

bool
CustomHUD::UnlockMutex ()
{
	return (mutex != NULL) ? ReleaseMutex (mutex) : false;
}

