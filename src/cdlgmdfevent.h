// cdlgmdfevent.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2024 Ake Hedman, Grodans Paradis AB
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

#ifndef CDLGMDFDM_EVENT_H
#define CDLGMDFDM_EVENT_H

#include <mdf.h>
#include <vscpworks.h>

#include <QDialog>

namespace Ui {
class CDlgMdfEvent;
}

class CDlgMdfEvent : public QDialog {
  Q_OBJECT

public:
  explicit CDlgMdfEvent(QWidget* parent = nullptr);
  ~CDlgMdfEvent();

  static const int index_name      = 0;
  static const int index_class     = 1;
  static const int index_type      = 2;
  static const int index_priority  = 3;
  static const int index_direction = 4;
  static const int index_data      = 5;

  static const char pre_str_event[];

  /*!
    Init dialog data
    @param CMDF *pmdf Pointer to MDF
    @param pmdfobject Pointer to MDF object
    @param bAdd True if event should be added.
    @param index Selected file item

  */
  void initDialogData(CMDF* pmdf, CMDF_Event* pevent, bool bAdd, int index = 0);

  /*!
    Only enable parameter list
  */
  void setReadOnly(void);

  // ----------------------------------------------------------------------------
  //                             Getters & Setters
  // ----------------------------------------------------------------------------

  // priority
  uint8_t getPriority(void);
  void setPriority(uint8_t code);

  // name
  QString getName(void);
  void setName(const QString& name);

  // Direction
  mdf_event_direction getDirection(void);
  void setDirection(mdf_event_direction dir);

  /*!
    Setters/getters for VSCP class/type
  */
  uint16_t getVscpClass(void);
  uint16_t getVscpType(void);
  void setVscpClassType(uint16_t vscpClass, uint16_t vscpType);

  // Render available actions
  void renderEventData(void);

  /*!
        Set initial focus to name field
    */
  void setInitialFocus(void);

  /*!
      Show information page for the currently  selected
      VSCP class
  */
  void showVscpClassInfo();

  /*!
        Show information page for the currently selected
        VSCP type
    */
    void showVscpTypeInfo();

  /*!
        Fill the VSCP class combobox with
        VSCP classes.
        @param vscpClass VSCP class to select. Defaults
                to zero for the first item.
    */
  void fillVscpClass(uint16_t vscpclass = 0);

  /*!
      Fill the VSCP type combobox with
      VSCP types.
      @param vscpClass Class o fill types for
      @param vscpType VSCP type to select. defaults to
              zero for the firsts item.
  */
  void fillVscpType(uint16_t vscpclass, uint16_t vscpType = 0);

public slots:

  // Edit event data item
  void editEventData(void);

  // Add event data item
  void addEventData(void);

  // Duplicate event data item
  void dupEventData(void);

  // Delete event data item
  void deleteEventData(void);

  /*!
    Accept dialog data and write to register
  */
  void accept(void);

private slots:
  /*!
      User changed VSCP class in combo
      @param index New selected index
  */
  void currentVscpClassIndexChanged(int index);

private:
  Ui::CDlgMdfEvent* ui;

  /// True if event should be added.
  bool m_bAdd;

  /// Pointer to MDF
  CMDF* m_pmdf;

  /// Event to add/edit
  CMDF_Event* m_pEvent;
};

#endif // CDLGMDFDM_ACTIONS_H
