// Include the implementations to exercise the internal diff and plot helpers.
#include "../src/cfrmmqttexplorer.cpp"
#include "../src/cfrmrawcansession.cpp"

#include <QTemporaryDir>

class ExplorerRegressionTest {
public:
  static void check(bool condition, const char* message)
  {
    if (!condition) {
      qFatal("%s", message);
    }
  }

  static void runMqtt()
  {
    json themedConnection = {
      { "uuid", "test-connection" },
      { "ui", { { "theme", "light" } } }
    };
    check(vscpworks_connectionDarkTheme(json::object(), true),
          "The configured dark theme must be the fallback for unthemed connections");
    check(!vscpworks_connectionDarkTheme(themedConnection, true),
          "A saved light theme must override the configured dark default");
    vscpworks_setConnectionTheme(themedConnection, true);
    check(vscpworks_connectionDarkTheme(themedConnection, false) &&
            vscpworks_connectionUuid(themedConnection) == "test-connection",
          "A connection theme must be serializable with its connection UUID");

    const QString manyLines = QString("x\n").repeated(20000);
    check(diffToHtml(manyLines, manyLines).contains("too large"), "Large diff must be bounded");
    const QString atBudget = QString("x\n").repeated(998);
    check(!diffToHtml(atBudget, atBudget).contains("too large"),
          "A comparison at the matrix budget must be allowed");
    check(diffToHtml(atBudget + "\nx", atBudget).contains("too large"),
          "A comparison just above the matrix budget must be rejected");
    const QString diff = diffToHtml("same\nold", "same\nnew");
    check(diff.contains("- old") && diff.contains("+ new") && diff.contains("  same"),
          "Small diffs must still show changed and unchanged lines");
    const QString darkDiff = diffToHtml("old", "new", true);
    check(darkDiff.contains("#e5e7eb") && darkDiff.contains("#14532d") &&
            darkDiff.contains("#7f1d1d"),
          "Dark diffs must use readable foreground and change colors");

    json connection = { { "host", "127.0.0.1" }, { "port", 1 } };
    CFrmMqttExplorer explorer(nullptr, &connection);
    explorer.disconnectFromBroker();
    const QColor lightBase = QApplication::palette().color(QPalette::Base);
    explorer.m_darkTheme = true;
    explorer.applyTheme();
    check(explorer.m_tree->palette().color(QPalette::Base) == QColor("#141414"),
          "Dark theme must set the MQTT tree palette");
    check(explorer.m_plot->palette().color(QPalette::Base) == QColor("#141414"),
          "Dark theme must set the MQTT plot palette");
    check(explorer.styleSheet().contains("background-color: #141414"),
          "Dark theme must provide explicit dark widget styles");
    explorer.m_darkTheme = false;
    explorer.applyTheme();
    check(explorer.m_tree->palette().color(QPalette::Base) == lightBase,
          "Light theme must restore the application palette");
    const auto receive = [&](const QString& topic, const QByteArray& payload) {
      explorer.handleIncomingMessage(topic, payload, false, 1, 0);
      explorer.flushPendingMessages();
    };
    receive("plant/temp", "12");
    auto* branch = explorer.ensureTopicPath("plant");
    check(branch->data(0, CFrmMqttExplorer::RoleTopic).toString() == "plant",
          "Intermediate topic actions must target their own prefix");
    check(explorer.ensureTopicPath("/plant")->data(0, CFrmMqttExplorer::RoleTopic).toString() == "/plant",
          "Leading empty levels must be preserved");
    check(explorer.ensureTopicPath("plant//temp") != explorer.ensureTopicPath("plant/temp"),
          "Empty MQTT topic levels must not alias");
    explorer.populateHistory(branch);
    check(explorer.m_historyTree->topLevelItemCount() == 0 && explorer.m_plot->m_values.isEmpty(),
          "Subtopic values must not appear in a branch's history or plot");

    receive("plant", "20");
    receive("plant", "21");
    explorer.m_tree->setCurrentItem(branch);
    check(explorer.m_plot->m_values == QVector<double>({ 20, 21 }),
          "Plot must contain only the selected topic's numeric messages");
    auto* older = explorer.m_historyTree->topLevelItem(1);
    explorer.m_historyTree->setCurrentItem(older);
    const QVariant selectedId = older->data(0, CFrmMqttExplorer::RoleHistoryId);
    receive("elsewhere", "5");
    receive("plant", "22");
    QCoreApplication::processEvents();
    check(explorer.m_historyTree->currentItem() != nullptr &&
            explorer.m_historyTree->currentItem()->data(0, CFrmMqttExplorer::RoleHistoryId) == selectedId,
          "History refresh must preserve the selected message");

    const QByteArray fullPayload(300, 'x');
    receive("plant/temp", fullPayload);
    receive("plant/temp", QByteArray());
    explorer.onFilterChanged("topic=plant/temp");
    const QString text = buildVisibleMessageText(explorer.m_tree, "topic=plant/temp", "");
    check(text.contains("Topic: plant/temp\nPayload: " + QString::fromUtf8(fullPayload)),
          "Text export must include hidden history with full topic and payload");
    check(!text.contains("Topic: elsewhere"), "Text export must still respect filters");
    const auto records = collectMessageJson(explorer.ensureTopicPath("plant/temp"), "", "");
    check(records.size() == 3 && records.last().toObject()["payload"].toString().isEmpty(),
          "JSON export must retain all matching records, including empty payloads");
    check(records.first().toObject()["format"].toString() == "Text" &&
            !records.first().toObject()["timestamp"].toString().isEmpty(),
          "Exported format and timestamp must use the correct roles");
    check(collectMessageJson(branch, "qos=2", "").isEmpty(), "JSON export must apply message filters");

    explorer.m_comboReceiveMode->setCurrentIndex(1);
    receive("plant", "23");
    check(explorer.ensureTopicPath("plant/temp")->treeWidget() == explorer.m_tree,
          "Replacing a branch value must not delete its subtopics");
  }

