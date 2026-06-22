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
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMetaObject>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QUuid>
#include <QUrl>
#include <QVBoxLayout>
#include <QXmlStreamReader>

#include <algorithm>

namespace {

QString trimmedTopic(const QString& topic)
{
  return topic.trimmed();
}

} // namespace

CFrmMqttExplorer::CFrmMqttExplorer(QWidget* parent, json* pconn)
  : QDialog(parent)
  , m_mosq(nullptr)
  , m_connected(false)
  , m_tlsEnabled(false)
  , m_verifyPeer(true)
  , m_port(1883)
  , m_keepAlive(60)
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
  setWindowTitle(tr("MQTT Explorer"));
  resize(1200, 760);

  auto* mainLayout = new QVBoxLayout(this);

  auto* connBox    = new QGroupBox(tr("Connection"), this);
  auto* connLayout = new QHBoxLayout(connBox);
  m_lblStatus      = new QLabel(tr("Disconnected"), connBox);
  m_btnConnect     = new QPushButton(tr("Connect"), connBox);
  connLayout->addWidget(m_lblStatus, 1);
  connLayout->addWidget(m_btnConnect);
  mainLayout->addWidget(connBox);

  auto* subscribeBox    = new QGroupBox(tr("Subscribe"), this);
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

  auto* publishBox    = new QGroupBox(tr("Publish"), this);
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
  mainLayout->addWidget(publishBox);

  auto* filterLayout = new QHBoxLayout();
  filterLayout->addWidget(new QLabel(tr("Filter:"), this));
  m_editFilter = new QLineEdit(this);
  m_editFilter->setPlaceholderText(tr("Filter by topic or payload"));
  filterLayout->addWidget(m_editFilter, 1);
  m_btnCopy = new QPushButton(tr("Copy selected"), this);
  m_btnSave = new QPushButton(tr("Save selected"), this);
  filterLayout->addWidget(m_btnCopy);
  filterLayout->addWidget(m_btnSave);
  mainLayout->addLayout(filterLayout);

  auto* splitter = new QSplitter(Qt::Horizontal, this);
  m_tree          = new QTreeWidget(splitter);
  m_tree->setHeaderLabels({ tr("Topic/Message"), tr("Value") });
  m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
  m_tree->setAlternatingRowColors(true);
  m_tree->setUniformRowHeights(true);
  m_tree->setExpandsOnDoubleClick(true);

  m_details = new QPlainTextEdit(splitter);
  m_details->setReadOnly(true);
  splitter->setStretchFactor(0, 3);
  splitter->setStretchFactor(1, 2);
  mainLayout->addWidget(splitter, 1);

  connect(m_btnConnect,
          &QPushButton::clicked,
          this,
          &CFrmMqttExplorer::onConnectClicked);
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
  connect(m_tree,
          &QTreeWidget::itemSelectionChanged,
          this,
          &CFrmMqttExplorer::onTreeSelectionChanged);
  connect(m_btnCopy, &QPushButton::clicked, this, &CFrmMqttExplorer::onCopySelected);
  connect(m_btnSave, &QPushButton::clicked, this, &CFrmMqttExplorer::onSaveSelected);
}

