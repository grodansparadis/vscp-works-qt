// cdlgtxtsearch.h
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

#ifndef CDLGTXTSEARCH_H
#define CDLGTXTSEARCH_H

#include <QDialog>

// Defined search types
typedef enum search_type {
  SEARCH_TYPE_EXACT,
  SEARCH_TYPE_CONTAINS,
  SEARCH_TYPE_START,
  SEARCH_TYPE_END,
  SEARCH_TYPE_REGULAR_EXPRESSION
} search_type;

namespace Ui {
class CDlgTxtSearch;
}

class CDlgTxtSearch : public QDialog {
  Q_OBJECT

public:
public:
  explicit CDlgTxtSearch(QWidget* parent = nullptr);
  ~CDlgTxtSearch();

  /*!
      Set inital focus to description
  */
  void setInitialFocus(void);

  /*!
      Setters/getters for search text
  */
  std::string getSearchText(void);
  void setSearchText(const std::string& str);

  /*!
      Setters/getters for search type
  */
  search_type getSearchType(void);
  void setSearchType(search_type type);

  /*!
    Setters/Getters for case sensitive checkbox
  */
  bool isCaseSensitive();
  void setCaseSensitive(bool bon = true);

  /*!
    Setters/Getters for marking of search results
  */
  bool isMarkSearchResults();
  void setMarkSearchResults(bool bon = true);

private:
  Ui::CDlgTxtSearch* ui;
};

#endif // CDLGTXTSEARCH_H