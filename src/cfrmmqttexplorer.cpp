#include "cfrmmqttexplorer.h"

#include <mosquitto.h>

#include <QApplication>
#include <QAbstractItemView>
#include <QByteArray>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QGridLayout>
#include <QToolBar>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMetaObject>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTextStream>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUuid>
#include <QUrl>
#include <QTimer>
#include <QVBoxLayout>
#include <QXmlStreamReader>

#include <algorithm>
#include <functional>

namespace {

QString trimmedTopic(const QString& topic)
{
  return topic.trimmed();
}

QString escapeHtml(const QString& text)
{
  QString escaped = text;
  escaped.replace("&", "&amp;");
  escaped.replace("<", "&lt;");
  escaped.replace(">", "&gt;");
  return escaped;
}

QString colorizeJsonContent(const QString& text)
{
  QString escaped = escapeHtml(text);
  escaped.replace('\n', "<br>");
  escaped.replace(' ', "&nbsp;");

  QString out;
  out.reserve(escaped.size() + 64);
  bool inString = false;
  bool inKey = false;
  bool escapeNext = false;
  for (int i = 0; i < escaped.size(); ++i) {
    const QChar ch = escaped.at(i);
    if (escapeNext) {
      out += ch;
      escapeNext = false;
      continue;
    }
    if (ch == '\\') {
      out += ch;
      escapeNext = true;
      continue;
    }
    if (ch == '"') {
      if (inString) {
        inString = false;
        inKey = false;
        out += '"';
      }
      else {
        inString = true;
        inKey = false;
        out += '"';
      }
      continue;
    }
    if (inString) {
      out += ch;
      continue;
    }
    if (ch == '{' || ch == '}' || ch == '[' || ch == ']' || ch == ':' || ch == ',') {
      out += QString("<span style='color:#64748b'>%1</span>").arg(ch);
      continue;
    }
    if (ch == 't' || ch == 'f' || ch == 'n') {
      QString token;
      int j = i;
      while (j < escaped.size() && (escaped.at(j).isLetter() || escaped.at(j) == '"')) {
        token += escaped.at(j);
        ++j;
      }
      if ((token == "true") || (token == "false") || (token == "null")) {
        out += QString("<span style='color:#16a34a'>%1</span>").arg(token);
        i = j - 1;
        continue;
      }
    }
    if (ch.isDigit() || ch == '-' || ch == '.') {
      QString token;
      int j = i;
      while (j < escaped.size() && (escaped.at(j).isDigit() || escaped.at(j) == '-' || escaped.at(j) == '.' || escaped.at(j) == 'e' || escaped.at(j) == 'E' || escaped.at(j) == '+')) {
        token += escaped.at(j);
        ++j;
      }
      if (!token.isEmpty()) {
        out += QString("<span style='color:#2563eb'>%1</span>").arg(token);
        i = j - 1;
        continue;
      }
    }
    out += ch;
  }
  return out;
}

QString colorizeXmlContent(const QString& text)
{
  QString escaped = escapeHtml(text);
  escaped.replace('\n', "<br>");
  QString out;
  out.reserve(escaped.size() + 64);
  bool inTag = false;
  bool inAttrName = false;
  bool inAttrValue = false;
  for (int i = 0; i < escaped.size(); ++i) {
    const QChar ch = escaped.at(i);
    if (ch == '<') {
      inTag = true;
      inAttrName = false;
      inAttrValue = false;
      out += "<span style='color:#2563eb'>";
      out += ch;
      continue;
    }
    if (ch == '>') {
      inTag = false;
      out += ch;
      out += "</span>";
      continue;
    }
    if (inTag) {
      if (ch == '"') {
        inAttrValue = !inAttrValue;
        if (inAttrValue) {
          out += "<span style='color:#16a34a'>";
        }
        else {
          out += "</span>";
        }
        out += ch;
        continue;
      }
      if (!inAttrValue && ch == ' ' && !inAttrName) {
        out += ch;
        continue;
      }
      if (!inAttrValue) {
        out += ch;
      }
      continue;
    }
    out += ch;
  }
  return out;
}

QString formatRichTextBlock(const QString& text)
{
  const QString escaped = escapeHtml(text).replace('\n', "<br>");
  return QString("<pre style='margin:0; white-space:pre-wrap; font-family:monospace;'>%1</pre>").arg(escaped);
}

constexpr int kItemRoleKind = Qt::UserRole + 1;
constexpr int kItemRoleTopic = Qt::UserRole + 2;
constexpr int kItemRolePayloadRaw = Qt::UserRole + 3;
constexpr int kItemRolePayloadFormatted = Qt::UserRole + 4;
constexpr int kItemRoleTimestamp = Qt::UserRole + 6;
constexpr int kItemRolePayloadFormat = Qt::UserRole + 7;
constexpr int kItemRoleRetained = Qt::UserRole + 8;
constexpr int kItemRoleQos = Qt::UserRole + 10;
constexpr int kItemRoleMid = Qt::UserRole + 11;
constexpr int kItemKindTopicNode = 1;
constexpr int kItemKindMessage = 2;

QString buildTopicPathFromItem(QTreeWidgetItem* item)
{
  QStringList parts;
  for (QTreeWidgetItem* current = item; nullptr != current; current = current->parent()) {
    const QString label = current->text(0).trimmed();
    if (!label.isEmpty()) {
      parts.prepend(label);
    }
  }
  return parts.join('/');
}

bool wildcardMatches(const QString& text, const QString& pattern)
{
  int textIndex = 0;
  int patternIndex = 0;
  int starIndex = -1;
  int matchIndex = 0;

  while (textIndex < text.size()) {
    if (patternIndex < pattern.size() && (pattern.at(patternIndex) == '*' || pattern.at(patternIndex).toLower() == text.at(textIndex).toLower())) {
      if (pattern.at(patternIndex) == '*') {
        starIndex = patternIndex;
        matchIndex = textIndex;
        ++patternIndex;
      }
      else {
        ++textIndex;
        ++patternIndex;
      }
    }
    else if (starIndex >= 0) {
      patternIndex = starIndex + 1;
      matchIndex += 1;
      textIndex = matchIndex;
    }
    else {
      return false;
    }
  }

  while (patternIndex < pattern.size() && pattern.at(patternIndex) == '*') {
    ++patternIndex;
  }

  return patternIndex == pattern.size();
}

bool matchesTopicFilter(const QString& topic, const QString& pattern)
{
  const QString trimmedPattern = pattern.trimmed();
  if (trimmedPattern.isEmpty()) {
    return true;
  }

  const QString normalizedTopic = topic.trimmed();
  if (normalizedTopic.isEmpty()) {
    return false;
  }

  if (!trimmedPattern.contains('*')) {
    return normalizedTopic.compare(trimmedPattern, Qt::CaseInsensitive) == 0;
  }

  return wildcardMatches(normalizedTopic, trimmedPattern);
}

bool itemMatchesQos(QTreeWidgetItem* item, int expectedQos)
{
  if (nullptr == item) {
    return false;
  }

  if (item->data(0, kItemRoleQos).isValid()) {
    return item->data(0, kItemRoleQos).toInt() == expectedQos;
  }

  for (int i = 0; i < item->childCount(); ++i) {
    if (itemMatchesQos(item->child(i), expectedQos)) {
      return true;
    }
  }

  return false;
}

bool itemMatchesRetain(QTreeWidgetItem* item, bool expectedRetain)
{
  if (nullptr == item) {
    return false;
  }

  if (item->data(0, kItemRoleRetained).isValid()) {
    return item->data(0, kItemRoleRetained).toBool() == expectedRetain;
  }

  for (int i = 0; i < item->childCount(); ++i) {
    if (itemMatchesRetain(item->child(i), expectedRetain)) {
      return true;
    }
  }

  return false;
}

bool itemMatchesFilter(QTreeWidgetItem* item,
                       const QString& filterText,
                       const QString& topicFilterText)
{
  if (nullptr == item) {
    return false;
  }

  const QString filterLower = filterText.trimmed().toLower();
  const QString topicFilterLower = topicFilterText.trimmed().toLower();
  const QString search = item->data(0, Qt::UserRole + 5).toString();
  const QString topic = item->data(0, Qt::UserRole + 2).toString();
  const QString label = item->text(0);
  const QString branchPath = buildTopicPathFromItem(item);
  const QStringList clauses = filterLower.split(',', Qt::SkipEmptyParts);

  bool generalMatches = true;
  for (const QString& clause : clauses) {
    const QString trimmedClause = clause.trimmed();
    if (trimmedClause.isEmpty()) {
      continue;
    }

    if (trimmedClause.startsWith("qos=", Qt::CaseInsensitive)) {
      const QString qosValue = trimmedClause.mid(4).trimmed();
      bool ok = false;
      const int expectedQos = qosValue.toInt(&ok);
      if (!ok) {
        generalMatches = false;
        break;
      }
      generalMatches = generalMatches && itemMatchesQos(item, expectedQos);
    }
    else if (trimmedClause.startsWith("retain=", Qt::CaseInsensitive) ||
             trimmedClause.startsWith("bretain=", Qt::CaseInsensitive)) {
      const QString retainValue = trimmedClause.startsWith("bretain=", Qt::CaseInsensitive)
                                     ? trimmedClause.mid(8).trimmed().toLower()
                                     : trimmedClause.mid(7).trimmed().toLower();
      const bool expectedRetain = retainValue == "true";
      if (retainValue != "true" && retainValue != "false") {
        generalMatches = false;
        break;
      }
      generalMatches = generalMatches && itemMatchesRetain(item, expectedRetain);
    }
    else if (trimmedClause.startsWith("topic=", Qt::CaseInsensitive)) {
      QString topicPattern = trimmedClause.mid(6).trimmed();
      if (topicPattern.startsWith('"') && topicPattern.endsWith('"') && topicPattern.size() >= 2) {
        topicPattern = topicPattern.mid(1, topicPattern.size() - 2).trimmed();
      }
      generalMatches = generalMatches &&
                       (matchesTopicFilter(topic, topicPattern) ||
                        matchesTopicFilter(label, topicPattern) ||
                        matchesTopicFilter(branchPath, topicPattern));
    }
    else {
      generalMatches = generalMatches && search.contains(trimmedClause, Qt::CaseInsensitive);
    }
  }

  const bool topicMatches = topicFilterLower.isEmpty() ||
                            matchesTopicFilter(topic, topicFilterLower) ||
                            matchesTopicFilter(label, topicFilterLower) ||
                            matchesTopicFilter(branchPath, topicFilterLower);
  return generalMatches && topicMatches;
}

} // namespace