void
CFrmMqttExplorer::configureFromConnection()
{
  if (m_conn.contains("name") && m_conn["name"].is_string()) {
    setWindowTitle(tr("MQTT Explorer - %1")
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
}

bool
CFrmMqttExplorer::connectToBroker()
{
  disconnectFromBroker();

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
  m_btnConnect->setText(tr("Disconnect"));
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

  m_connected = false;
  m_btnConnect->setText(tr("Connect"));
}

bool
CFrmMqttExplorer::isConnected() const
{
  return m_connected && (nullptr != m_mosq);
}

void
CFrmMqttExplorer::onConnectClicked()
{
  if (isConnected()) {
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
}

void
CFrmMqttExplorer::onSubscribeClicked()
{
  if (!isConnected()) {
    setStatus(tr("Not connected"), true);
    return;
  }

  subscribeTopic(m_editSubscribeTopic->text(), m_comboSubscribeQos->currentText().toInt());
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
      item->setData(0, RoleKind, KindTopicNode);
      item->setData(0, RoleTopic, normalized);
      item->setData(0, RoleSearchText, currentPath.toLower());
      if (nullptr == parent) {
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
                                             const QString& formattedPayload)
{
  if (nullptr == topicNode) {
    return;
  }

  const QString payloadText = QString::fromUtf8(payload);
  topicNode->setText(1, payloadText.left(120).replace('\n', " "));
  topicNode->setData(0, RolePayloadRaw, payloadText);
  topicNode->setData(0, RolePayloadFormatted, formattedPayload);
  topicNode->setData(0,
                     RoleSearchText,
                     buildSearchText(topic, payload, formattedPayload).toLower());
}

void
CFrmMqttExplorer::appendMessageNode(QTreeWidgetItem* topicNode,
                                    const QString& topic,
                                    const QByteArray& payload,
                                    const QString& formattedPayload,
                                    bool retained)
{
  if (nullptr == topicNode) {
    return;
  }

  const QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
  const QString preview =
    QString::fromUtf8(payload).left(120).replace('\n', " ").replace('\r', " ");
  auto* msgItem = new QTreeWidgetItem({ timestamp + (retained ? " [R]" : ""), preview });
  msgItem->setData(0, RoleKind, KindMessage);
  msgItem->setData(0, RoleTopic, topic);
  msgItem->setData(0, RolePayloadRaw, QString::fromUtf8(payload));
  msgItem->setData(0, RolePayloadFormatted, formattedPayload);
  msgItem->setData(0,
                   RoleSearchText,
                   buildSearchText(topic, payload, formattedPayload).toLower());
  topicNode->addChild(msgItem);
  topicNode->setExpanded(true);

  constexpr int kMaxMessagesPerTopic = 500;
  while (topicNode->childCount() > kMaxMessagesPerTopic) {
    delete topicNode->takeChild(0);
  }
}

QString
CFrmMqttExplorer::formatPayloadForDisplay(const QByteArray& payload,
                                          QString* outFormat) const
{
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
CFrmMqttExplorer::applyFilterRecursive(QTreeWidgetItem* item, const QString& filterLower)
{
  if (nullptr == item) {
    return false;
  }

  bool visible = filterLower.isEmpty();
  const QString search = item->data(0, RoleSearchText).toString();
  if (!filterLower.isEmpty() && search.contains(filterLower, Qt::CaseInsensitive)) {
    visible = true;
  }

  for (int i = 0; i < item->childCount(); ++i) {
    visible = applyFilterRecursive(item->child(i), filterLower) || visible;
  }

  item->setHidden(!visible);
  return visible;
}

void
CFrmMqttExplorer::onFilterChanged(const QString& filter)
{
  const QString lower = filter.trimmed().toLower();
  for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
    applyFilterRecursive(m_tree->topLevelItem(i), lower);
  }
}

void
CFrmMqttExplorer::onTreeSelectionChanged()
{
  const auto selected = m_tree->selectedItems();
  if (selected.isEmpty()) {
    m_details->clear();
    return;
  }

  auto* item = selected.first();
  const QString topic = item->data(0, RoleTopic).toString();
  const QString raw = item->data(0, RolePayloadRaw).toString();
  const QString formatted = item->data(0, RolePayloadFormatted).toString();
  const int kind = item->data(0, RoleKind).toInt();

  QString text;
  text += tr("Topic: %1\n").arg(topic);
  if (KindMessage == kind) {
    text += tr("Type: Message\n");
  }
  else {
    text += tr("Type: Topic node\n");
  }
  text += "\n";
  if (!formatted.isEmpty()) {
    text += tr("Decoded payload:\n");
    text += formatted;
    text += "\n\n";
  }
  if (!raw.isEmpty()) {
    text += tr("Raw payload:\n");
    text += raw;
  }

  m_details->setPlainText(text.trimmed());
}

void
CFrmMqttExplorer::onCopySelected()
{
  const QString text = m_details->toPlainText();
  if (text.isEmpty()) {
    return;
  }
  QApplication::clipboard()->setText(text);
}

void
CFrmMqttExplorer::onSaveSelected()
{
  const QString text = m_details->toPlainText();
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
                         tr("MQTT Explorer"),
                         tr("Failed to save selected data to '%1'.").arg(path));
    return;
  }

  file.write(text.toUtf8());
  file.close();
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
    m_connected = true;
    setStatus(tr("Connected to %1:%2").arg(m_host).arg(m_port));
    subscribeConfiguredTopics();
    return;
  }

  m_connected = false;
  m_btnConnect->setText(tr("Connect"));
  setStatus(tr("Connect failed: %1").arg(QString::fromUtf8(mosquitto_connack_string(rc))),
            true);
}

void
CFrmMqttExplorer::handleDisconnected(int rc)
{
  m_connected = false;
  m_btnConnect->setText(tr("Connect"));
  if (0 == rc) {
    setStatus(tr("Disconnected"));
  }
  else {
    setStatus(tr("Connection lost: %1").arg(rc), true);
  }
}

void
CFrmMqttExplorer::handleIncomingMessage(const QString& topic,
                                        const QByteArray& payload,
                                        bool retained)
{
  QString format;
  const QString formatted = formatPayloadForDisplay(payload, &format);

  QTreeWidgetItem* topicNode = ensureTopicPath(topic);
  updateTopicNodeWithMessage(topicNode, topic, payload, formatted);
  appendMessageNode(topicNode, topic, payload, formatted, retained);
  onFilterChanged(m_editFilter->text());

  if (m_tree->selectedItems().isEmpty()) {
    m_tree->setCurrentItem(topicNode);
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

  QMetaObject::invokeMethod(
    self,
    [self, topic, payload, retained]() { self->handleIncomingMessage(topic, payload, retained); },
    Qt::QueuedConnection);
}
