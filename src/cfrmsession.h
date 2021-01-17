// cfrmsession.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB
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
class QIcon;
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
    void createRxGroupBox();
    void createTxGridGroup();
    void createFormGroupBox();

    void menu_connect();
    void menu_filter();

    void transmitEvent();

    enum { NumGridRows = 8, NumButtons = 4 };

    QMenuBar *m_menuBar;
    QToolBar *m_toolBar;
    QGroupBox *m_horizontalGroupBox;
    QGroupBox *m_gridGroupBox;
    QGroupBox *m_txGroupBox;
    QGroupBox *m_formGroupBox;
    QTextEdit *m_infoArea;
    QTextEdit *m_bigEditor;
    QLabel *m_labels[NumGridRows];
    QLineEdit *m_lineEdits[NumGridRows];
    QPushButton *m_buttons[NumButtons];
    QDialogButtonBox *m_buttonBox;

    QTableWidget *m_rxTable;
    QTableWidget *m_txTable;

    QMenu *m_fileMenu;
    QMenu *m_hostMenu;
    QMenu *m_editMenu;
    QMenu *m_viewMenu;
    QMenu *m_vscpMenu;
    QMenu *m_settingsMenu;
    QMenu *m_toolsMenu;
  
    QToolBar *m_editToolBar;

    // File menu
    QAction *m_loadEventsAct;
    QAction *m_saveEventsAct;
    QAction *m_loadTxAct;
    QAction *m_saveTxAct;
    QAction *m_exitAct;

    // Host meny
    QAction *m_connectAct;
    QAction *m_disconnectAct;
    QAction *m_pauseAct;
    QAction *m_addHostAct;
    QAction *m_editHostAct;

    // Edit menu
    QAction *m_cutAct;
    QAction *m_copyAct;
    QAction *m_pasteBeforeAct;
    QAction *m_pasteAfterAct;

    // View menu
    QAction *m_viewMessageAct;
    QAction *m_viewCountAct;
    QAction *m_viewClrRxListAct;
    QAction *m_viewClrTxListAct;

    // VSCP menu
    QAction *m_readRegAct;
    QAction *m_writeRegAct;
    QAction *m_readAllRegAct;
    QAction *m_readGuidAct;
    QAction *m_readMdfAct;
    QAction *m_loadMdfAct;

    // Settings menu
    QAction *m_setFilterAct;
    QAction *m_settingsAct;

    QToolBar *m_txToolBar;
};

#endif // CFRMSESSION_H