CFrmMqttExplorer::CFrmMqttExplorer(QWidget* parent, json* pconn)
  : QDialog(parent)
  , m_mosq(nullptr)
  , m_connected(false)
  , m_connecting(false)
  , m_tlsEnabled(false)
  , m_verifyPeer(true)
  , m_port(1883)
  , m_keepAlive(60)
  , m_messageFlushTimer(nullptr)
  , m_messageRenderCount(0)
  , m_receivePaused(false)
  , m_lastRenderedMessageCount(0)
{
  if (nullptr != pconn) {
    m_conn = *pconn;
  }

  setupUi();
  configureFromConnection();
  connectToBroker();
}

CFrmMqttExplorer::~CFrmMqttExplorer()
{
  disconnectFromBroker();
}

void
CFrmMqttExplorer::setupUi()
{
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("MQTT raw explorer"));
  resize(1200, 760);

  auto* mainLayout = new QVBoxLayout(this);

  m_menuBar       = new QMenuBar(this);
  auto* toolBar   = new QToolBar(tr("MQTT tools"), this);
  toolBar->setMovable(false);
  toolBar->setIconSize(QSize(16, 16));
  toolBar->setStyleSheet("QToolBar { spacing: 6px; padding: 4px; background: #f8fafc; border: 0; }"
                         "QToolButton { background-color: #e2e8f0; border: 1px solid #cbd5e1; border-radius: 6px; padding: 4px 8px; }"
                         "QToolButton:hover { background-color: #cbd5e1; }");
  toolBar->addWidget(new QLabel(tr("MQTT"), this));
  toolBar->addSeparator();
  m_actConnect = toolBar->addAction(tr("Connect"));
  connect(m_actConnect, &QAction::triggered, this, &CFrmMqttExplorer::onConnectClicked);
  m_actPauseReceive = toolBar->addAction(tr("Pause"));
  connect(m_actPauseReceive, &QAction::triggered, this, &CFrmMqttExplorer::onPauseReceiveClicked);
  auto* subscribeAction = toolBar->addAction(tr("Subscribe"));
  connect(subscribeAction, &QAction::triggered, this, &CFrmMqttExplorer::onMenuSubscribe);
  mainLayout->setMenuBar(m_menuBar);
  mainLayout->addWidget(toolBar);
  m_subscribeMenu = m_menuBar->addMenu(tr("&Subscribe"));
  m_actSubscribe  = m_subscribeMenu->addAction(tr("Subscribe current topic"));
  m_actUnsubscribe = m_subscribeMenu->addAction(tr("Unsubscribe current topic"));
  m_subscribeMenu->addSeparator();
  m_actSubscribeConfigured = m_subscribeMenu->addAction(tr("Subscribe configured topics"));
  m_actClearSubscriptions  = m_subscribeMenu->addAction(tr("Clear subscriptions"));
  mainLayout->setMenuBar(m_menuBar);

  auto* connBox    = new QGroupBox(tr("Connection"), this);
  connBox->setStyleSheet("QGroupBox { background-color: #fefce8; }");
  auto* connLayout = new QHBoxLayout(connBox);
  m_lblStatus      = new QLabel(tr("Disconnected"), connBox);
  m_btnConnect     = new QPushButton(tr("Connect"), connBox);
  m_btnPauseReceive = new QPushButton(tr("Pause"), connBox);
  m_btnPauseReceive->setCheckable(true);
  m_btnPauseReceive->setToolTip(tr("Pause or resume incoming MQTT traffic"));
  m_comboReceiveMode = new QComboBox(connBox);
  m_comboReceiveMode->addItem(tr("Append all"), ReceiveAppend);
  m_comboReceiveMode->addItem(tr("Replace per topic"), ReceiveReplace);
  m_comboReceiveMode->setCurrentIndex(0);
  m_comboReceiveMode->setToolTip(tr("Choose whether new messages append or replace the latest message for the same topic"));
  connLayout->addWidget(m_lblStatus, 1);
  connLayout->addWidget(m_comboReceiveMode);
  connLayout->addWidget(m_btnPauseReceive);
  connLayout->addWidget(m_btnConnect);
  mainLayout->addWidget(connBox);

  auto* subscribeBox    = new QGroupBox(tr("Subscribe to topics"), this);
  subscribeBox->setStyleSheet("QGroupBox { background-color: #fefce8; }");
  auto* subscribeLayout = new QGridLayout(subscribeBox);
  m_editSubscribeTopic  = new QLineEdit(subscribeBox);
  m_editSubscribeTopic->setPlaceholderText(tr("Topic (supports + and # wildcards)"));
  m_comboSubscribeQos = new QComboBox(subscribeBox);
  m_comboSubscribeQos->addItems({ "0", "1", "2" });
  m_btnSubscribe   = new QPushButton(tr("Subscribe"), subscribeBox);
  m_btnUnsubscribe = new QPushButton(tr("Unsubscribe"), subscribeBox);
  subscribeLayout->addWidget(new QLabel(tr("Topic:"), subscribeBox), 0, 0);
  subscribeLayout->addWidget(m_editSubscribeTopic, 0, 1);
  subscribeLayout->addWidget(new QLabel(tr("QoS:"), subscribeBox), 0, 2);
  subscribeLayout->addWidget(m_comboSubscribeQos, 0, 3);
  subscribeLayout->addWidget(m_btnSubscribe, 0, 4);
  subscribeLayout->addWidget(m_btnUnsubscribe, 0, 5);
  mainLayout->addWidget(subscribeBox);

  auto* topicFilterLayout = new QHBoxLayout();
  topicFilterLayout->setContentsMargins(0, 4, 0, 4);
  topicFilterLayout->addWidget(new QLabel(tr("Topic search:"), this));
  m_editTopicFilter = new QLineEdit(this);
  m_editTopicFilter->setPlaceholderText(tr("Search by topic path (use * as wildcard)"));
  topicFilterLayout->addWidget(m_editTopicFilter, 1);
  mainLayout->addLayout(topicFilterLayout);

  auto* filterLayout = new QHBoxLayout();
  filterLayout->setContentsMargins(0, 4, 0, 4);
  filterLayout->addWidget(new QLabel(tr("Filter:"), this));
  m_editFilter = new QLineEdit(this);
  m_editFilter->setPlaceholderText(tr("Filter by text, qos=n, retain=true/false, topic=..."));
  filterLayout->addWidget(m_editFilter, 1);
  m_btnSave = new QPushButton(tr("Save selected"), this);
  filterLayout->addWidget(m_btnSave);
  mainLayout->addLayout(filterLayout);

  auto* splitter = new QSplitter(Qt::Horizontal, this);
  m_tree          = new QTreeWidget(splitter);
  m_tree->setColumnCount(2);
  m_tree->setHeaderLabels({ tr("Topic/Message"), tr("Message data") });
  m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
  m_tree->setAlternatingRowColors(true);
  m_tree->setUniformRowHeights(true);
  m_tree->setWordWrap(false);
  m_tree->setExpandsOnDoubleClick(true);
  m_tree->setAnimated(false);
  m_tree->setRootIsDecorated(true);
  m_tree->setMouseTracking(true);
  m_tree->header()->setSectionsMovable(true);
  m_tree->header()->setSectionResizeMode(0, QHeaderView::Interactive);
  m_tree->header()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_tree->header()->resizeSection(0, 520);
  m_tree->header()->setStretchLastSection(true);

  m_detailsTree = new QTreeWidget(splitter);
  m_detailsTree->setHeaderLabels({ tr("Field"), tr("Value") });
  m_detailsTree->setAlternatingRowColors(true);
  m_detailsTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_detailsTree->header()->setSectionResizeMode(1, QHeaderView::Stretch);

  splitter->setStretchFactor(0, 5);
  splitter->setStretchFactor(1, 2);
  mainLayout->addWidget(splitter, 1);

  auto* publishBox    = new QGroupBox(tr("Publish"), this);
  publishBox->setStyleSheet("QGroupBox { background-color: #fefce8; }");
  auto* publishLayout = new QGridLayout(publishBox);
  m_editPublishTopic  = new QLineEdit(publishBox);
  m_editPublishTopic->setPlaceholderText(tr("Topic"));
  m_comboPublishQos = new QComboBox(publishBox);
  m_comboPublishQos->addItems({ "0", "1", "2" });
  m_chkPublishRetain = new QCheckBox(tr("Retain"), publishBox);
  m_editPublishPayload = new QPlainTextEdit(publishBox);
  m_editPublishPayload->setPlaceholderText(tr("Payload"));
  m_btnPublish = new QPushButton(tr("Publish"), publishBox);
  publishLayout->addWidget(new QLabel(tr("Topic:"), publishBox), 0, 0);
  publishLayout->addWidget(m_editPublishTopic, 0, 1, 1, 3);
  publishLayout->addWidget(new QLabel(tr("QoS:"), publishBox), 0, 4);
  publishLayout->addWidget(m_comboPublishQos, 0, 5);
  publishLayout->addWidget(m_chkPublishRetain, 0, 6);
  publishLayout->addWidget(new QLabel(tr("Payload:"), publishBox), 1, 0);
  publishLayout->addWidget(m_editPublishPayload, 1, 1, 1, 6);
  publishLayout->addWidget(m_btnPublish, 2, 6);
  publishBox->setMaximumHeight(240);
  publishBox->setMinimumHeight(180);
  m_listPublishTopics = new QListWidget(publishBox);
  m_listPublishTopics->setAlternatingRowColors(true);
  m_listPublishTopics->setSelectionMode(QAbstractItemView::SingleSelection);
  m_btnAddPublishTopic = new QPushButton(tr("Add topic"), publishBox);
  m_btnUsePublishTopic = new QPushButton(tr("Use selected"), publishBox);
  m_btnClearPublishTopics = new QPushButton(tr("Clear"), publishBox);
  m_btnSavePublishTopics = new QPushButton(tr("Save"), publishBox);
  m_btnLoadPublishTopics = new QPushButton(tr("Load"), publishBox);
  publishLayout->addWidget(new QLabel(tr("Publish topic list:"), publishBox), 2, 0);
  publishLayout->addWidget(m_listPublishTopics, 2, 1, 2, 3);
  publishLayout->addWidget(m_btnAddPublishTopic, 2, 4);
  publishLayout->addWidget(m_btnUsePublishTopic, 2, 5);
  publishLayout->addWidget(m_btnClearPublishTopics, 2, 6);
  publishLayout->addWidget(m_btnSavePublishTopics, 3, 4);
  publishLayout->addWidget(m_btnLoadPublishTopics, 3, 5);
  mainLayout->addWidget(publishBox);

  setStyleSheet(
    "QWidget { font-family: 'Segoe UI', 'Liberation Sans', sans-serif; color: #0f172a; }"
    "QGroupBox { background-color: #f8fafc; border: 1px solid #d4dbe5; border-radius: 10px; margin-top: 8px; "
    "padding-top: 8px; font-weight: 600; color: #0f172a; }"
    "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }"
    "QTreeWidget, QListWidget, QPlainTextEdit, QLineEdit, QComboBox { "
    "background-color: #ffffff; border: 1px solid #cbd5e1; border-radius: 8px; padding: 4px; }"
    "QPushButton { background-color: #2563eb; color: white; border: 1px solid #1d4ed8; border-radius: 8px; "
    "padding: 5px 12px; font-weight: 600; }"
    "QPushButton:hover { background-color: #1d4ed8; }"
    "QPushButton:checked, QPushButton:pressed { background-color: #0f766e; border-color: #0f766e; }"
    "QPushButton:disabled { background-color: #94a3b8; border-color: #94a3b8; }"
    "QHeaderView::section { background-color: #eef2ff; padding: 5px; border: 0; border-right: 1px solid #dbeafe; }");

  connect(m_btnConnect,
          &QPushButton::clicked,
          this,
          &CFrmMqttExplorer::onConnectClicked);
  connect(m_btnPauseReceive,
          &QPushButton::clicked,
          this,
          &CFrmMqttExplorer::onPauseReceiveClicked);
  connect(m_comboReceiveMode,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &CFrmMqttExplorer::onReceiveModeChanged);
  connect(m_btnPublish,
          &QPushButton::clicked,
          this,
          &CFrmMqttExplorer::onPublishClicked);
  connect(m_btnSubscribe,
          &QPushButton::clicked,
          this,
          &CFrmMqttExplorer::onSubscribeClicked);
  connect(m_btnUnsubscribe,
          &QPushButton::clicked,
          this,
          &CFrmMqttExplorer::onUnsubscribeClicked);
  connect(m_editFilter,
          &QLineEdit::textChanged,
          this,
          &CFrmMqttExplorer::onFilterChanged);
  connect(m_editTopicFilter,
          &QLineEdit::textChanged,
          this,
          &CFrmMqttExplorer::onTopicFilterChanged);
  connect(m_tree,
          &QTreeWidget::itemSelectionChanged,
          this,
          &CFrmMqttExplorer::onTreeSelectionChanged);
  m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_tree,
          &QTreeWidget::itemEntered,
          this,
          [&](QTreeWidgetItem* item, int column) {
            if (nullptr != item && column == 0) {
              const QString topic = item->data(0, RoleTopic).toString();
              if (!topic.isEmpty()) {
                m_tree->setToolTip(topic);
              }
            }
          });
  connect(m_btnAddPublishTopic, &QPushButton::clicked, this, &CFrmMqttExplorer::onAddPublishTopic);
  connect(m_btnUsePublishTopic,
          &QPushButton::clicked,
          this,
          &CFrmMqttExplorer::onUseSelectedPublishTopic);
  connect(m_btnClearPublishTopics,
          &QPushButton::clicked,
          this,
          &CFrmMqttExplorer::onClearPublishTopics);
  connect(m_btnSavePublishTopics,
          &QPushButton::clicked,
          this,
          &CFrmMqttExplorer::onSavePublishTopics);
  connect(m_btnLoadPublishTopics,
          &QPushButton::clicked,
          this,
          &CFrmMqttExplorer::onLoadPublishTopics);
  connect(m_listPublishTopics,
          &QListWidget::itemDoubleClicked,
          this,
          &CFrmMqttExplorer::onUseSelectedPublishTopic);
  connect(m_btnSave, &QPushButton::clicked, this, &CFrmMqttExplorer::onSaveSelected);
  connect(m_btnSave, &QPushButton::clicked, this, &CFrmMqttExplorer::onSaveSelectedEvent);
  m_messageFlushTimer = new QTimer(this);
  m_messageFlushTimer->setInterval(200);
  m_messageFlushTimer->setSingleShot(false);
  connect(m_messageFlushTimer, &QTimer::timeout, this, &CFrmMqttExplorer::flushPendingMessages);
  connect(m_actSubscribe, &QAction::triggered, this, &CFrmMqttExplorer::onMenuSubscribe);
  connect(m_actUnsubscribe, &QAction::triggered, this, &CFrmMqttExplorer::onMenuUnsubscribe);
  connect(m_actSubscribeConfigured,
          &QAction::triggered,
          this,
          &CFrmMqttExplorer::onMenuSubscribeConfigured);
  connect(m_actClearSubscriptions,
          &QAction::triggered,
          this,
          &CFrmMqttExplorer::onMenuClearSubscriptions);
}

