// explorerstyle.cpp
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

#include "explorerstyle.h"

#include <QAbstractButton>
#include <QAction>
#include <QFontDatabase>
#include <QLabel>
#include <QStyle>
#include <QToolBar>

namespace explorerstyle {

QString
styleSheet()
{
  return QStringLiteral(R"(
    QDialog {
      background: palette(window);
    }
    QMenuBar {
      background: transparent;
      border: none;
    }
    QToolBar {
      background: transparent;
      border: none;
      spacing: 4px;
      padding: 0px;
    }
    QToolBar QToolButton {
      border: 1px solid transparent;
      border-radius: 6px;
      padding: 5px 10px;
    }
    QToolBar QToolButton:hover {
      background: palette(midlight);
      border-color: palette(mid);
    }
    QToolBar QToolButton:checked {
      background: palette(midlight);
      border-color: palette(mid);
    }
    QGroupBox {
      background: palette(base);
      border: 1px solid palette(mid);
      border-radius: 10px;
      margin-top: 16px;
      padding: 14px 10px 10px 10px;
    }
    QGroupBox::title {
      subcontrol-origin: margin;
      left: 12px;
      top: 4px;
      padding: 0 4px;
      font-weight: 600;
    }
    QPushButton {
      background: palette(button);
      border: 1px solid palette(mid);
      border-radius: 6px;
      padding: 5px 14px;
    }
    QPushButton:hover {
      background: palette(midlight);
    }
    QPushButton:checked {
      background: palette(midlight);
      font-weight: 600;
    }
    QPushButton:disabled {
      color: palette(mid);
    }
    QPushButton[explorerRole="primary"], QToolButton[explorerRole="primary"] {
      background: palette(highlight);
      color: palette(highlighted-text);
      border: 1px solid palette(highlight);
      font-weight: 600;
    }
    QPushButton[explorerRole="primary"]:disabled {
      background: palette(mid);
      border-color: palette(mid);
    }
    QLineEdit, QPlainTextEdit, QSpinBox {
      background: palette(base);
      border: 1px solid palette(mid);
      border-radius: 6px;
      padding: 4px 8px;
    }
    QLineEdit:focus, QPlainTextEdit:focus, QSpinBox:focus {
      border: 1px solid palette(highlight);
    }
    QTreeWidget, QTableWidget, QListWidget {
      background: palette(base);
      alternate-background-color: palette(alternate-base);
      border: 1px solid palette(mid);
      border-radius: 8px;
      padding: 2px;
    }
    QHeaderView::section {
      background: palette(window);
      border: none;
      border-bottom: 1px solid palette(mid);
      padding: 6px 8px;
      font-weight: 600;
    }
    QSplitter::handle {
      background: transparent;
    }
    QLabel[explorerRole="badge"] {
      border-radius: 10px;
      padding: 3px 12px;
      font-weight: 600;
    }
    QLabel[explorerState="connected"] {
      background: #dcfce7;
      color: #166534;
    }
    QLabel[explorerState="connecting"], QLabel[explorerState="paused"] {
      background: #fef3c7;
      color: #92400e;
    }
    QLabel[explorerState="disconnected"] {
      background: #e5e7eb;
      color: #374151;
    }
    QLabel[explorerState="error"] {
      background: #fee2e2;
      color: #991b1b;
    }
    QLabel[explorerRole="hint"] {
      color: palette(dark);
    }
  )");
}

static void
repolish(QWidget* widget)
{
  widget->style()->unpolish(widget);
  widget->style()->polish(widget);
  widget->update();
}

void
setPrimary(QAbstractButton* button)
{
  if (nullptr == button) {
    return;
  }
  button->setProperty("explorerRole", "primary");
  repolish(button);
}

void
setPrimary(QToolBar* toolBar, QAction* action)
{
  if ((nullptr == toolBar) || (nullptr == action)) {
    return;
  }
  QWidget* widget = toolBar->widgetForAction(action);
  if (nullptr != widget) {
    widget->setProperty("explorerRole", "primary");
    repolish(widget);
  }
}

void
makeBadge(QLabel* label)
{
  if (nullptr == label) {
    return;
  }
  label->setProperty("explorerRole", "badge");
  label->setAlignment(Qt::AlignCenter);
  repolish(label);
}

void
setBadge(QLabel* label, State state, const QString& text)
{
  if (nullptr == label) {
    return;
  }

  const char* name = "disconnected";
  switch (state) {
    case State::Connected:
      name = "connected";
      break;
    case State::Connecting:
      name = "connecting";
      break;
    case State::Paused:
      name = "paused";
      break;
    case State::Error:
      name = "error";
      break;
    case State::Disconnected:
    default:
      break;
  }

  label->setText(text);
  if (label->property("explorerState").toString() != QLatin1String(name)) {
    label->setProperty("explorerState", name);
    repolish(label);
  }
}

QFont
monospaceFont()
{
  return QFontDatabase::systemFont(QFontDatabase::FixedFont);
}

} // namespace explorerstyle
