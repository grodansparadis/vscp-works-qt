// explorerstyle.h
//
// Shared look for the raw CAN and MQTT explorer windows.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2026 Ake Hedman, the VSCP project
// <info@vscp.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef EXPLORERSTYLE_H
#define EXPLORERSTYLE_H

#include <QFont>
#include <QString>

class QAbstractButton;
class QLabel;
class QToolBar;
class QAction;

namespace explorerstyle {

/// Connection state shown by a status badge
enum class State { Disconnected, Connecting, Connected, Paused, Error };

/*!
  Style sheet for the explorer windows. Colors come from the palette, so
  light and dark desktop themes both work.
*/
QString
styleSheet();

/*!
  Mark a button as the main action of its area (accent color)
*/
void
setPrimary(QAbstractButton* button);

/*!
  Mark the tool button of a toolbar action as the main action
*/
void
setPrimary(QToolBar* toolBar, QAction* action);

/*!
  Make a label a status badge (rounded, colored by state)
*/
void
makeBadge(QLabel* label);

/*!
  Set the text and state of a status badge
*/
void
setBadge(QLabel* label, State state, const QString& text);

/*!
  Fixed-width font for ids, hex data and payloads
*/
QFont
monospaceFont();

} // namespace explorerstyle

#endif // EXPLORERSTYLE_H