void
CFrmMqttExplorer::updateConnectionUiState()
{
  if (nullptr == m_btnConnect) {
    return;
  }

  if (m_connected) {
    m_btnConnect->setText(tr("Disconnect"));
    m_btnConnect->setToolTip(tr("Disconnect from the MQTT broker"));
  }
  else if (m_connecting) {
    m_btnConnect->setText(tr("Connecting..."));
    m_btnConnect->setToolTip(tr("Connecting to the MQTT broker"));
  }
  else {
    m_btnConnect->setText(tr("Connect"));
    m_btnConnect->setToolTip(tr("Connect to the MQTT broker"));
  }

  if (nullptr != m_actConnect) {
    m_actConnect->setText(m_connected ? tr("Disconnect") : (m_connecting ? tr("Connecting...") : tr("Connect")));
  }
}

void
CFrmMqttExplorer::updateReceiveUiState()
{
  if (nullptr == m_btnPauseReceive) {
    return;
  }

  m_btnPauseReceive->setCheckable(true);
  m_btnPauseReceive->setChecked(m_receivePaused);
  m_btnPauseReceive->setText(m_receivePaused ? tr("Resume") : tr("Pause"));
  m_btnPauseReceive->setToolTip(m_receivePaused ? tr("Resume incoming MQTT traffic")
                                               : tr("Pause incoming MQTT traffic"));

  if (nullptr != m_actPauseReceive) {
    m_actPauseReceive->setText(m_receivePaused ? tr("Resume") : tr("Pause"));
  }
}

