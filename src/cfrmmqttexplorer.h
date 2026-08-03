#ifndef CFRMMQTTEXPLORER_H
#define CFRMMQTTEXPLORER_H

#include "vscp-client-base.h"

#include <QByteArray>
#include <QDialog>
#include <QHash>
#include <QList>
#include <QSet>
#include <QTimer>

struct mosquitto;
struct mosquitto_message;

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;
class QJsonValue;
class QAction;
class QMenu;
class QMenuBar;
class QTreeWidget;
class QTreeWidgetItem;

class CFrmMqttExplorer : public QDialog {
  Q_OBJECT

public:
  explicit CFrmMqttExplorer(QWidget* parent, json* pconn);
  ~CFrmMqttExplorer();

private slots:
  void onConnectClicked();
  void onPublishClicked();
  void onSubscribeClicked();
  void onUnsubscribeClicked();
  void onAddPublishTopic();
  void onUseSelectedPublishTopic();
  void onClearPublishTopics();
  void onSavePublishTopics();
  void onLoadPublishTopics();
  void onMenuSubscribe();
  void onMenuUnsubscribe();
  void onMenuSubscribeConfigured();
  void onMenuClearSubscriptions();
  void onFilterChanged(const QString& filter);
  void onTopicFilterChanged(const QString& filter);
  void onTreeSelectionChanged();
  void onSaveSelected();
  void onPauseReceiveClicked();
  void onReceiveModeChanged(int index);
  void onSaveSelectedEvent();
  void flushPendingMessages();
  void handleConnected(int rc);
  void handleDisconnected(int rc);
  void handleIncomingMessage(const QString& topic,
                             const QByteArray& payload,
                             bool retained,
                             int qos,
                             int mid);

private:
  enum ItemRole {
    RoleKind = Qt::UserRole + 1,
    RoleTopic,
    RolePayloadRaw,
    RolePayloadFormatted,
    RoleSearchText,
    RoleTimestamp,
    RolePayloadFormat,
    RoleRetained,
    RolePayloadSize,
    RoleQos,
    RoleMid,
    RoleMessageCount
  };

  enum ItemKind { KindTopicNode = 1, KindMessage = 2 };
  enum ReceiveMode { ReceiveAppend = 0, ReceiveReplace = 1 };

  struct PendingMessage {
    QString topic;
    QByteArray payload;
    bool retained;
    int qos;
    int mid;
  };

  void setupUi();
  void configureFromConnection();
  bool connectToBroker();
  void disconnectFromBroker();
  bool isConnected() const;
  void updateConnectionUiState();
  void updateReceiveUiState();
  bool subscribeTopic(const QString& topic, int qos);
  bool unsubscribeTopic(const QString& topic);
  bool publishMessage(const QString& topic,
                      const QByteArray& payload,
                      int qos,
                      bool retain);

  QTreeWidgetItem* ensureTopicPath(const QString& topic);
  void pruneTopicNode(QTreeWidgetItem* item);
  void pruneTopicNodeIndexIfNeeded();
  void updateTopicNodeWithMessage(QTreeWidgetItem* topicNode,
                                  const QString& topic,
                                  const QByteArray& payload,
                                  const QString& formattedPayload,
                                  const QString& format,
                                  bool retained,
                                  int qos,
                                  int mid);
  void appendMessageNode(QTreeWidgetItem* topicNode,
                         const QString& topic,
                         const QByteArray& payload,
                         const QString& formattedPayload,
                         const QString& format,
                         bool retained,
                         int qos,
                         int mid);
  QString formatPayloadForDisplay(const QByteArray& payload,
                                  QString* outFormat = nullptr) const;
  QString buildXmlDisplay(const QString& xml) const;
  QString buildSearchText(const QString& topic,
                          const QByteArray& payload,
                          const QString& formatted) const;
  void flashItemHighlight(QTreeWidgetItem* item);
  bool applyFilterRecursive(QTreeWidgetItem* item,
                            const QString& filterLower,
                            const QString& topicFilterLower);
  QString buildDetailsText(QTreeWidgetItem* item) const;
  QString buildSelectedTopicSummary(QTreeWidgetItem* item) const;
  void refreshSelectedDetails();
  void renderMessageTree(const QString& topic,
                         const QString& format,
                         const QString& formattedPayload,
                         const QString& rawPayload,
                         bool retained,
                         int payloadSize,
                         int qos,
                         int mid,
                         const QString& timestamp);
  void addJsonNode(const QString& key,
                   const QJsonValue& value,
                   QTreeWidgetItem* parent);
  void addXmlNode(const QString& xml,
                  QTreeWidgetItem* parent);
  void addPublishTopicIfMissing(const QString& topic);
  void addSubscriptionIfMissing(const QString& topic);
  void setStatus(const QString& status, bool error = false);
  void applyTlsSettings();
  void subscribeConfiguredTopics();

  static void onMosquittoConnectStatic(struct mosquitto* mosq,
                                       void* userdata,
                                       int rc);
  static void onMosquittoDisconnectStatic(struct mosquitto* mosq,
                                          void* userdata,
                                          int rc);
  static void onMosquittoMessageStatic(struct mosquitto* mosq,
                                       void* userdata,
                                       const struct mosquitto_message* message);

  json m_conn;
  struct mosquitto* m_mosq;
  bool m_connected;
  bool m_connecting;
  bool m_tlsEnabled;
  bool m_receivePaused;
  bool m_verifyPeer;
  QString m_host;
  int m_port;
  QString m_clientId;
  QString m_user;
  QString m_password;
  int m_keepAlive;
  QString m_cafile;
  QString m_capath;
  QString m_certfile;
  QString m_keyfile;
  QString m_pwkeyfile;
  QSet<QString> m_subscriptions;
  QSet<QString> m_initialSubscriptions;
  QSet<QString> m_publishTopics;
  QHash<QString, QTreeWidgetItem*> m_topicNodeByPath;
  QList<PendingMessage> m_pendingMessages;
  QTimer* m_messageFlushTimer;
  int m_messageRenderCount;
  int m_lastRenderedMessageCount;

  QPushButton* m_btnConnect;
  QPushButton* m_btnPauseReceive;
  QPushButton* m_btnSubscribe;
  QPushButton* m_btnUnsubscribe;
  QPushButton* m_btnPublish;
  QPushButton* m_btnSave;
  QPushButton* m_btnAddPublishTopic;
  QPushButton* m_btnUsePublishTopic;
  QPushButton* m_btnClearPublishTopics;
  QPushButton* m_btnSavePublishTopics;
  QPushButton* m_btnLoadPublishTopics;
  QLineEdit* m_editSubscribeTopic;
  QComboBox* m_comboSubscribeQos;
  QComboBox* m_comboReceiveMode;
  QLineEdit* m_editPublishTopic;
  QComboBox* m_comboPublishQos;
  QCheckBox* m_chkPublishRetain;
  QPlainTextEdit* m_editPublishPayload;
  QLineEdit* m_editFilter;
  QLineEdit* m_editTopicFilter;
  QListWidget* m_listPublishTopics;
  QTreeWidget* m_tree;
  QTreeWidget* m_detailsTree;
  QMenuBar* m_menuBar;
  QMenu* m_subscribeMenu;
  QAction* m_actConnect;
  QAction* m_actPauseReceive;
  QAction* m_actSubscribe;
  QAction* m_actUnsubscribe;
  QAction* m_actSubscribeConfigured;
  QAction* m_actClearSubscriptions;
  QLabel* m_lblStatus;
};

#endif // CFRMMQTTEXPLORER_H
