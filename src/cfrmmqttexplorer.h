#ifndef CFRMMQTTEXPLORER_H
#define CFRMMQTTEXPLORER_H

#include "vscp-client-base.h"

#include <QByteArray>
#include <QDialog>
#include <QHash>
#include <QSet>

struct mosquitto;
struct mosquitto_message;

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
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
  void onFilterChanged(const QString& filter);
  void onTreeSelectionChanged();
  void onCopySelected();
  void onSaveSelected();
  void handleConnected(int rc);
  void handleDisconnected(int rc);
  void handleIncomingMessage(const QString& topic,
                             const QByteArray& payload,
                             bool retained);

private:
  enum ItemRole {
    RoleKind = Qt::UserRole + 1,
    RoleTopic,
    RolePayloadRaw,
    RolePayloadFormatted,
    RoleSearchText
  };

  enum ItemKind { KindTopicNode = 1, KindMessage = 2 };

  void setupUi();
  void configureFromConnection();
  bool connectToBroker();
  void disconnectFromBroker();
  bool isConnected() const;
  bool subscribeTopic(const QString& topic, int qos);
  bool unsubscribeTopic(const QString& topic);
  bool publishMessage(const QString& topic,
                      const QByteArray& payload,
                      int qos,
                      bool retain);

  QTreeWidgetItem* ensureTopicPath(const QString& topic);
  void updateTopicNodeWithMessage(QTreeWidgetItem* topicNode,
                                  const QString& topic,
                                  const QByteArray& payload,
                                  const QString& formattedPayload);
  void appendMessageNode(QTreeWidgetItem* topicNode,
                         const QString& topic,
                         const QByteArray& payload,
                         const QString& formattedPayload,
                         bool retained);
  QString formatPayloadForDisplay(const QByteArray& payload,
                                  QString* outFormat = nullptr) const;
  QString buildXmlDisplay(const QString& xml) const;
  QString buildSearchText(const QString& topic,
                          const QByteArray& payload,
                          const QString& formatted) const;
  bool applyFilterRecursive(QTreeWidgetItem* item, const QString& filterLower);
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
  bool m_tlsEnabled;
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
  QHash<QString, QTreeWidgetItem*> m_topicNodeByPath;

  QPushButton* m_btnConnect;
  QPushButton* m_btnSubscribe;
  QPushButton* m_btnUnsubscribe;
  QPushButton* m_btnPublish;
  QPushButton* m_btnCopy;
  QPushButton* m_btnSave;
  QLineEdit* m_editSubscribeTopic;
  QComboBox* m_comboSubscribeQos;
  QLineEdit* m_editPublishTopic;
  QComboBox* m_comboPublishQos;
  QCheckBox* m_chkPublishRetain;
  QPlainTextEdit* m_editPublishPayload;
  QLineEdit* m_editFilter;
  QTreeWidget* m_tree;
  QPlainTextEdit* m_details;
  QLabel* m_lblStatus;
};

#endif // CFRMMQTTEXPLORER_H