  static void runCan()
  {
    CFrmRawCanSession session(nullptr, nullptr);
    QCanBusFrame frame(0x123, QByteArray::fromHex("1122"));
    session.appendFrame(frame, CFrmRawCanSession::FrameDirection::Rx);
    session.refreshViews();
    session.m_treeFrames->setCurrentItem(session.m_treeFrames->topLevelItem(0));
    check(session.m_paused, "Selecting a frame must pause capture");
    const bool dark = session.m_darkTheme;
    session.toggleTheme();
    check(session.m_paused && session.m_actPause->isChecked() &&
            session.m_treeFrames->selectedItems().size() == 1 && session.m_darkTheme != dark,
          "Theme switching must preserve pause and selection");

    session.m_comboViewMode->setCurrentIndex(1);
    frame.setExtendedFrameFormat(true);
    session.appendFrame(frame, CFrmRawCanSession::FrameDirection::Tx);
    session.refreshViews();
    QTextDocument counts;
    counts.setHtml(session.m_countersLabel->text());
    check(counts.toPlainText().contains("EXT 1") && counts.toPlainText().contains("TX 1"),
          "Counters must update while ID statistics is active");
    session.clearLog();
    counts.setHtml(session.m_countersLabel->text());
    check(counts.toPlainText().contains("STD 0") && counts.toPlainText().contains("EXT 0") &&
            counts.toPlainText().contains("TX 0") && counts.toPlainText().contains("ERR 0"),
          "Clearing the log must reset counters");
  }
};

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  QTemporaryDir settings;
  ExplorerRegressionTest::check(settings.isValid(), "Temporary settings directory must exist");
  QCoreApplication::setOrganizationName("VSCP-test");
  QCoreApplication::setApplicationName("explorer-regressions");
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
  mosquitto_lib_init();
  ExplorerRegressionTest::runMqtt();
  ExplorerRegressionTest::runCan();
  mosquitto_lib_cleanup();
}
