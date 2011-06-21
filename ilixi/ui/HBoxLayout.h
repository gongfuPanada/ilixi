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

#ifndef ILIXI_HBOXLAYOUT_H_
#define ILIXI_HBOXLAYOUT_H_

#include "ui/LayoutBase.h"

namespace ilixi
{
  //! Horizontal layout
  /*!
   * In this layout widgets are aligned horizontally on a single line. Default spacing is 5 pixels.
   */
  class HBoxLayout : public LayoutBase
  {
  public:
    HBoxLayout(Widget* parent = 0);

    ~HBoxLayout();

    virtual int
    heightForWidth(int width) const;

    virtual Size
    preferredSize() const;

    void
    tile();

  private:
    virtual void
    updateActiveWidgetList();
  };
}

#endif /* ILIXI_HBOXLAYOUT_H_ */
