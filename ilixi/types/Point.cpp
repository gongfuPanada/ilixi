/*
 Copyright 2010, 2011 Tarik Sekmen.

 All Rights Reserved.

 Written by Tarik Sekmen <tarik@ilixi.org>.

 This file is part of ilixi.

 ilixi is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 ilixi is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with ilixi.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "types/Point.h"

using namespace ilixi;

Point::Point() :
  _x(0), _y(0)
{
}

Point::Point(const Point& point) :
  _x(point._x), _y(point._y)
{
}

Point::Point(int x, int y) :
  _x(x), _y(y)
{
}

int
Point::x() const
{
  return _x;
}

int
Point::y() const
{
  return _y;
}

bool
Point::isNull() const
{
  if (_x == 0 && _y == 0)
    return true;
  return false;
}

void
Point::moveTo(int x, int y)
{
  _x = x;
  _y = y;
}

void
Point::moveTo(const Point &point)
{
  *this = point;
}

void
Point::translate(int x, int y)
{
  _x += x;
  _y += y;
}

void
Point::setX(int x)
{
  _x = x;
}

void
Point::setY(int y)
{
  _y = y;
}

Point&
Point::operator=(const Point &point)
{
  if (this != &point)
    {
      _x = point._x;
      _y = point._y;
    }
  return *this;
}

Point&
Point::operator +=(const Point &point)
{
  _x += point._x;
  _y += point._y;
  return *this;
}

Point&
Point::operator-=(const Point &point)
{
  _x -= point._x;
  _y -= point._y;
  return *this;
}

Point&
Point::operator*=(float factor)
{
  _x *= factor;
  _y *= factor;
  return *this;
}

Point&
Point::operator/=(float factor)
{
  _x /= factor;
  _y /= factor;
  return *this;
}

bool
Point::operator==(const Point &point) const
{
  return ((_x == point._x) && (_y == point._y));
}

bool
Point::operator!=(const Point &point) const
{
  return !(*this == point);
}
