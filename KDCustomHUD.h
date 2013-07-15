/******************************************************************************
 *  KDCustomHUD.h: CanvasPoint, CanvasSize, CanvasRect, HUDBitmap, CustomHUD
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

#ifndef KDCUSTOMHUD_H
#define KDCUSTOMHUD_H

#if !SCR_GENSCRIPTS
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <lg/scrservices.h>
#endif // !SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
struct CanvasPoint
{
	int x, y;

	CanvasPoint (int x = 0, int y = 0);

	bool Valid () const;
	bool operator == (const CanvasPoint& rhs) const;
	bool operator != (const CanvasPoint& rhs) const;

	CanvasPoint operator + (const CanvasPoint& rhs) const;
	CanvasPoint operator - (const CanvasPoint& rhs) const;
	CanvasPoint operator * (int rhs) const;
	CanvasPoint operator / (int rhs) const;

	CanvasPoint& operator += (const CanvasPoint& rhs);
	CanvasPoint& operator -= (const CanvasPoint& rhs);

	static const CanvasPoint ORIGIN;
	static const CanvasPoint OFFSCREEN;
};
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
struct CanvasSize
{
	int w, h;

	CanvasSize (int w = 0, int h = 0);

	bool Valid () const;
	bool operator == (const CanvasSize& rhs) const;
	bool operator != (const CanvasSize& rhs) const;
};
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
struct CanvasRect : public CanvasPoint, public CanvasSize
{
	CanvasRect (int x = 0, int y = 0, int w = 0, int h = 0);
	CanvasRect (CanvasPoint position, CanvasSize size);
	explicit CanvasRect (CanvasSize size); // at ORIGIN

	bool Valid () const;
	bool operator == (const CanvasRect& rhs) const;
	bool operator != (const CanvasRect& rhs) const;

	CanvasRect operator + (const CanvasPoint& rhs) const;
	CanvasRect operator - (const CanvasPoint& rhs) const;

	static const CanvasRect NOCLIP;
	static const CanvasRect OFFSCREEN;
};
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class HUDBitmap
{
public:
	virtual ~HUDBitmap ();

	CanvasSize GetSize () const;

	std::size_t GetFrames () const;
	static const std::size_t STATIC;

protected:
	friend class CustomHUD;
	friend class HUDElement;

	HUDBitmap (const char* path, bool animation);

	void Draw (std::size_t frame, CanvasPoint position,
		CanvasRect clip = CanvasRect::NOCLIP);

private:
	static const int INVALID_HANDLE;
	std::vector<int> frames;
};
typedef std::shared_ptr<HUDBitmap> HUDBitmapPtr;
typedef std::unordered_map<std::string, std::weak_ptr<HUDBitmap>> HUDBitmaps;
#endif // SCR_GENSCRIPTS



#if !SCR_GENSCRIPTS
class HUDElement;
typedef std::list<HUDElement*> HUDElements;

class CustomHUD;
typedef std::shared_ptr<CustomHUD> CustomHUDPtr;

class CustomHUD : public IDarkOverlayHandler
{
public:
	virtual ~CustomHUD ();

	static CustomHUDPtr Get ();

	STDMETHOD_(void, DrawHUD) ();
	STDMETHOD_(void, DrawTOverlay) ();
	STDMETHOD_(void, OnUIEnterMode) ();

	bool RegisterElement (HUDElement& element);
	void UnregisterElement (HUDElement& element);

	HUDBitmapPtr LoadBitmap (const char* path, bool animation);

protected:
	CustomHUD ();

	bool LockMutex ();
	bool UnlockMutex ();

private:
	HANDLE mutex;

	HUDElements elements;
	HUDBitmaps bitmaps;
};
#endif // SCR_GENSCRIPTS



#endif // KDCUSTOMHUD_H