void
CFrmMqttExplorer::configureFromConnection()
{
  if (m_conn.contains("name") && m_conn["name"].is_string()) {
    setWindowTitle(tr("MQTT raw explorer - %1")
                     .arg(QString::fromStdString(m_conn["name"].get<std::string>())));
  }

  m_host = "localhost";
  if (m_conn.contains("host") && m_conn["host"].is_string()) {
    m_host = QString::fromStdString(m_conn["host"].get<std::string>());
  }

  if (m_conn.contains("port") && m_conn["port"].is_number_integer()) {
    m_port = m_conn["port"].get<int>();
  }

  if (m_conn.contains("keepalive") && m_conn["keepalive"].is_number_integer()) {
    m_keepAlive = std::max(0, m_conn["keepalive"].get<int>());
  }

  if (m_conn.contains("clientid") && m_conn["clientid"].is_string()) {
    m_clientId = QString::fromStdString(m_conn["clientid"].get<std::string>());
  }
  if (m_clientId.isEmpty()) {
    m_clientId = QString("vscpworks-explorer-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
  }

  if (m_conn.contains("user") && m_conn["user"].is_string()) {
    m_user = QString::fromStdString(m_conn["user"].get<std::string>());
  }
  if (m_conn.contains("password") && m_conn["password"].is_string()) {
    m_password = QString::fromStdString(m_conn["password"].get<std::string>());
  }

  if (m_conn.contains("btls") && m_conn["btls"].is_boolean()) {
    m_tlsEnabled = m_conn["btls"].get<bool>();
  }
  if (m_conn.contains("bverifypeer") && m_conn["bverifypeer"].is_boolean()) {
    m_verifyPeer = m_conn["bverifypeer"].get<bool>();
  }
  if (m_conn.contains("cafile") && m_conn["cafile"].is_string()) {
    m_cafile = QString::fromStdString(m_conn["cafile"].get<std::string>());
  }
  if (m_conn.contains("capath") && m_conn["capath"].is_string()) {
    m_capath = QString::fromStdString(m_conn["capath"].get<std::string>());
  }
  if (m_conn.contains("certfile") && m_conn["certfile"].is_string()) {
    m_certfile = QString::fromStdString(m_conn["certfile"].get<std::string>());
  }
  if (m_conn.contains("keyfile") && m_conn["keyfile"].is_string()) {
    m_keyfile = QString::fromStdString(m_conn["keyfile"].get<std::string>());
  }
  if (m_conn.contains("pwkeyfile") && m_conn["pwkeyfile"].is_string()) {
    m_pwkeyfile = QString::fromStdString(m_conn["pwkeyfile"].get<std::string>());
  }

  const QUrl url(m_host);
  if (url.isValid() && !url.scheme().isEmpty() && !url.host().isEmpty()) {
    m_host = url.host();
    if (url.port() > 0) {
      m_port = url.port();
    }
    if (!m_conn.contains("btls") && (url.scheme() == "mqtts" || url.scheme() == "ssl")) {
      m_tlsEnabled = true;
    }
  }

  if (m_tlsEnabled && m_port == 1883 && !m_conn.contains("port")) {
    m_port = 8883;
  }

  if (m_conn.contains("subscribe") && m_conn["subscribe"].is_array()) {
    for (const auto& entry : m_conn["subscribe"]) {
      if (entry.contains("topic") && entry["topic"].is_string()) {
        m_initialSubscriptions.insert(
          QString::fromStdString(entry["topic"].get<std::string>()));
      }
    }
  }
  if (m_conn.contains("subscriptions") && m_conn["subscriptions"].is_array()) {
    for (const auto& entry : m_conn["subscriptions"]) {
      if (entry.contains("topic") && entry["topic"].is_string()) {
        m_initialSubscriptions.insert(
          QString::fromStdString(entry["topic"].get<std::string>()));
      }
    }
  }

  if (m_conn.contains("publish") && m_conn["publish"].is_array()) {
    for (const auto& entry : m_conn["publish"]) {
      if (entry.contains("topic") && entry["topic"].is_string()) {
        addPublishTopicIfMissing(QString::fromStdString(entry["topic"].get<std::string>()));
      }
    }
  }

  if (m_conn.contains("publishing") && m_conn["publishing"].is_array()) {
    for (const auto& entry : m_conn["publishing"]) {
      if (entry.contains("topic") && entry["topic"].is_string()) {
        addPublishTopicIfMissing(QString::fromStdString(entry["topic"].get<std::string>()));
      }
    }
  }
}

bool
CFrmMqttExplorer::connectToBroker()
{
  if (m_connecting || isConnected()) {
    disconnectFromBroker();
    return false;
  }

  disconnectFromBroker();
  m_connecting = true;
  updateConnectionUiState();

  mosquitto_lib_init();
  m_mosq = mosquitto_new(m_clientId.toUtf8().constData(), true, this);
  if (nullptr == m_mosq) {
    setStatus(tr("Failed to create MQTT client"), true);
    return false;
  }

  mosquitto_connect_callback_set(m_mosq, &CFrmMqttExplorer::onMosquittoConnectStatic);
  mosquitto_disconnect_callback_set(m_mosq,
                                    &CFrmMqttExplorer::onMosquittoDisconnectStatic);
  mosquitto_message_callback_set(m_mosq, &CFrmMqttExplorer::onMosquittoMessageStatic);

  if (!m_user.isEmpty()) {
    const int rc = mosquitto_username_pw_set(m_mosq,
                                             m_user.toUtf8().constData(),
                                             m_password.toUtf8().constData());
    if (MOSQ_ERR_SUCCESS != rc) {
      setStatus(tr("Failed to set username/password: %1")
                  .arg(QString::fromUtf8(mosquitto_strerror(rc))),
                true);
      disconnectFromBroker();
      return false;
    }
  }

  if (m_tlsEnabled) {
    applyTlsSettings();
  }

  const int loopRc = mosquitto_loop_start(m_mosq);
  if (MOSQ_ERR_SUCCESS != loopRc) {
    setStatus(tr("Failed to start MQTT loop: %1")
                .arg(QString::fromUtf8(mosquitto_strerror(loopRc))),
              true);
    disconnectFromBroker();
    return false;
  }

  const int connectRc =
    mosquitto_connect_async(m_mosq, m_host.toUtf8().constData(), m_port, std::max(5, m_keepAlive));
  if (MOSQ_ERR_SUCCESS != connectRc) {
    setStatus(tr("Failed to connect to %1:%2 - %3")
                .arg(m_host)
                .arg(m_port)
                .arg(QString::fromUtf8(mosquitto_strerror(connectRc))),
              true);
    disconnectFromBroker();
    return false;
  }

  setStatus(tr("Connecting to %1:%2 ...").arg(m_host).arg(m_port));
  updateConnectionUiState();
  return true;
}

void
CFrmMqttExplorer::disconnectFromBroker()
{
  if (nullptr != m_mosq) {
    mosquitto_disconnect(m_mosq);
    mosquitto_loop_stop(m_mosq, false);
    mosquitto_destroy(m_mosq);
    m_mosq = nullptr;
  }

  m_connecting = false;
  m_connected = false;
  updateConnectionUiState();
}

bool
CFrmMqttExplorer::isConnected() const
{
  return m_connected && (nullptr != m_mosq);
}

void
CFrmMqttExplorer::onConnectClicked()
{
  if (m_connecting || isConnected()) {
    disconnectFromBroker();
    setStatus(tr("Disconnected"));
    return;
  }

  connectToBroker();
}

bool
CFrmMqttExplorer::subscribeTopic(const QString& topic, int qos)
{
  const QString t = trimmedTopic(topic);
  if (t.isEmpty() || nullptr == m_mosq) {
    return false;
  }

  const int rc = mosquitto_subscribe(m_mosq, nullptr, t.toUtf8().constData(), qos);
  if (MOSQ_ERR_SUCCESS != rc) {
    setStatus(tr("Subscribe failed for '%1': %2")
                .arg(t)
                .arg(QString::fromUtf8(mosquitto_strerror(rc))),
              true);
    return false;
  }

  m_subscriptions.insert(t);
  setStatus(tr("Subscribed to '%1'").arg(t));
  return true;
}

bool
CFrmMqttExplorer::unsubscribeTopic(const QString& topic)
{
  const QString t = trimmedTopic(topic);
  if (t.isEmpty() || nullptr == m_mosq) {
    return false;
  }

  const int rc = mosquitto_unsubscribe(m_mosq, nullptr, t.toUtf8().constData());
  if (MOSQ_ERR_SUCCESS != rc) {
    setStatus(tr("Unsubscribe failed for '%1': %2")
                .arg(t)
                .arg(QString::fromUtf8(mosquitto_strerror(rc))),
              true);
    return false;
  }

  m_subscriptions.remove(t);
  setStatus(tr("Unsubscribed from '%1'").arg(t));
  return true;
}

bool
CFrmMqttExplorer::publishMessage(const QString& topic,
                                 const QByteArray& payload,
                                 int qos,
                                 bool retain)
{
  const QString t = trimmedTopic(topic);
  if (t.isEmpty() || nullptr == m_mosq) {
    return false;
  }

  const int rc = mosquitto_publish(m_mosq,
                                   nullptr,
                                   t.toUtf8().constData(),
                                   payload.size(),
                                   payload.constData(),
                                   qos,
                                   retain);
  if (MOSQ_ERR_SUCCESS != rc) {
    setStatus(tr("Publish failed for '%1': %2")
                .arg(t)
                .arg(QString::fromUtf8(mosquitto_strerror(rc))),
              true);
    return false;
  }

  setStatus(tr("Published to '%1'").arg(t));
  return true;
}

void
CFrmMqttExplorer::onPublishClicked()
{
  if (!isConnected()) {
    setStatus(tr("Not connected"), true);
    return;
  }

  if (!publishMessage(m_editPublishTopic->text(),
                      m_editPublishPayload->toPlainText().toUtf8(),
                      m_comboPublishQos->currentText().toInt(),
                      m_chkPublishRetain->isChecked())) {
    return;
  }

  addPublishTopicIfMissing(m_editPublishTopic->text());
}

void
CFrmMqttExplorer::onSubscribeClicked()
{
  if (!isConnected()) {
    setStatus(tr("Not connected"), true);
    return;
  }

  if (subscribeTopic(m_editSubscribeTopic->text(), m_comboSubscribeQos->currentText().toInt())) {
    addSubscriptionIfMissing(m_editSubscribeTopic->text());
  }
}

void
CFrmMqttExplorer::onUnsubscribeClicked()
{
  if (!isConnected()) {
    setStatus(tr("Not connected"), true);
    return;
  }

  QString topic = trimmedTopic(m_editSubscribeTopic->text());
  if (topic.isEmpty()) {
    const auto selected = m_tree->selectedItems();
    if (!selected.isEmpty()) {
      topic = selected.first()->data(0, RoleTopic).toString();
    }
  }

  if (topic.isEmpty()) {
    setStatus(tr("No topic selected"), true);
    return;
  }

  unsubscribeTopic(topic);
}

void
CFrmMqttExplorer::addPublishTopicIfMissing(const QString& topic)
{
  const QString t = trimmedTopic(topic);
  if (t.isEmpty() || m_publishTopics.contains(t)) {
    return;
  }

  m_publishTopics.insert(t);
  m_listPublishTopics->addItem(t);
  m_listPublishTopics->sortItems();
}

void
CFrmMqttExplorer::addSubscriptionIfMissing(const QString& topic)
{
  const QString t = trimmedTopic(topic);
  if (t.isEmpty()) {
    return;
  }

  m_initialSubscriptions.insert(t);
}

void
CFrmMqttExplorer::onAddPublishTopic()
{
  bool ok = false;
  const QString topic =
    QInputDialog::getText(this, tr("Add publish topic"), tr("Topic:"), QLineEdit::Normal, "", &ok);
  if (!ok) {
    return;
  }

  addPublishTopicIfMissing(topic);
}

void
CFrmMqttExplorer::onUseSelectedPublishTopic()
{
  const auto* item = m_listPublishTopics->currentItem();
  if (nullptr == item) {
    return;
  }

  m_editPublishTopic->setText(item->text());
}

void
CFrmMqttExplorer::onClearPublishTopics()
{
  m_publishTopics.clear();
  m_listPublishTopics->clear();
}

void
CFrmMqttExplorer::onSavePublishTopics()
{
  const QString path =
    QFileDialog::getSaveFileName(this,
                                 tr("Save publish topics"),
                                 QDir::homePath() + "/mqtt-publish-topics.json",
                                 tr("JSON files (*.json);;Text files (*.txt);;All files (*)"));
  if (path.isEmpty()) {
    return;
  }

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    setStatus(tr("Unable to save publish topics"), true);
    return;
  }

  if (path.endsWith(".txt", Qt::CaseInsensitive)) {
    QStringList topics(m_publishTopics.values());
    topics.sort(Qt::CaseInsensitive);
    for (const auto& topic : topics) {
      file.write(topic.toUtf8());
      file.write("\n");
    }
  }
  else {
    QJsonArray arr;
    QStringList topics(m_publishTopics.values());
    topics.sort(Qt::CaseInsensitive);
    for (const auto& topic : topics) {
      arr.append(topic);
    }
    QJsonObject root;
    root["topics"] = arr;
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
  }

  setStatus(tr("Saved publish topics to %1").arg(path));
}

