// cfrmrawcansession.h
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

#ifndef CFRMRAWCANSESSION_H
#define CFRMRAWCANSESSION_H

#ifndef WIN32

#include <QtSerialBus/QCanBusDevice>
#include <QtSerialBus/QCanBusFrame>

#include <QAction>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QPushButton>
#include <QSettings>
#include <QShowEvent>
#include <QStackedWidget>
#include <QTableWidget>
#include <QToolBar>
#include <QTreeWidget>
#include <QVector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

class CFrmRawCanSession : public QDialog
{
  Q_OBJECT

public:
  explicit CFrmRawCanSession(QWidget* parent, json* pconn);
  ~CFrmRawCanSession();

private slots:
  void connectOrDisconnect();
  void sendFrame();
  void processReceivedFrames();
  void processError(QCanBusDevice::CanBusError error);
  void clearLog();
  void showHelp();
  void onViewModeChanged(int index);
  void showIdFiltersDialog();
  void addIdFilter();
  void removeSelectedIdFilter();
  void onFilterTableChanged(QTableWidgetItem* item);
  void saveCurrentFrameAsTemplate();
  void deleteSelectedTemplate();
  void clearTemplates();
  void sendSelectedTemplate();
  void loadTemplatesFromDisk();
  void saveTemplatesToDisk();
  void refreshTemplatesView();
  void onTemplateSelectionChanged();
  void setSendFrameVisible(bool visible);
  void setSavedFramesVisible(bool visible);

protected:
  void showEvent(QShowEvent* event) override;

private:
  struct SavedFrameTemplate;

  enum class FrameDirection
  {
    Tx,
    Rx
  };

  struct FrameRecord {
    QDateTime timestamp;
    FrameDirection direction;
    QCanBusFrame frame;
  };

  struct IdFilterRange {
    bool enabled;
    uint32_t idFrom;
    uint32_t idTo;
  };

  struct SavedFrameTemplate {
    QString name;
    QString frameId;
    QString payload;
    bool extended;
    bool fd;
    bool bitrateSwitch;
    bool errorStateIndicator;
    bool remoteRequest;
  };

  void setupUi();
  void appendFrame(const QCanBusFrame& frame, FrameDirection direction);
  void applyTemplateToInputs(const SavedFrameTemplate& templateItem);
  void setConnectedState(bool connected);
  bool parseFrameId(uint32_t& id);
  bool parsePayload(QByteArray& payload);
  bool parseIdValue(const QString& str, uint32_t& id) const;
  bool isFrameVisibleByFilter(const QCanBusFrame& frame) const;
  void refreshViews();
  void refreshFrameView();
  void refreshSummaryView();
  void refreshFilterModelFromTable();
  void populateFilterTableFromModel();
  QString directionText(FrameDirection direction) const;
  QColor rowBackgroundColorForDirection(FrameDirection direction) const;
  QColor rowForegroundColorForDirection(FrameDirection direction) const;
  QColor frameTypeBackgroundColor(const QCanBusFrame& frame) const;
  QColor frameTypeForegroundColor(const QCanBusFrame& frame) const;
  QString formatId(uint32_t id, bool extended) const;
  QString formatPayload(const QByteArray& payload) const;
  QString frameFlagsToString(const QCanBusFrame& frame) const;

  json m_connObject;
  QString m_interfaceName;

  QCanBusDevice* m_canDevice;
  QVector<FrameRecord> m_frameHistory;
  QVector<IdFilterRange> m_idFilters;
  QVector<SavedFrameTemplate> m_savedTemplates;
  bool m_autoConnectAttempted;

  QLabel* m_statusLabel;
  QComboBox* m_comboViewMode;
  QTableWidget* m_tableIdFilters;
  QStackedWidget* m_stackViews;
  QDialog* m_idFilterDialog;
  QGroupBox* m_sendFrameBox;
  QGroupBox* m_templatesBox;
  QLineEdit* m_editFrameId;
  QLineEdit* m_editPayload;
  QCheckBox* m_chkExtended;
  QCheckBox* m_chkFd;
  QCheckBox* m_chkBitrateSwitch;
  QCheckBox* m_chkErrorStateIndicator;
  QCheckBox* m_chkRemoteRequest;
  QPushButton* m_btnConnect;
  QPushButton* m_btnSend;
  QPushButton* m_btnClear;
  QPushButton* m_btnAddFilter;
  QPushButton* m_btnRemoveFilter;
  QTreeWidget* m_treeFrames;
  QTableWidget* m_tableSummary;
  QTreeWidget* m_treeTemplates;
  QMenuBar* m_menuBar;
  QToolBar* m_toolBar;
  QAction* m_actSaveCurrentFrame;
  QAction* m_actSendSelectedFrame;
  QAction* m_actDeleteSelectedFrame;
  QAction* m_actClearFrames;
  QAction* m_actLoadFromDisk;
  QAction* m_actSaveToDisk;
  QAction* m_actShowIdFilters;
  QAction* m_actToggleSendFrame;
  QAction* m_actToggleSavedFrames;
};

#endif // !WIN32

#endif // CFRMRAWCANSESSION_H
