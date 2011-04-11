/*
 Copyright 2010, 2011 Tarik Sekmen.

 All Rights Reserved.

 Written by Tarik Sekmen <tarik@ilixi.org>.

 ilixi is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 ilixi is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GridLayout.h"
#include "ui/RadioButton.h"
#include "core/Logger.h"

using namespace ilixi;

GridLayout::GridLayout(unsigned int rows, unsigned int column, Widget* parent) :
  LayoutBase(parent), _rows(rows), _cols(column)
{
  _cells.assign(_rows * _cols, (CellData*) NULL);
  _colWidths.assign(_cols, 0);
  _rowHeights.assign(_rows, 0);
}

GridLayout::~GridLayout()
{
  for (unsigned int c = 0; c < _cols; c++)
    {
      for (unsigned int r = 0; r < _rows; r++)
        {
          int index = r * _cols + c;

          if (_cells[index] == NULL)
            continue;

          else if (_cells[index]->lastRow == r && _cells[index]->lastCol == c)
            {
              delete _cells[index];
              _cells[index] = NULL;
            }
        }
    }
}

int
GridLayout::heightForWidth(int width) const
{
  return -1;
}

Size
GridLayout::preferredSize() const
{
  return Size();

  //  Widget* widget;
  //  int index = 0;
  //
  //  for (int i = 0; i < _rows * _cols; i++)
  //    {
  //      if (!_cells[i])
  //        continue;
  //      _cells[i]->ignored = false;
  //      _cells[i]->width = -2;
  //      _cells[i]->height = -2;
  //      _cells[i]->h4w = -2;
  //    }
  //
  //  std::vector<int> colW, rowH;
  //  colW.assign(_cols, 0);
  //  rowH.assign(_rows, 0);
  //
  //  for (int c = 0; c < _cols; ++c)
  //    {
  //      for (int r = 0; r < _rows; ++r)
  //        {
  //          index = r * _cols + c;
  //
  //          // request preferred size from the widget only once.
  //          if (_cells[index]->width == -2)
  //            {
  //              Size s = widget->preferredSize();
  //              _cells[index]->width = s.width();
  //              _cells[index]->height = s.height();
  //            }
  //
  //          if (_cells[index]->lastRow == r && _cells[index]->lastCol == c)
  //            {
  //              // calculate H space required
  //              for (int i = _cells[index]->col; i < _cells[index]->lastCol; i++)
  //                {
  //                  spaceUsed += cd[c] + spacing();
  //                }
  //              spaceReq = _cells[index]->width - spaceUsed;
  //              cd[c].value = spaceReq;
  //              // integrate c and r sums.
  //            }
  //        }
  //    }
}

unsigned int
GridLayout::columns() const
{
  return _cols;
}

unsigned int
GridLayout::rows() const
{
  return _rows;
}

unsigned int
GridLayout::columnWidth(int column) const
{
  if (column < _cols)
    return _colWidths[column];
  return 0;
}

unsigned int
GridLayout::rowHeight(int row) const
{
  if (row < _rows)
    return _rowHeights[row];
  return 0;
}

void
GridLayout::setColumnWidth(unsigned int column, unsigned int colWidth)
{
  if (column < _cols)
    _colWidths[column] = colWidth;
}

void
GridLayout::setRowHeight(unsigned int row, unsigned int rowHeight)
{
  if (row < _rows)
    _rowHeights[row] = rowHeight;
}

void
GridLayout::addWidget(Widget* widget)
{
  if (!widget)
    return;

  bool result = false;
  for (unsigned int i = 0; i < _cells.size(); i++)
    {
      // add widget if cell is empty.
      if (_cells[i] == NULL)
        {
          addChild(widget);
          _layoutModified = true;
          RadioButton* radio = dynamic_cast<RadioButton*> (widget);
          if (radio)
            _group->add(radio);
          int r = i / _cols;
          int c = r ? i % (r * _cols) : i;
          CellData* data = new CellData(widget, r, c, r, c);
          _cells[i] = data;
          result = true;
          // LOG_DEBUG("Widget added from (%d, %d) to (%d, %d)", r, c, r, c);
          break;
        }
    }
  if (!result)
    ILOG_ERROR("No more space in this layout!");
}

void
GridLayout::addWidget(Widget* widget, int row, int col, int rowSpan,
    int colSpan)
{
  if (!widget)
    return;

  if (row > _rows)
    {
      ILOG_ERROR("Row index %d is outside grid!", row);
      return;
    }

  if (col > _cols)
    {
      ILOG_ERROR("Column index %d is outside grid!", col);
      return;
    }

  int index = row * _cols + col;
  if (_cells[index])
    {
      ILOG_ERROR("Cell [%d, %d] is occupied!", row, col);
      return;
    }

  addChild(widget);
  _layoutModified = true;
  RadioButton* radio = dynamic_cast<RadioButton*> (widget);
  if (radio)
    _group->add(radio);

  int lastRow = row;
  if (rowSpan == -1)
    lastRow = _rows - 1;
  else if (rowSpan > 1)
    lastRow = row + rowSpan - 1;
  if (lastRow >= _rows)
    lastRow = _rows - 1;

  int lastCol = col;
  if (colSpan == -1)
    lastCol = _cols - 1;
  else if (colSpan > 1)
    lastCol = col + colSpan - 1;
  if (lastCol >= _cols)
    lastCol = _cols - 1;

  CellData* data = new CellData(widget, row, col, lastRow, lastCol);
  _cells[index] = data;

  for (int r = row; r <= data->lastRow; r++)
    {
      for (int c = col; c <= data->lastCol; c++)
        _cells[r * _cols + c] = data;
    }
  //  LOG_DEBUG("Widget added from (%d, %d) to (%d, %d)", row, col, lastRow, lastCol);
}

void
GridLayout::tile()
{
  if (!_layoutModified)
    return;

  int index = 0; // Cell index
  Widget* widget; // Widget at _cells[index]
  int expanding = 0; // number of expanding columns or rows.
  int spaceReq; // cell's required space in horizontal or vertical direction.
  int spaceUsed = 0; // space made available to widget in horizontal or vertical direction.
  int cActive = 0; // number of active columns
  int rActive = 0; // number of active rows

  //***********************************************************
  //                    Clear Cell Data
  //***********************************************************
  for (int i = 0; i < _rows * _cols; i++)
    {
      if (!_cells[i])
        continue;
      _cells[i]->ignored = false;
      _cells[i]->width = -2;
      _cells[i]->height = -2;
      _cells[i]->h4w = -2;
    }

  //***********************************************************
  //                  Initialise Column Data
  //***********************************************************
  ILOG_DEBUG("Initialising column data...");
  LineDataVector cd;
  for (unsigned int c = 0; c < _cols; c++)
    {
      cd.push_back(LineData());

      // if column has user set width, fix it and ignore cell data.
      if (_colWidths[c])
        {
          cd[c].min = _colWidths[c];
          cd[c].value = _colWidths[c];
          cd[c].active = true;
          cActive++;
          continue;
        }

      // For each cell on this column...
      for (unsigned int r = 0; r < _rows; r++)
        {
          index = r * _cols + c;

          // if the cell is empty, go to next cell.
          if (!_cells[index])
            continue;

          // lookup widget on this cell
          widget = _cells[index]->widget;

          // if the widget is ignored, go to next cell.
          if (!widget->visible() && (widget->vConstraint() == IgnoredConstraint
              || widget->hConstraint() == IgnoredConstraint))
            {
              // this cell becomes ignored from now on...
              _cells[index]->ignored = true;
              continue;
            }

          // this column has active widget(s)
          cd[c].active = true;

          // request preferred size from the widget only once.
          if (_cells[index]->width == -2)
            {
              Size s = widget->preferredSize();
              _cells[index]->width = s.width();
              _cells[index]->height = s.height();
            }

          // This flag is used to specify that data for current column should be updated.
          // If the widget does not span horizontally further this cell, then flag is set to true.
          bool updateCol = false;

          // the widget on the cell is not spanning.
          if (_cells[index]->col == _cells[index]->lastCol)
            {
              spaceUsed = 0;
              spaceReq = _cells[index]->width;
              updateCol = true;
            }

          // the widget on the cell is not spanning no more.
          else if (c == _cells[index]->lastCol)
            {
              spaceUsed = 0;

              // if previous cells can not shrink or they have min. width set, subtract their total width (cUsed) from this widget's width.
              // this amount is always available to the widget for use.
              for (int i = _cells[index]->col; i < _cells[index]->lastCol; i++)
                {
                  if (cd[i].min)
                    spaceUsed += cd[i].min + spacing();
                  else if (!(cd[i].constraint & ShrinkPolicy))
                    spaceUsed += cd[i].value + spacing();
                }
              spaceReq = _cells[index]->width - spaceUsed;

              if (spaceReq < 0)
                spaceReq = 0;
              updateCol = true;
            }

          if (updateCol)
            {
              // handle max. and min. size for column
              if (cd[c].min < (widget->minWidth() - spaceUsed))
                cd[c].min = widget->minWidth() - spaceUsed;

              if (widget->maxWidth() > 0 && cd[c].max > (widget->maxWidth()
                  - spaceUsed))
                cd[c].max = widget->maxWidth() - spaceUsed;

              // the widget's width is fixed.
              if (widget->hConstraint() == FixedConstraint)
                {
                  // update column value as cell can not shrink any more...
                  if (spaceReq > cd[c].value)
                    {
                      cd[c].value = spaceReq;
                      ILOG_DEBUG("Column %d value is updated to %d on cell [%d, %d]", c, spaceReq, r, c);
                    }
                }
              else
                {
                  // the widget requires space.
                  if (!(widget->hConstraint() & ShrinkPolicy) && cd[c].value
                      < spaceReq)
                    {
                      cd[c].value = spaceReq;
                      ILOG_DEBUG("Column %d value is updated to %d on cell [%d, %d]", c, spaceReq, r, c);
                    }

                  if (!(cd[c].constraint & GrowPolicy) && widget->hConstraint()
                      & GrowPolicy)
                    {
                      ILOG_DEBUG("Column %d acquires GrowPolicy on cell [%d, %d]", c, r, c);
                      cd[c].constraint = cd[c].constraint | GrowPolicy;
                    }

                  // column does not grow but cell can not shrink
                  else if (widget->hConstraint() & ShrinkPolicy && (cd[c].value
                      < spaceReq))
                    {
                      ILOG_DEBUG("Column %d can shrink", c);
                      cd[c].constraint = cd[c].constraint | ShrinkPolicy;
                      cd[c].value = spaceReq;
                    }
                }
              // if cell should expand, set the expand flag to 1.
              if (widget->hConstraint() & ExpandPolicy)
                {
                  if (!(cd[c].constraint & ExpandPolicy))
                    expanding++;
                  cd[c].constraint = cd[c].constraint | ExpandPolicy;
                }
            }
        }
      ILOG_DEBUG("Col[%d] value: %d, min: %d, max: %d, cons: %d", c, cd[c].value,cd[c].min, cd[c].max, cd[c].constraint);
      if (cd[c].active)
        cActive++;
    }

  // Arrange columns
  arrangeLine(cd, width(), cActive, expanding);

  //***********************************************************
  //                  Initialise Row Data
  //***********************************************************
  ILOG_DEBUG("Initialising row data...");
  expanding = 0;
  LineDataVector rd;

  for (unsigned int r = 0; r < _rows; r++)
    {
      rd.push_back(LineData());
      rd[r].constraint = FixedConstraint;

      // if row has user set height, fix it and ignore cell data.
      if (_rowHeights[r])
        {
          rd[r].min = _rowHeights[r];
          rd[r].value = _rowHeights[r];
          rd[r].active = true;
          rd[r].constraint = FixedConstraint;
          rActive++;
          continue;
        }

      // For each cell on this row...
      for (unsigned int c = 0; c < _cols; c++)
        {
          index = r * _cols + c;

          // if the cell is empty, go to next cell.
          if (!_cells[index])
            continue;

          // if widget is ignored, continue to next cell.
          if (_cells[index]->ignored)
            continue;

          // lookup widget on this cell
          widget = _cells[index]->widget;

          // this row has active widget(s)
          rd[r].active = true;

          // This flag is used to specify that data for current row should be updated.
          // If the widget does not span vertically further this cell, then flag is set to true.
          bool updateRow = false;

          if (c != _cells[index]->lastCol)
            continue;

          // the widget on the cell is not spanning.
          if (_cells[index]->row == _cells[index]->lastRow)
            {
              spaceUsed = 0;

              // calculate height for width and update height if widget allows...
              _cells[index]->h4w = widget->heightForWidth(cd[c].value);
              if (_cells[index]->h4w)
                {
                  // widget can shrink and h4w is less than widget's preferred height.
                  if (widget->vConstraint() & ShrinkPolicy
                      && _cells[index]->h4w < _cells[index]->height)
                    _cells[index]->height = _cells[index]->h4w;

                  // widget can grow and h4w is greater than widget's preferred height.

                  else if (widget->vConstraint() & GrowPolicy
                      && _cells[index]->h4w > _cells[index]->height)
                    _cells[index]->height = _cells[index]->h4w;
                }

              spaceReq = _cells[index]->height;
              updateRow = true;
              ILOG_DEBUG("R-Single > req: %d used: %d", spaceReq, spaceUsed);
            }

          //  spanning widget ends on this cell.

          else if (r == _cells[index]->lastRow)
            {
              // if previous cells can not shrink or they have min. width set, subtract their total height (cUsed)from this widget's width
              for (int i = _cells[index]->row; i < _cells[index]->lastRow; i++)
                {
                  if (rd[i].min)
                    spaceUsed += rd[i].min + spacing();
                  //                  else if (cd[c].max < INT_MAX)
                  //                    cUsed += cd[c].max;

                  else if (!(rd[i].constraint & ShrinkPolicy))
                    spaceUsed += rd[i].value + spacing();
                }

              // calculate height for width and update height if widget allows...
              _cells[index]->h4w = widget->heightForWidth(spaceUsed
                  + cd[c].value);
              if (_cells[index]->h4w)
                {
                  // can shrink
                  if (widget->vConstraint() & ShrinkPolicy
                      && _cells[index]->h4w < _cells[index]->height)
                    _cells[index]->height = _cells[index]->h4w;

                  // cannot grow

                  else if (widget->vConstraint() & GrowPolicy
                      && _cells[index]->h4w > _cells[index]->height)
                    _cells[index]->height = _cells[index]->h4w;
                }

              spaceReq = _cells[index]->height - spaceUsed;
              if (spaceReq < 0)
                spaceReq = 0;
              updateRow = true;
              ILOG_DEBUG("R-Span > req: %d used: %d", spaceReq, spaceUsed);
            }

          // calculate best height for row.
          if (updateRow)
            {
              // update min if it is less than this cells' min. (greatest min)
              if (rd[r].min < (widget->minHeight() - spaceUsed))
                rd[r].min = widget->minHeight() - spaceUsed;

              // update max if it is greater than this cells' max. (smallest max)
              if (widget->maxHeight() > 0 && rd[r].max > (widget->maxHeight()
                  - spaceUsed))
                rd[r].max = widget->maxHeight() - spaceUsed;

              if (widget->vConstraint() == FixedConstraint)
                {
                  // update row value as cell can not shrink any more...
                  if (spaceReq > rd[r].value)
                    {
                      rd[r].value = spaceReq;
                      ILOG_DEBUG("Row %d value is updated to %d on cell [%d, %d]", r, spaceReq, r, c);
                    }
                }
              else
                {
                  // the widget requires space.
                  if (!(widget->vConstraint() & ShrinkPolicy) && rd[r].value
                      < spaceReq)
                    {
                      rd[r].value = spaceReq;
                      ILOG_DEBUG("Row %d value is updated to %d on cell [%d, %d]", r, spaceReq, r, c);
                    }

                  if (!(rd[r].constraint & GrowPolicy) && widget->vConstraint()
                      & GrowPolicy)
                    {
                      ILOG_DEBUG("Row %d acquires GrowPolicy on cell [%d, %d]", r, r, c);
                      rd[r].constraint = rd[r].constraint | GrowPolicy;
                    }

                  // row does not grow but cell can not shrink
                  else if (widget->vConstraint() & ShrinkPolicy && rd[r].value
                      < spaceReq)
                    {
                      ILOG_DEBUG("Row %d can shrink", r);
                      rd[r].constraint = rd[r].constraint | ShrinkPolicy;
                      rd[r].value = spaceReq;
                    }
                }
              // if cell should expand, set the expand flag to 1.
              if (widget->vConstraint() & ExpandPolicy)
                {
                  if (!(rd[r].constraint & ExpandPolicy))
                    expanding++;
                  rd[r].constraint = rd[r].constraint | ExpandPolicy;
                }
            }
        }
      ILOG_DEBUG("Row[%d] h: %d constraint: %d", r, rd[r].value, rd[r].constraint);
      if (rd[r].active)
        rActive++;
    }
  // Arrange rows
  arrangeLine(rd, height(), rActive, expanding);

  //***********************************************************
  //                      Arrange Widgets
  //***********************************************************
  int wWidth = 0; // widget width
  int wHeight = 0; // widget height

  int wX = 0;
  int wY = 0;

  bool updateWidget = false;

  for (unsigned int c = 0; c < _cols; c++)
    {
      // ignore inactive columns
      if (!cd[c].active)
        continue;

      for (unsigned int r = 0; r < _rows; r++)
        {
          // ignore inactive rows.
          if (!rd[r].active)
            continue;

          // if current cell at index is empty, continue to next cell
          index = r * _cols + c;
          if (!_cells[index])
            continue;

          // if widget on the cell is ignored, continue to next cell
          widget = _cells[index]->widget;
          if (_cells[index]->ignored)
            continue;

          updateWidget = false;

          if (_cells[index]->lastRow == _cells[index]->row
              && _cells[index]->lastCol == _cells[index]->col)
            {
              wWidth = cd[c].value;
              wHeight = rd[r].value;
              updateWidget = true;
            }

          else if (_cells[index]->lastRow == r && _cells[index]->lastCol == c)
            {
              wWidth = 0;
              wHeight = 0;

              for (int i = _cells[index]->col; i < c; i++)
                wWidth += cd[i].value + spacing();
              wWidth += cd[c].value;

              for (int i = _cells[index]->row; i < r; i++)
                wHeight += rd[i].value + spacing();
              wHeight += rd[r].value;
              updateWidget = true;
            }

          // Set widget geometry...
          if (updateWidget)
            {
              int x = cd[_cells[index]->col].pos;
              int y = rd[_cells[index]->row].pos;

              // set width
              if (widget->hConstraint() == FixedConstraint)
                widget->setWidth(_cells[index]->width);

              else if (wWidth < _cells[index]->width && !(widget->hConstraint()
                  & ShrinkPolicy))
                widget->setWidth(_cells[index]->width);

              else if (wWidth > _cells[index]->width && !(widget->hConstraint()
                  & GrowPolicy))
                widget->setWidth(_cells[index]->width);

              else
                widget->setWidth(wWidth);

              // set height
              if (widget->vConstraint() == FixedConstraint)
                {
                  widget->setHeight(_cells[index]->height);
                  y += (rd[_cells[index]->row].value - widget->height()) / 2;
                }

              else if (wHeight < _cells[index]->height
                  && !(widget->vConstraint() & ShrinkPolicy))
                widget->setHeight(_cells[index]->height);

              else if (wHeight > _cells[index]->height
                  && !(widget->vConstraint() & GrowPolicy))
                widget->setHeight(_cells[index]->height);

              else
                widget->setHeight(wHeight);

              widget->moveTo(x, y);

              // LOG_DEBUG("Widget in Cell[%d, %d] - Pos(%d, %d) - Size(%d, %d)", _cells[index]->row, _cells[index]->col, widget->x(), widget->y(), widget->width(), widget->height());
            }
        }
    }
  _layoutModified = false;
}

void
GridLayout::arrangeLine(LineDataVector& ld, int availableSpace, int nActive,
    int expanding)
{
  availableSpace -= ((nActive - 1) * spacing());
  int lineAverage = availableSpace / nActive;
  ILOG_DEBUG("Count: %d\t availableSpace: %d\t average: %d", nActive, availableSpace, lineAverage);
  LineDataVector ldCopy = ld;
  //***********************************************************
  //                    FixedConstraint
  //***********************************************************
  int spaceUsed = 0;
  LineDataVectorIterator it = ldCopy.begin();
  while (it != ldCopy.end())
    {
      //        ignore non-active columns...
      if (!((LineData) *it).active)
        it = ldCopy.erase(it);
      else if (((LineData) *it).constraint == FixedConstraint
          && ((LineData) *it).min == 0)
      //          && ((ActiveWidget) *it).widget->maxHeight() < 0)

        {
          spaceUsed += ((LineData) *it).value;
          it = ldCopy.erase(it);
          nActive--;
        }
      else
        ++it;
    }
  if (spaceUsed)
    {
      availableSpace -= spaceUsed;
      if (ldCopy.size())
        lineAverage = availableSpace / ldCopy.size();
      ILOG_DEBUG("Count@fixed: %d\t availableSpace: %d\t average: %d", ldCopy.size(), availableSpace, lineAverage);
    }

  // TODO: Error in max-min size.

  //***********************************************************
  //                      MaximumSize
  //***********************************************************
  spaceUsed = 0;
  it = ldCopy.begin();
  while (it != ldCopy.end())
    {
      if (!(((LineData) *it).constraint & ExpandPolicy) && lineAverage
          > ((LineData) *it).max)
        {
          spaceUsed += ((LineData) *it).max;
          it = ldCopy.erase(it);
        }
      else
        ++it;
    }
  if (spaceUsed)
    {
      availableSpace -= spaceUsed;
      if (ldCopy.size())
        lineAverage = availableSpace / ldCopy.size();
      ILOG_DEBUG("Count@Max: %d\t availableSpace: %d\t average: %d", ldCopy.size(), availableSpace, lineAverage);
    }

  //***********************************************************
  //                       MinimumSize
  //***********************************************************
  spaceUsed = 0;
  it = ldCopy.begin();
  while (it != ldCopy.end())
    {
      if (lineAverage < ((LineData) *it).min)
        {
          spaceUsed += ((LineData) *it).min;
          it = ldCopy.erase(it);
        }
      else
        ++it;
    }
  if (spaceUsed)
    {
      availableSpace -= spaceUsed;
      if (ldCopy.size())
        lineAverage = availableSpace / ldCopy.size();
      ILOG_DEBUG("Count@Min: %d\t spaceUsed=%d\t availableSpace: %d\t average: %d", ldCopy.size(), spaceUsed, availableSpace, lineAverage);
    }

  //***********************************************************
  //                     ShrinkPolicy
  //***********************************************************
  spaceUsed = 0;
  it = ldCopy.begin();
  while (it != ldCopy.end())
    {
      if (lineAverage < ((LineData) *it).value && !(((LineData) *it).constraint
          & ShrinkPolicy))
        {
          spaceUsed += ((LineData) *it).value;
          it = ldCopy.erase(it);
        }
      else
        ++it;
    }
  if (spaceUsed)
    {
      availableSpace -= spaceUsed;
      if (ldCopy.size())
        lineAverage = availableSpace / ldCopy.size();
      ILOG_DEBUG("Count@MiC: %d\t availableSpace: %d\t average: %d", ldCopy.size(), availableSpace, lineAverage);
    }

  //***********************************************************
  //                     GrowPolicy
  //***********************************************************
  spaceUsed = 0;
  it = ldCopy.begin();
  while (it != ldCopy.end())
    {
      if (lineAverage > ((LineData) *it).value && !(((LineData) *it).constraint
          & GrowPolicy))
        {
          spaceUsed += ((LineData) *it).value;
          it = ldCopy.erase(it);
        }
      else
        ++it;
    }
  if (spaceUsed)
    {
      availableSpace -= spaceUsed;
      if (ldCopy.size())
        lineAverage = availableSpace / ldCopy.size();
      ILOG_DEBUG("Count@MaC: %d\t availableSpace: %d\t average: %d", ldCopy.size(), availableSpace, lineAverage);
    }

  //***********************************************************
  //                       ExpandPolicy
  //***********************************************************
  int expandAvg = 0;
  if (expanding)
    {
      ILOG_DEBUG("Entering expanding... Avg: %d", lineAverage);
      int expandSpace = 0;
      it = ldCopy.begin();
      while (it != ldCopy.end())
        {
          if (!(((LineData) *it).constraint & ExpandPolicy))
            {
              if (((LineData) *it).min > 0 && lineAverage
                  > ((LineData) *it).min && ((LineData) *it).min
                  > ((LineData) *it).value)
                {
                  expandSpace += lineAverage - ((LineData) *it).min;
                  ((LineData) *it).value = ((LineData) *it).min;
                  ILOG_DEBUG("Additional space found: %d, min: %d", lineAverage - ((LineData) *it).min, ((LineData) *it).min);
                }
              else if (lineAverage > ((LineData) *it).value)
                {
                  expandSpace += lineAverage - ((LineData) *it).value;
                  ILOG_DEBUG("Additional space found: %d, value: %d", lineAverage - ((LineData) *it).value, ((LineData) *it).value);
                }
            }
          ++it;
        }
      if (expandSpace)
        {
          expandAvg = expandSpace / expanding;
          ILOG_DEBUG("expandAverage: %d, average: %d", expandAvg, lineAverage);
        }
    }

  //***********************************************************
  //                    Set Line Geometry
  //***********************************************************
  int artifact = availableSpace - lineAverage * ldCopy.size();
  int pos = 0;
  ILOG_DEBUG("available: %d, average: %d, artifact: %d", availableSpace, lineAverage, artifact);

  for (unsigned int i = 0; i < ld.size(); i++)
    {
      if (!ld[i].active) // ignore inactive rows
        continue;

      if (expanding)
        {
          if (ld[i].constraint & ExpandPolicy)
            {
              if (artifact)
                {
                  ld[i].value = lineAverage + expandAvg + artifact;
                  artifact = 0;
                }
              else
                ld[i].value = lineAverage + expandAvg;
            }
          else if (ld[i].min > ld[i].value)
            ld[i].value = ld[i].min;

          else if (ld[i].max < ld[i].value)
            ld[i].value = ld[i].max;

          else if (ld[i].constraint & ShrinkPolicy && lineAverage < ld[i].value)
            ld[i].value = lineAverage;
        }
      else
        {
          if (ld[i].min > lineAverage)
            ld[i].value = ld[i].min;

          else if (ld[i].max < lineAverage)
            ld[i].value = ld[i].max;

          else if (ld[i].constraint & ShrinkPolicy && ld[i].value > lineAverage)
            ld[i].value = lineAverage;

          else if (ld[i].constraint & GrowPolicy && ld[i].value < lineAverage)
            {
              if (artifact)
                {
                  ld[i].value = lineAverage + artifact;
                  artifact = 0;
                }
              else
                ld[i].value = lineAverage;
            }
        }
      ld[i].pos = pos;
      //LOG_DEBUG(">> line[%d] - pos=%d, value=%d, cons=%d", i, pos, ld[i].value, ld[i].constraint);
      pos += ld[i].value + spacing();
    }
}