void
CFrmMqttExplorer::onLoadPublishTopics()
{
  const QString path =
    QFileDialog::getOpenFileName(this,
                                 tr("Load publish topics"),
                                 QDir::homePath(),
                                 tr("JSON files (*.json);;Text files (*.txt);;All files (*)"));
  if (path.isEmpty()) {
    return;
  }

  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    setStatus(tr("Unable to load publish topics"), true);
    return;
  }

  m_publishTopics.clear();
  m_listPublishTopics->clear();

  const QByteArray data = file.readAll();
  QJsonParseError jsonErr;
  const auto doc = QJsonDocument::fromJson(data, &jsonErr);
  if (QJsonParseError::NoError == jsonErr.error && doc.isObject()) {
    const auto root = doc.object();
    if (root.contains("topics") && root["topics"].isArray()) {
      for (const auto& value : root["topics"].toArray()) {
        if (value.isString()) {
          addPublishTopicIfMissing(value.toString());
        }
      }
    }
  }
  else {
    const QStringList lines = QString::fromUtf8(data).split('\n');
    for (const auto& line : lines) {
      addPublishTopicIfMissing(line);
    }
  }

  setStatus(tr("Loaded publish topics from %1").arg(path));
}

void
CFrmMqttExplorer::onPauseReceiveClicked()
{
  m_receivePaused = !m_receivePaused;
  updateReceiveUiState();
  setStatus(m_receivePaused ? tr("Receiving paused") : tr("Receiving resumed"));

  if (!m_receivePaused && !m_pendingMessages.isEmpty()) {
    flushPendingMessages();
  }
}

void
CFrmMqttExplorer::onReceiveModeChanged(int index)
{
  const QVariant data = m_comboReceiveMode->itemData(index);
  const int mode = data.isValid() ? data.toInt() : ReceiveAppend;
  Q_UNUSED(mode);
  setStatus(tr("Receive mode updated"));
}

void
CFrmMqttExplorer::onSaveSelectedEvent()
{
  const auto selected = m_tree->selectedItems();
  if (selected.isEmpty()) {
    return;
  }

  const QString text = buildDetailsText(selected.first());
  if (text.isEmpty()) {
    return;
  }

  const QString path = QFileDialog::getSaveFileName(this,
                                                    tr("Save selected MQTT event"),
                                                    QDir::homePath() + "/mqtt-event.txt",
                                                    tr("Text files (*.txt);;All files (*)"));
  if (path.isEmpty()) {
    return;
  }

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this,
                         tr("MQTT raw explorer"),
                         tr("Failed to save selected event to '%1'.").arg(path));
    return;
  }

  file.write(text.toUtf8());
  file.close();
  setStatus(tr("Saved selected event to %1").arg(path));
}

void
CFrmMqttExplorer::onMenuSubscribe()
{
  onSubscribeClicked();
}

void
CFrmMqttExplorer::onMenuUnsubscribe()
{
  onUnsubscribeClicked();
}

void
CFrmMqttExplorer::onMenuSubscribeConfigured()
{
  subscribeConfiguredTopics();
}

void
CFrmMqttExplorer::onMenuClearSubscriptions()
{
  m_subscriptions.clear();
  setStatus(tr("Subscription tracking cleared"));
}

void
CFrmMqttExplorer::pruneTopicNode(QTreeWidgetItem* item)
{
  if (nullptr == item) {
    return;
  }

  const QString searchKey = item->data(0, RoleSearchText).toString();
  if (!searchKey.isEmpty()) {
    m_topicNodeByPath.remove(searchKey);
  }

  for (int i = 0; i < item->childCount(); ++i) {
    pruneTopicNode(item->child(i));
  }
}

void
CFrmMqttExplorer::pruneTopicNodeIndexIfNeeded()
{
  constexpr int kMaxTopLevelTopics = 100;
  while (m_tree->topLevelItemCount() >= kMaxTopLevelTopics) {
    auto* oldest = m_tree->takeTopLevelItem(0);
    if (nullptr == oldest) {
      break;
    }
    pruneTopicNode(oldest);
    delete oldest;
  }
}

QTreeWidgetItem*
CFrmMqttExplorer::ensureTopicPath(const QString& topic)
{
  QString normalized = topic;
  if (normalized.isEmpty()) {
    normalized = "/";
  }

  if (m_topicNodeByPath.contains(normalized)) {
    return m_topicNodeByPath.value(normalized);
  }

  const QStringList parts = normalized.split('/', Qt::KeepEmptyParts);
  QString currentPath;
  QTreeWidgetItem* parent = nullptr;

  for (int i = 0; i < parts.size(); ++i) {
    const QString segment = parts.at(i).isEmpty() ? "/" : parts.at(i);
    currentPath = currentPath.isEmpty() ? segment : currentPath + "/" + segment;

    if (!m_topicNodeByPath.contains(currentPath)) {
      auto* item = new QTreeWidgetItem({ segment, "" });
      item->setForeground(0, QColor("#2563eb"));
      item->setForeground(1, QColor("#64748b"));
      item->setData(0, RoleKind, KindTopicNode);
      item->setData(0, RoleTopic, normalized);
      item->setData(0, RoleSearchText, currentPath.toLower());
      item->setData(0, RoleMessageCount, 0);
      if (nullptr == parent) {
        pruneTopicNodeIndexIfNeeded();
        m_tree->addTopLevelItem(item);
      }
      else {
        parent->addChild(item);
      }
      m_topicNodeByPath.insert(currentPath, item);
    }
    parent = m_topicNodeByPath.value(currentPath);
  }

  m_topicNodeByPath.insert(normalized, parent);
  return parent;
}

