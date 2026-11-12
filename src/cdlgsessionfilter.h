// cdlgsessionfilter.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
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
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef CDLGSESSIONFILTER_H
#define CDLGSESSIONFILTER_H

#include "sessionfilter.h"

#include <QDialog>
#include <QListWidgetItem>

#include <vector>

namespace Ui {
class CDlgSessionFilter;
}

class CDlgSessionFilter : public QDialog {
  Q_OBJECT

public:
  explicit CDlgSessionFilter(QWidget* parent = nullptr);
  ~CDlgSessionFilter();

public:
  /// File with the session filters of the user
  static QString defaultFilterPath(void);

  /// Read the session filters of the user (empty list if there are none)
  static std::vector<CSessionFilter> loadDefaultFilters(void);

  /// Save the session filters of the user
  static bool saveDefaultFilters(const std::vector<CSessionFilter>& filters);

  /// Filters as edited in the dialog
  const std::vector<CSessionFilter>& getFilters(void) const { return m_filters; };

  /// Select a filter by name
  void selectFilter(const QString& name);

  /// Saves the filters when OK is pressed
  void accept(void) override;

private slots:

  /// Show the description of the selected filter
  void filterSelected(int row);

  /// Context menu
  void showTxContextMenu(const QPoint& pos);

  /// Add filter
  void addFilter(void);

  /// Edit filter
  void editFilter(void);

  /// Clone filter
  void cloneFilter(void);

  /// Delete filter
  void deleteFilter(void);

  /// Load filter
  void loadFilter(void);

  /// Save filter
  void saveFilter(void);

  /// Help
  void showHelp(void);

private:
  /// Fill the list with the filter names
  void fillList(int selectRow);

  /// True if a filter (other than skip) has this name
  bool nameExists(const std::string& name, int skip = -1) const;

  Ui::CDlgSessionFilter* ui;

  /// The filters
  std::vector<CSessionFilter> m_filters;
};

#endif // CDLGSESSIONFILTER_H
