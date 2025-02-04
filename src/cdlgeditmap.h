// cdlgeditmap.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2025 Ake Hedman, Grodans Paradis AB
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

#ifndef CDLGEDITMAP_H
#define CDLGEDITMAP_H

#include <vscpworks.h>
#include <mdf.h>

#include <QDialog>

typedef enum descriptionMap {
  map_type_description,
  map_type_info_url
} descriptionMap;

namespace Ui {
class CDlgEditMap;
}

class CDlgEditMap : public QDialog {
  Q_OBJECT

public:
  explicit CDlgEditMap(QWidget* parent = nullptr);
  ~CDlgEditMap();

  /*!
      Set inital focus to description
  */
  void setInitialFocus(void);

  /*!
    Init dialog data
    @param pmap Pointer to map
    @param pselstr Pointer to ISO639 language string to handle.
      Set to nullptr to add new item
  */
  void initDialogData(std::map<std::string, std::string>* pmap, descriptionMap type, const QString* pselstr = nullptr);

  /*!
    Fill in Info URL items
    @param pselstr Pointer to language string or nullptr
  */
  void fillMapData(const QString* pselstr = nullptr);

  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

public slots:
  void accept(void);

  // Description buttons
  void addMapItem(void);
  void editMapItem(void);
  void dupMapItem(void);
  void deleteMapItem(void);

private slots:

  /// Help
  void showHelp(void);

private:
  Ui::CDlgEditMap* ui;

  // map data
  std::map<std::string, std::string>* m_pMap;

  // Initial selstr
  QString m_initial_selstr;
};

#endif // CDLGEDITMAP_H