void
CFrmMqttExplorer::updateTopicNodeWithMessage(QTreeWidgetItem* topicNode,
                                             const QString& topic,
                                             const QByteArray& payload,
                                             const QString& formattedPayload,
                                             const QString& format,
                                             bool retained,
                                             int qos,
                                             int mid)
{
  if (nullptr == topicNode) {
    return;
  }


  const QString previousPayload = topicNode->data(0, RolePayloadRaw).toString();
  const bool changed = !previousPayload.isEmpty() && previousPayload != QString::fromUtf8(payload);
  QString preview = QString::fromUtf8(payload);
  preview.replace('\n', " ");
  preview.replace('\r', " ");
  preview = preview.trimmed();
  if (preview.size() > 140) {
    preview = preview.left(137) + "...";
  }
  topicNode->setForeground(0, QColor("#2563eb"));
  topicNode->setForeground(1, changed ? QColor("#16a34a") : QColor("#16a34a"));
  if (topicNode->data(0, RoleKind).toInt() == KindTopicNode) {
    topicNode->setText(1, QString());
    topicNode->setForeground(1, QColor("#64748b"));
  }
  else {
    topicNode->setText(1, preview.isEmpty() ? tr("<empty>") : preview);
    topicNode->setForeground(1, QColor("#16a34a"));
  }
  topicNode->setBackground(0, QColor("#eff6ff"));
  topicNode->setBackground(1, QColor("#f5f3ff"));
  topicNode->setData(0, RolePayloadRaw, QString::fromUtf8(payload));
  topicNode->setData(0, RolePayloadFormatted, formattedPayload);
  topicNode->setData(0, RolePayloadFormat, format);
  topicNode->setData(0, RoleRetained, retained);
  topicNode->setData(0, RolePayloadSize, payload.size());
  topicNode->setData(0, RoleQos, qos);
  topicNode->setData(0, RoleMid, mid);
  topicNode->setData(0, RoleTimestamp, QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
  topicNode->setData(0,
                     RoleSearchText,
                     buildSearchText(topic, payload, formattedPayload).toLower());

  for (QTreeWidgetItem* current = topicNode; nullptr != current; current = current->parent()) {
    const int count = current->data(0, RoleMessageCount).toInt();
    current->setData(0, RoleMessageCount, count + 1);
  }

  const auto selected = m_tree->selectedItems();
  if (!selected.isEmpty()) {
    auto* selectedItem = selected.first();
    const bool sameTopic = selectedItem->data(0, RoleTopic).toString() == topic ||
                           (selectedItem == topicNode) ||
                           (selectedItem->parent() == topicNode);
    if (sameTopic) {
      refreshSelectedDetails();
    }
  }
}

void
CFrmMqttExplorer::flashItemHighlight(QTreeWidgetItem* item)
{
  if (nullptr == item || nullptr == item->treeWidget()) {
    return;
  }

  const bool isTopicNode = item->data(0, RoleKind).toInt() == KindTopicNode;
  if (isTopicNode) {
    const QString label = item->text(0);
    if (!label.startsWith("● ")) {
      item->setText(0, QStringLiteral("● %1").arg(label));
    }
  }

  QTimer::singleShot(1200, item->treeWidget(), [item, isTopicNode]() {
    if (nullptr == item || nullptr == item->treeWidget()) {
      return;
    }

    if (isTopicNode) {
      QString label = item->text(0);
      if (label.startsWith("● ")) {
        label.remove(0, 2);
      }
      item->setText(0, label);
    }
  });
}

void
CFrmMqttExplorer::appendMessageNode(QTreeWidgetItem* topicNode,
                                    const QString& topic,
                                    const QByteArray& payload,
                                    const QString& formattedPayload,
                                    const QString& format,
                                    bool retained,
                                    int qos,
                                    int mid)
{
  if (nullptr == topicNode) {
    return;
  }

  const QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
  QString preview = QString::fromUtf8(payload);
  preview.replace('\n', " ");
  preview.replace('\r', " ");
  preview = preview.trimmed();
  if (preview.size() > 140) {
    preview = preview.left(137) + "...";
  }
  const QString topicDisplay = topic.isEmpty() ? tr("<empty>") : topic;
  const QString firstLine = topicDisplay;
  auto* msgItem = new QTreeWidgetItem({ firstLine, preview.isEmpty() ? tr("<empty>") : preview });

  const QString previousPayload = topicNode->childCount() > 0
                                    ? topicNode->child(topicNode->childCount() - 1)->data(0, RolePayloadRaw).toString()
                                    : QString();
  const bool valueChanged = !previousPayload.isEmpty() && previousPayload != QString::fromUtf8(payload);
  msgItem->setData(0, RoleKind, KindMessage);
  msgItem->setForeground(0, QColor(retained ? "#7c3aed" : "#0f766e"));
  msgItem->setForeground(1, valueChanged ? QColor("#16a34a") : QColor("#16a34a"));
  msgItem->setData(0, RoleTopic, topic);
  msgItem->setData(0, RolePayloadRaw, QString::fromUtf8(payload));
  msgItem->setData(0, RolePayloadFormatted, formattedPayload);
  msgItem->setData(0, RoleTimestamp, timestamp);
  msgItem->setData(0, RolePayloadFormat, format);
  msgItem->setData(0, RoleRetained, retained);
  msgItem->setData(0, RolePayloadSize, payload.size());
  msgItem->setData(0, RoleQos, qos);
  msgItem->setData(0, RoleMid, mid);
  const QString metaText = format + "\nqos:" + QString::number(qos) + "\n" + (retained ? "retained yes" : "retained no");
  msgItem->setData(0,
                   RoleSearchText,
                   (buildSearchText(topic, payload, formattedPayload) + "\n" + metaText).toLower());
  topicNode->addChild(msgItem);

  for (QTreeWidgetItem* current = topicNode; nullptr != current; current = current->parent()) {
    const int count = current->data(0, RoleMessageCount).toInt();
    current->setData(0, RoleMessageCount, count + 1);
  }

  if (m_comboReceiveMode->currentData().toInt() == ReceiveReplace) {
    flashItemHighlight(topicNode);
  }
  else {
    flashItemHighlight(topicNode);
  }

  const auto selected = m_tree->selectedItems();
  if (!selected.isEmpty()) {
    auto* selectedItem = selected.first();
    const bool sameTopic = selectedItem->data(0, RoleTopic).toString() == topic ||
                           (selectedItem == topicNode) ||
                           (selectedItem->parent() == topicNode);
    if (sameTopic) {
      refreshSelectedDetails();
    }
  }

  constexpr int kMaxMessagesPerTopic = 250;
  if (m_comboReceiveMode->currentData().toInt() == ReceiveReplace) {
    while (topicNode->childCount() > 0) {
      delete topicNode->takeChild(0);
    }
  }
  else {
    while (topicNode->childCount() > kMaxMessagesPerTopic) {
      delete topicNode->takeChild(0);
    }
  }

}

QString
CFrmMqttExplorer::formatPayloadForDisplay(const QByteArray& payload,
                                          QString* outFormat) const
{
  if (payload.size() > 256 * 1024) {
    if (nullptr != outFormat) {
      *outFormat = tr("Binary");
    }
    return tr("Payload skipped for performance");
  }

  const QString raw = QString::fromUtf8(payload);
  const QString trimmed = raw.trimmed();

  QJsonParseError jsonErr;
  const auto jsonDoc = QJsonDocument::fromJson(payload, &jsonErr);
  if (QJsonParseError::NoError == jsonErr.error &&
      (jsonDoc.isObject() || jsonDoc.isArray())) {
    if (nullptr != outFormat) {
      *outFormat = tr("JSON");
    }
    return QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Indented));
  }

  if (trimmed.startsWith('<') && trimmed.endsWith('>')) {
    const QString xmlDisplay = buildXmlDisplay(trimmed);
    if (!xmlDisplay.isEmpty()) {
      if (nullptr != outFormat) {
        *outFormat = tr("XML");
      }
      return xmlDisplay;
    }
  }

  if (nullptr != outFormat) {
    *outFormat = tr("Text");
  }
  return raw;
}

QString
CFrmMqttExplorer::buildXmlDisplay(const QString& xml) const
{
  QXmlStreamReader reader(xml);
  QString out;
  int depth = 0;

  while (!reader.atEnd()) {
    reader.readNext();
    if (reader.isStartElement()) {
      out += QString(depth * 2, ' ') + "<" + reader.name().toString();
      const auto attrs = reader.attributes();
      for (const auto& attr : attrs) {
        out += " " + attr.name().toString() + "=\"" + attr.value().toString() + "\"";
      }
      out += ">\n";
      ++depth;
    }
    else if (reader.isEndElement()) {
      depth = std::max(0, depth - 1);
      out += QString(depth * 2, ' ') + "</" + reader.name().toString() + ">\n";
    }
    else if (reader.isCharacters() && !reader.isWhitespace()) {
      out += QString(depth * 2, ' ') + reader.text().toString() + "\n";
    }
  }

  if (reader.hasError()) {
    return QString();
  }

  return out.trimmed();
}

QString
CFrmMqttExplorer::buildSearchText(const QString& topic,
                                  const QByteArray& payload,
                                  const QString& formatted) const
{
  return topic + "\n" + QString::fromUtf8(payload) + "\n" + formatted;
}

bool
CFrmMqttExplorer::applyFilterRecursive(QTreeWidgetItem* item,
                                        const QString& filterLower,
                                        const QString& topicFilterLower)
{
  if (nullptr == item) {
    return false;
  }

  const bool matches = itemMatchesFilter(item, filterLower, topicFilterLower);
  bool visible = matches;

  for (int i = 0; i < item->childCount(); ++i) {
    visible = applyFilterRecursive(item->child(i), filterLower, topicFilterLower) || visible;
  }

  item->setHidden(!visible);
  return visible;
}

void
CFrmMqttExplorer::onFilterChanged(const QString& filter)
{
  const QString lower = filter.trimmed().toLower();
  const QString topicLower = m_editTopicFilter ? m_editTopicFilter->text().trimmed().toLower() : QString();
  for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
    applyFilterRecursive(m_tree->topLevelItem(i), lower, topicLower);
  }
}

void
CFrmMqttExplorer::onTopicFilterChanged(const QString& filter)
{
  const QString lower = filter.trimmed().toLower();
  const QString generalLower = m_editFilter ? m_editFilter->text().trimmed().toLower() : QString();
  for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
    applyFilterRecursive(m_tree->topLevelItem(i), generalLower, lower);
  }
}

void
CFrmMqttExplorer::refreshSelectedDetails()
{
  QTimer::singleShot(0, this, [this]() {
    const auto selected = m_tree->selectedItems();
    if (selected.isEmpty()) {
      return;
    }

    if (selected.first()->treeWidget() != m_tree) {
      return;
    }

    onTreeSelectionChanged();
  });
}

QString
CFrmMqttExplorer::buildSelectedTopicSummary(QTreeWidgetItem* item) const
{
  if (nullptr == item || item->data(0, RoleKind).toInt() != KindTopicNode) {
    return QString();
  }

  int topicCount = 0;
  for (int i = 0; i < item->childCount(); ++i) {
    auto* child = item->child(i);
    if (nullptr != child && child->data(0, RoleKind).toInt() == KindTopicNode) {
      ++topicCount;
    }
  }

  const int messageCount = item->data(0, RoleMessageCount).toInt();
  return tr("Summary: %1 topics, %2 messages under this topic").arg(topicCount).arg(messageCount);
}

QString
CFrmMqttExplorer::buildDetailsText(QTreeWidgetItem* item) const
{
  if (nullptr == item) {
    return QString();
  }

  QString text;
  QTextStream stream(&text);
  stream << tr("Topic") << ": " << item->data(0, RoleTopic).toString() << "\n";
  if (!item->data(0, RoleTimestamp).toString().isEmpty()) {
    stream << tr("Timestamp") << ": " << item->data(0, RoleTimestamp).toString() << "\n";
  }
  if (!item->data(0, RolePayloadFormat).toString().isEmpty()) {
    stream << tr("Format") << ": " << item->data(0, RolePayloadFormat).toString() << "\n";
  }
  if (item->data(0, RoleQos).isValid()) {
    stream << tr("QoS") << ": " << item->data(0, RoleQos).toInt() << "\n";
  }
  stream << tr("Retained") << ": " << (item->data(0, RoleRetained).toBool() ? tr("yes") : tr("no")) << "\n";
  stream << tr("Bytes") << ": " << item->data(0, RolePayloadSize).toInt() << "\n";
  if (item->data(0, RoleMid).toInt() > 0) {
    stream << tr("Message id") << ": " << item->data(0, RoleMid).toInt() << "\n";
  }
  stream << "\n" << tr("Decoded payload") << ":\n" << item->data(0, RolePayloadFormatted).toString()
         << "\n\n" << tr("Raw payload") << ":\n" << item->data(0, RolePayloadRaw).toString() << "\n";

  return text.trimmed();
}

