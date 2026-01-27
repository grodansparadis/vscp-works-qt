// cdlgselectmqtttopic.h
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

#ifndef CDLGSELECTMQTTTOPIC_H
#define CDLGSELECTMQTTTOPIC_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgSelectMqttTopics;
}

class CDlgSelectMqttTopics : public QDialog {
  Q_OBJECT

public:
  explicit CDlgSelectMqttTopics(QWidget* parent = nullptr);
  ~CDlgSelectMqttTopics();

public:
  // const int SUBSCRIBE = 0;
  // const int PUBLISH = 1;
  // const int CLRRETAIN = 2;
  typedef enum {
    SUBSCRIBE,
    PUBLISH,
    CLRRETAIN
  } dlgtype;

  /*!
    Fill table with subscription topics
  */
  void fillSubscriptionTopics(void);

  /*!
    Fill table with publish topics
    */
  void fillPublishTopics(void);

  /*!
    Initialize the dialog from connection data
    @param bSubscription Set to true to handle subscrivbe topics,
      false to handel publiosh topics
    @param pvscpClient Pointer to client object. Is expected
      to be initialized and opened.
    @return VSCP_ERROR_SUCCESS if OK otherwise errorcode
  */
  int init(dlgtype type, const CVscpClient* pvscpClient);

  /*!
    Get state for a row in the table widget list
  */
  bool isActive(int pos);

public slots:

  /*!
    Called when the accepted button is pressed
    Will update the subscribe/publish list with
    the active/inactive content from the list
  */
  void accepted(void);

  /*!
    Double click is used to select or deselect topics
  */
  // void onRowDoubleClicked(void);

  /// Help
  void showHelp(void);

private:
  Ui::CDlgSelectMqttTopics* ui;

  /// Topics types
  bool m_type;

  /// Pointer to client object
  vscpClientMqtt* m_pvscpClient;
};

#endif // CDlgSelectMqttTopics_H
