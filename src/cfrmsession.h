// cfrmsession.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2020 Ake Hedman, Grodans Paradis AB
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
#ifndef CFRMSESSION_H
#define CFRMSESSION_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QAction;
class QDialogButtonBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QMenuBar;
class QPushButton;
class QTextEdit;
class QToolBar;
class QVBoxLayout;
class QAction;
class QTableWidget;
class QToolBox;
QT_END_NAMESPACE

class CFrmSession : public QDialog
{
    Q_OBJECT

public:
    CFrmSession(QWidget *parent);
    ~CFrmSession();

private:
    void createMenu();
    void createHorizontalGroupBox();
    void createGridGroupBox();
    void createTxGridGroup();
    void createFormGroupBox();

    void transmitEvent();

    enum { NumGridRows = 8, NumButtons = 4 };

    QMenuBar *menuBar;
    QToolBar *toolBar;
    QGroupBox *horizontalGroupBox;
    QGroupBox *gridGroupBox;
    QGroupBox *txGroupBox;
    QGroupBox *formGroupBox;
    QTextEdit *smallEditor;
    QTextEdit *bigEditor;
    QLabel *labels[NumGridRows];
    QLineEdit *lineEdits[NumGridRows];
    QPushButton *buttons[NumButtons];
    QDialogButtonBox *buttonBox;

    QTableWidget *rxTable;
    QTableWidget *txTable;

    QMenu *fileMenu;
    QMenu *hostMenu;
    QMenu *editMenu;
    QMenu *viewMenu;
    QMenu *vscpMenu;
    QMenu *settingsMenu;
    QMenu *toolsMenu;
  
    QToolBar *editToolBar;

    // File menu
    QAction *loadEventsAct;
    QAction *saveEventsAct;
    QAction *loadTxAct;
    QAction *saveTxAct;
    QAction *exitAct;

    // Host meny
    QAction *connectAct;
    QAction *disconnectAct;
    QAction *pauseAct;
    QAction *addHostAct;
    QAction *editHostAct;

    // Edit menu
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteBeforeAct;
    QAction *pasteAfterAct;

    // View menu
    QAction *viewMessageAct;
    QAction *viewCountAct;
    QAction *viewClrRxListAct;
    QAction *viewClrTxListAct;

    // VSCP menu
    QAction *readRegAct;
    QAction *writeRegAct;
    QAction *readAllRegAct;
    QAction *readGuidAct;
    QAction *readMdfAct;
    QAction *loadMdfAct;

    // Settings menu
    QAction *setFilterAct;
    QAction *settingsAct;

    QToolBar *txToolBar;
};

#endif // CFRMSESSION_H