void
CFrmMqttExplorer::addJsonNode(const QString& key,
                              const QJsonValue& value,
                              QTreeWidgetItem* parent)
{
  QString shownKey = key.isEmpty() ? "<item>" : key;
  if (value.isObject()) {
    auto* node = new QTreeWidgetItem(parent, { shownKey, tr("object") });
    const auto obj = value.toObject();
    for (auto it = obj.begin(); it != obj.end(); ++it) {
      addJsonNode(it.key(), it.value(), node);
    }
  }
  else if (value.isArray()) {
    auto* node = new QTreeWidgetItem(parent, { shownKey, tr("array") });
    const auto arr = value.toArray();
    for (int i = 0; i < arr.size(); ++i) {
      addJsonNode(QString("[%1]").arg(i), arr.at(i), node);
    }
  }
  else if (value.isBool()) {
    new QTreeWidgetItem(parent, { shownKey, value.toBool() ? "true" : "false" });
  }
  else if (value.isDouble()) {
    new QTreeWidgetItem(parent, { shownKey, QString::number(value.toDouble()) });
  }
  else if (value.isNull()) {
    new QTreeWidgetItem(parent, { shownKey, "null" });
  }
  else {
    new QTreeWidgetItem(parent, { shownKey, value.toString() });
  }
}

void
CFrmMqttExplorer::addXmlNode(const QString& xml,
                             QTreeWidgetItem* parent)
{
  QXmlStreamReader reader(xml);
  QList<QTreeWidgetItem*> stack;
  stack.push_back(parent);

  while (!reader.atEnd()) {
    reader.readNext();
    if (reader.isStartElement()) {
      auto* node = new QTreeWidgetItem(stack.back(), { reader.name().toString(), "" });
      const auto attrs = reader.attributes();
      for (const auto& attr : attrs) {
        new QTreeWidgetItem(node, { "@" + attr.name().toString(), attr.value().toString() });
      }
      stack.push_back(node);
    }
    else if (reader.isEndElement()) {
      if (stack.size() > 1) {
        stack.pop_back();
      }
    }
    else if (reader.isCharacters() && !reader.isWhitespace()) {
      new QTreeWidgetItem(stack.back(), { "#text", reader.text().toString() });
    }
  }
}

void
CFrmMqttExplorer::renderMessageTree(const QString& topic,
                                    const QString& format,
                                    const QString& formattedPayload,
                                    const QString& rawPayload,
                                    bool retained,
                                    int payloadSize,
                                    int qos,
                                    int mid,
                                    const QString& timestamp)
{
  m_detailsTree->clear();
  auto* root = new QTreeWidgetItem(m_detailsTree, { tr("Message"), "" });
  new QTreeWidgetItem(root, { tr("Topic"), topic });
  new QTreeWidgetItem(root, { tr("Timestamp"), timestamp });
  new QTreeWidgetItem(root, { tr("Receive timestamp"), timestamp });
  new QTreeWidgetItem(root, { tr("Format"), format });
  new QTreeWidgetItem(root, { tr("QoS"), QString::number(qos) });
  new QTreeWidgetItem(root, { tr("Retained"), retained ? "yes" : "no" });
  new QTreeWidgetItem(root, { tr("Bytes"), QString::number(payloadSize) });
  if (mid > 0) {
    new QTreeWidgetItem(root, { tr("Message id"), QString::number(mid) });
  }

  auto* payloadNode = new QTreeWidgetItem(root, { tr("Payload"), format.isEmpty() ? tr("Text") : format });
  if ("JSON" == format) {
    QJsonParseError jsonErr;
    const auto doc = QJsonDocument::fromJson(rawPayload.toUtf8(), &jsonErr);
    if (QJsonParseError::NoError == jsonErr.error) {
      if (doc.isObject()) {
        const auto obj = doc.object();
        for (auto it = obj.begin(); it != obj.end(); ++it) {
          addJsonNode(it.key(), it.value(), payloadNode);
        }
      }
      else if (doc.isArray()) {
        const auto arr = doc.array();
        for (int i = 0; i < arr.size(); ++i) {
          addJsonNode(QString("[%1]").arg(i), arr.at(i), payloadNode);
        }
      }
      else {
        new QTreeWidgetItem(payloadNode, { tr("Decoded"), formatRichTextBlock(colorizeJsonContent(formattedPayload)) });
      }
    }
    else {
      new QTreeWidgetItem(payloadNode, { tr("Decoded"), formatRichTextBlock(colorizeJsonContent(formattedPayload)) });
    }
  }
  else if ("XML" == format) {
    const QString xmlDisplay = formattedPayload.isEmpty() ? buildXmlDisplay(rawPayload) : formattedPayload;
    new QTreeWidgetItem(payloadNode, { tr("Decoded"), formatRichTextBlock(colorizeXmlContent(xmlDisplay)) });
  }
  else {
    new QTreeWidgetItem(payloadNode, { tr("Decoded"), formatRichTextBlock(formattedPayload) });
  }

  auto* rawNode = new QTreeWidgetItem(root, { tr("Raw payload"), tr("Text") });
  auto* rawTextNode = new QTreeWidgetItem(rawNode, { tr("Value"), formatRichTextBlock(rawPayload) });
  rawTextNode->setForeground(1, QColor("#334155"));

  m_detailsTree->expandToDepth(2);
}

void
CFrmMqttExplorer::onTreeSelectionChanged()
{
  const auto selected = m_tree->selectedItems();
  if (selected.isEmpty()) {
    m_detailsTree->clear();
    return;
  }

  auto* item            = selected.first();
  auto* displayItem     = item;
  const int kind        = item->data(0, RoleKind).toInt();
  const QString topic   = item->data(0, RoleTopic).toString();

  if (kind != KindTopicNode && nullptr != item->parent() &&
      item->parent()->data(0, RoleKind).toInt() == KindTopicNode) {
    displayItem = item->parent();
  }

  if (displayItem->data(0, RoleKind).toInt() == KindTopicNode) {
    item = displayItem;
  }

  if (item->data(0, RoleKind).toInt() == KindTopicNode) {
    QString raw = item->data(0, RolePayloadRaw).toString();
    QString decoded = item->data(0, RolePayloadFormatted).toString();
    QString format = item->data(0, RolePayloadFormat).toString();
    bool retained = item->data(0, RoleRetained).toBool();
    int payloadSize = item->data(0, RolePayloadSize).toInt();
    int qos = item->data(0, RoleQos).toInt();
    int mid = item->data(0, RoleMid).toInt();
    QString stamp = item->data(0, RoleTimestamp).toString();

    if (item->childCount() > 0) {
      auto* latestChild = item->child(item->childCount() - 1);
      if (nullptr != latestChild) {
        raw = latestChild->data(0, RolePayloadRaw).toString();
        decoded = latestChild->data(0, RolePayloadFormatted).toString();
        format = latestChild->data(0, RolePayloadFormat).toString();
        retained = latestChild->data(0, RoleRetained).toBool();
        payloadSize = latestChild->data(0, RolePayloadSize).toInt();
        qos = latestChild->data(0, RoleQos).toInt();
        mid = latestChild->data(0, RoleMid).toInt();
        stamp = latestChild->data(0, RoleTimestamp).toString();
      }
    }

    if (!raw.isEmpty()) {
      renderMessageTree(topic, format, decoded, raw, retained, payloadSize, qos, mid, stamp);
      const QString summary = buildSelectedTopicSummary(item);
      if (!summary.isEmpty()) {
        auto* root = m_detailsTree->topLevelItem(0);
        if (nullptr != root) {
          auto* summaryNode = new QTreeWidgetItem(root, { tr("Summary"), summary });
          summaryNode->setForeground(1, QColor("#64748b"));
        }
      }
      return;
    }

    m_detailsTree->clear();
    auto* root = new QTreeWidgetItem(m_detailsTree, { tr("Topic node"), "" });
    new QTreeWidgetItem(root, { tr("Topic"), topic });
    const QString summary = buildSelectedTopicSummary(item);
    if (!summary.isEmpty()) {
      new QTreeWidgetItem(root, { tr("Summary"), summary });
    }
    if (!raw.isEmpty()) {
      new QTreeWidgetItem(root, { tr("Last payload"), raw.left(120) });
      new QTreeWidgetItem(root, { tr("Bytes"), QString::number(item->data(0, RolePayloadSize).toInt()) });
    }
    m_detailsTree->expandToDepth(1);
    return;
  }

  const QString raw     = item->data(0, RolePayloadRaw).toString();
  const QString decoded = item->data(0, RolePayloadFormatted).toString();
  const QString format  = item->data(0, RolePayloadFormat).toString();
  const bool retained   = item->data(0, RoleRetained).toBool();
  const int payloadSize = item->data(0, RolePayloadSize).toInt();
  const int qos         = item->data(0, RoleQos).toInt();
  const int mid         = item->data(0, RoleMid).toInt();
  const QString stamp   = item->data(0, RoleTimestamp).toString();

  renderMessageTree(topic, format, decoded, raw, retained, payloadSize, qos, mid, stamp);
  if (!decoded.isEmpty()) {
    m_detailsTree->clear();
    auto* root = new QTreeWidgetItem(m_detailsTree, { tr("Message"), decoded });
    new QTreeWidgetItem(root, { tr("Topic"), topic });
    new QTreeWidgetItem(root, { tr("Format"), format });
    new QTreeWidgetItem(root, { tr("Retained"), retained ? tr("yes") : tr("no") });
    new QTreeWidgetItem(root, { tr("Bytes"), QString::number(payloadSize) });
    m_detailsTree->expandToDepth(1);
  }
}

QJsonArray collectMessageJson(QTreeWidgetItem* rootItem,
                              const QString& filterText,
                              const QString& topicFilterText)
{
  QJsonArray items;
  if (nullptr == rootItem) {
    return items;
  }

  const auto collect = [&](QTreeWidgetItem* item, auto&& self) -> void {
    if (nullptr == item) {
      return;
    }

    bool isVisibleInTree = !item->isHidden();
    for (QTreeWidgetItem* ancestor = item->parent(); nullptr != ancestor; ancestor = ancestor->parent()) {
      if (ancestor->isHidden()) {
        isVisibleInTree = false;
        break;
      }
    }

    const int kind = item->data(0, kItemRoleKind).toInt();
    const bool shouldExport = isVisibleInTree &&
                              ((kind == kItemKindMessage) ||
                               (kind == kItemKindTopicNode && item->childCount() == 0 &&
                                !item->data(0, Qt::UserRole + 3).toString().isEmpty()));

    if (shouldExport) {
      QJsonObject entry;
      const QString topic = item->data(0, Qt::UserRole + 2).toString();
      const QString payload = item->data(0, Qt::UserRole + 3).toString();
      const QString formatted = item->data(0, Qt::UserRole + 4).toString();
      const QString format = item->data(0, Qt::UserRole + 6).toString();
      const QString timestamp = item->data(0, Qt::UserRole + 5).toString();
      entry["topic"] = topic.isEmpty() ? item->text(0) : topic;
      entry["payload"] = payload.isEmpty() ? item->text(1) : payload;
      entry["formatted"] = formatted;
      entry["format"] = format;
      entry["retained"] = item->data(0, kItemRoleRetained).toBool();
      entry["qos"] = item->data(0, kItemRoleQos).toInt();
      entry["mid"] = item->data(0, Qt::UserRole + 11).toInt();
      entry["timestamp"] = timestamp;
      items.append(entry);
    }

    for (int i = 0; i < item->childCount(); ++i) {
      self(item->child(i), self);
    }
  };

  collect(rootItem, collect);
  return items;
}

QString buildVisibleMessageText(QTreeWidget* tree,
                                 const QString& filterText,
                                 const QString& topicFilterText)
{
  Q_UNUSED(filterText);
  Q_UNUSED(topicFilterText);

  if (nullptr == tree) {
    return QString();
  }

  QStringList blocks;
  const auto collectVisible = [&](QTreeWidgetItem* item, auto&& self) -> void {
    if (nullptr == item) {
      return;
    }

    bool isVisibleInTree = !item->isHidden();
    for (QTreeWidgetItem* ancestor = item->parent(); nullptr != ancestor; ancestor = ancestor->parent()) {
      if (ancestor->isHidden()) {
        isVisibleInTree = false;
        break;
      }
    }

    if (!isVisibleInTree) {
      for (int i = 0; i < item->childCount(); ++i) {
        self(item->child(i), self);
      }
      return;
    }

    const QString topic = item->text(0).trimmed();
    const QString payload = item->text(1).trimmed();
    if (!topic.isEmpty() && (!payload.isEmpty() || item->childCount() == 0)) {
      blocks << QString("Topic: %1\nPayload: %2")
                    .arg(topic, payload.isEmpty() ? QStringLiteral("<empty>") : payload);
    }

    for (int i = 0; i < item->childCount(); ++i) {
      self(item->child(i), self);
    }
  };

  for (int i = 0; i < tree->topLevelItemCount(); ++i) {
    collectVisible(tree->topLevelItem(i), collectVisible);
  }

  return blocks.join("\n\n---\n\n");
}

void
CFrmMqttExplorer::onSaveSelected()
{
  const QString filterText = m_editFilter ? m_editFilter->text() : QString();
  const QString topicFilterText = m_editTopicFilter ? m_editTopicFilter->text() : QString();
  const QString text = buildVisibleMessageText(m_tree, filterText, topicFilterText);
  if (text.isEmpty()) {
    return;
  }

  const QString path = QFileDialog::getSaveFileName(this,
                                                    tr("Save selected MQTT data"),
                                                    QDir::homePath() + "/mqtt-data.txt",
                                                    tr("Text files (*.txt);;All files (*)"));
  if (path.isEmpty()) {
    return;
  }

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this,
                         tr("MQTT raw explorer"),
                         tr("Failed to save selected data to '%1'.").arg(path));
    return;
  }

  file.write(text.toUtf8());
  file.close();
  setStatus(tr("Saved selected MQTT data to %1").arg(path));
}

void
CFrmMqttExplorer::setStatus(const QString& status, bool error)
{
  m_lblStatus->setText(status);
  m_lblStatus->setStyleSheet(error ? "QLabel { color: #c62828; }"
                                   : "QLabel { color: palette(window-text); }");
}

void
CFrmMqttExplorer::applyTlsSettings()
{
  if (nullptr == m_mosq) {
    return;
  }

  const QByteArray cafile = m_cafile.toUtf8();
  const QByteArray capath = m_capath.toUtf8();
  const QByteArray certfile = m_certfile.toUtf8();
  const QByteArray keyfile = m_keyfile.toUtf8();
  const int tlsRc = mosquitto_tls_set(m_mosq,
                                      cafile.isEmpty() ? nullptr : cafile.constData(),
                                      capath.isEmpty() ? nullptr : capath.constData(),
                                      certfile.isEmpty() ? nullptr : certfile.constData(),
                                      keyfile.isEmpty() ? nullptr : keyfile.constData(),
                                      nullptr);
  if (MOSQ_ERR_SUCCESS != tlsRc) {
    setStatus(tr("TLS setup failed: %1")
                .arg(QString::fromUtf8(mosquitto_strerror(tlsRc))),
              true);
  }

  mosquitto_tls_opts_set(m_mosq, m_verifyPeer ? 1 : 0, nullptr, nullptr);

  mosquitto_tls_insecure_set(m_mosq, m_verifyPeer ? false : true);
}

void
CFrmMqttExplorer::subscribeConfiguredTopics()
{
  for (const auto& topic : m_initialSubscriptions) {
    if (!topic.trimmed().isEmpty()) {
      subscribeTopic(topic, 0);
    }
  }
}

void
CFrmMqttExplorer::handleConnected(int rc)
{
  if (0 == rc) {
    m_connecting = false;
    m_connected = true;
    updateConnectionUiState();
    setStatus(tr("Connected to %1:%2").arg(m_host).arg(m_port));
    subscribeConfiguredTopics();
    return;
  }

  m_connecting = false;
  m_connected = false;
  updateConnectionUiState();
  setStatus(tr("Connect failed: %1").arg(QString::fromUtf8(mosquitto_connack_string(rc))),
            true);
}

void
CFrmMqttExplorer::handleDisconnected(int rc)
{
  m_connecting = false;
  m_connected = false;
  updateConnectionUiState();
  if (0 == rc) {
    setStatus(tr("Disconnected"));
  }
  else {
    setStatus(tr("Connection lost: %1").arg(rc), true);
  }
}

void
CFrmMqttExplorer::flushPendingMessages()
{
  if (m_receivePaused) {
    m_pendingMessages.clear();
    return;
  }

  if (m_pendingMessages.isEmpty()) {
    m_messageFlushTimer->stop();
    return;
  }

  constexpr int kBatchSize = 20;
  const bool shouldFilter = !m_editFilter->text().trimmed().isEmpty();

  m_tree->setUpdatesEnabled(false);
  m_detailsTree->setUpdatesEnabled(false);

  for (int i = 0; i < kBatchSize && !m_pendingMessages.isEmpty(); ++i) {
    const PendingMessage pending = m_pendingMessages.takeFirst();
    QString format;
    const QString formatted = formatPayloadForDisplay(pending.payload, &format);

    QTreeWidgetItem* topicNode = ensureTopicPath(pending.topic);
    updateTopicNodeWithMessage(topicNode,
                                pending.topic,
                                pending.payload,
                                formatted,
                                format,
                                pending.retained,
                                pending.qos,
                                pending.mid);
    appendMessageNode(topicNode,
                      pending.topic,
                      pending.payload,
                      formatted,
                      format,
                      pending.retained,
                      pending.qos,
                      pending.mid);
  }

  if (shouldFilter) {
    onFilterChanged(m_editFilter->text());
  }

  m_tree->setUpdatesEnabled(true);
  m_detailsTree->setUpdatesEnabled(true);
  if (m_messageRenderCount != m_lastRenderedMessageCount) {
    m_tree->viewport()->update();
    m_detailsTree->viewport()->update();
    m_lastRenderedMessageCount = m_messageRenderCount;
  }

  if (m_pendingMessages.isEmpty()) {
    m_messageFlushTimer->stop();
  }

  if (!m_tree->selectedItems().isEmpty()) {
    refreshSelectedDetails();
  }
}

void
CFrmMqttExplorer::handleIncomingMessage(const QString& topic,
                                        const QByteArray& payload,
                                        bool retained,
                                        int qos,
                                        int mid)
{
  if (m_receivePaused) {
    return;
  }

  m_pendingMessages.append(PendingMessage{ topic, payload, retained, qos, mid });
  if (m_pendingMessages.size() > 300) {
    m_pendingMessages.erase(m_pendingMessages.begin(), m_pendingMessages.begin() + 150);
  }

  if (!m_messageFlushTimer->isActive()) {
    m_messageFlushTimer->start();
  }
}

void
CFrmMqttExplorer::onMosquittoConnectStatic(struct mosquitto*,
                                           void* userdata,
                                           int rc)
{
  auto* self = static_cast<CFrmMqttExplorer*>(userdata);
  if (nullptr == self) {
    return;
  }
  QMetaObject::invokeMethod(self, [self, rc]() { self->handleConnected(rc); }, Qt::QueuedConnection);
}

void
CFrmMqttExplorer::onMosquittoDisconnectStatic(struct mosquitto*,
                                              void* userdata,
                                              int rc)
{
  auto* self = static_cast<CFrmMqttExplorer*>(userdata);
  if (nullptr == self) {
    return;
  }
  QMetaObject::invokeMethod(self,
                            [self, rc]() { self->handleDisconnected(rc); },
                            Qt::QueuedConnection);
}

void
CFrmMqttExplorer::onMosquittoMessageStatic(struct mosquitto*,
                                           void* userdata,
                                           const struct mosquitto_message* message)
{
  auto* self = static_cast<CFrmMqttExplorer*>(userdata);
  if (nullptr == self || nullptr == message) {
    return;
  }

  const QString topic = QString::fromUtf8(message->topic ? message->topic : "");
  QByteArray payload;
  if (message->payloadlen > 0 && nullptr != message->payload) {
    payload = QByteArray(static_cast<const char*>(message->payload), message->payloadlen);
  }
  const bool retained = message->retain;
  const int qos       = message->qos;
  const int mid       = message->mid;

  QMetaObject::invokeMethod(
    self,
    [self, topic, payload, retained, qos, mid]() { self->handleIncomingMessage(topic, payload, retained, qos, mid); },
    Qt::QueuedConnection);
}
