// secretstore.cpp
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

#include "secretstore.h"

#include <vscp-client-base.h>

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS) && !defined(Q_OS_ANDROID)
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#define SECRETSTORE_USE_DBUS_CHECK 1
#endif

#include <keychain.h>

#include <spdlog/spdlog.h>

namespace {

// Keyring service name and key prefix. Windows Credential Manager uses the
// key alone as the credential name, so the key is made unique by itself.
const char* const KEYRING_SERVICE = "VSCP Works";
const char* const KEYRING_PREFIX  = "vscpworks/connection/";

// Fields that hold secrets in all connection types
const char* const SECRET_FIELDS[] = { "password", "key", "hmac-key", "binary-key", "binary-hmac-key", "pwkeyfile" };

bool
isEmptyValue(const json& v)
{
  return v.is_null() || (v.is_string() && v.get<std::string>().empty()) ||
         ((v.is_object() || v.is_array()) && v.empty());
}

// Run a keychain job and wait for it
void
runJob(QKeychain::Job& job)
{
  job.setAutoDelete(false);
  // Never let QtKeychain fall back to storing secrets in plain text
  job.setInsecureFallback(false);
  QEventLoop loop;
  QObject::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
  job.start();
  loop.exec();
}

} // namespace

const char* const SecretStore::MARKER = "secrets";

///////////////////////////////////////////////////////////////////////////////
// SecretStore
//

SecretStore::SecretStore(const QString& configDir)
{
  m_fallbackFile = QDir(configDir).filePath("vscpworks-secrets.json");
  m_keyringState = -1;
}

SecretStore::~SecretStore() {}

///////////////////////////////////////////////////////////////////////////////
// isSecretField
//

bool
SecretStore::isSecretField(const json& conn, const std::string& field)
{
  for (const char* name : SECRET_FIELDS) {
    if (field == name) {
      return true;
    }
  }

  // The embedded configuration of a Level II driver often holds credentials
  // and keys. For CANAL "config" is the driver parameter string.
  if (("config" == field) && conn.contains("type") && conn["type"].is_number_integer() &&
      (static_cast<int>(CVscpClient::connType::LEVEL2) == conn["type"].get<int>())) {
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// withoutSecrets
//

json
SecretStore::withoutSecrets(const json& conn)
{
  if (!conn.is_object()) {
    return conn;
  }
  json out = conn;
  for (auto it = conn.begin(); it != conn.end(); ++it) {
    if (isSecretField(conn, it.key())) {
      out.erase(it.key());
    }
  }
  out.erase(MARKER);
  return out;
}

///////////////////////////////////////////////////////////////////////////////
// hasSecrets
//

bool
SecretStore::hasSecrets(const json& conn)
{
  if (!conn.is_object()) {
    return false;
  }
  for (auto it = conn.begin(); it != conn.end(); ++it) {
    if (isSecretField(conn, it.key()) && !isEmptyValue(it.value())) {
      return true;
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// restrictToOwner
//

bool
SecretStore::restrictToOwner(const QString& path)
{
  if (!QFile::exists(path)) {
    return true;
  }
  return QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
}

///////////////////////////////////////////////////////////////////////////////
// Keyring
//

#ifdef SECRETSTORE_USE_DBUS_CHECK
// True if a D-Bus service is running or can be started on the session bus
static bool
dbusServiceAvailable(const QStringList& names)
{
  QDBusConnection bus = QDBusConnection::sessionBus();
  if (!bus.isConnected() || (nullptr == bus.interface())) {
    return false;
  }
  QStringList activatable = bus.interface()->activatableServiceNames().value();
  for (const QString& name : names) {
    if (bus.interface()->isServiceRegistered(name).value() || activatable.contains(name)) {
      return true;
    }
  }
  return false;
}
#endif

bool
SecretStore::isKeyringAvailable(void)
{
#ifdef SECRETSTORE_USE_DBUS_CHECK
  // Without a keyring service every keyring call waits for a D-Bus timeout,
  // so check first. QtKeychain uses the Secret Service (GNOME Keyring,
  // KeePassXC, ...) when built with libsecret, otherwise KWallet.
  if (-1 == m_keyringState) {
    QStringList services = { "org.kde.kwalletd5", "org.kde.kwalletd6" };
#ifdef VSCPWORKS_HAVE_LIBSECRET
    services << "org.freedesktop.secrets";
#endif
    if (!dbusServiceAvailable(services)) {
      spdlog::info("Secrets: No keyring service; passwords and keys are stored in {}", m_fallbackFile.toStdString());
      m_keyringState = 0;
    }
  }
#endif
  if (-1 == m_keyringState) {
    // Find out by reading an entry that does not exist: "not found" means
    // the keyring answered
    std::string value;
    bool bFound = false;
    keyringRead("probe", value, bFound);
  }
  return (1 == m_keyringState);
}

bool
SecretStore::keyringWrite(const std::string& key, const std::string& value)
{
  QKeychain::WritePasswordJob job(KEYRING_SERVICE);
  job.setKey(QString::fromStdString(KEYRING_PREFIX + key));
  job.setBinaryData(QByteArray::fromStdString(value));
  runJob(job);
  if (QKeychain::NoError != job.error()) {
    spdlog::warn("Secrets: keyring write failed, using {} instead: {}",
                 m_fallbackFile.toStdString(),
                 job.errorString().toStdString());
    // A keyring that has worked may still refuse one entry (Windows limits
    // a credential to 2560 bytes); then only that connection uses the file.
    // Otherwise don't wait for a failing keyring again.
    if (1 != m_keyringState) {
      m_keyringState = 0;
    }
    return false;
  }
  m_keyringState = 1;
  return true;
}

bool
SecretStore::keyringRead(const std::string& key, std::string& value, bool& bFound)
{
  bFound = false;
  QKeychain::ReadPasswordJob job(KEYRING_SERVICE);
  job.setKey(QString::fromStdString(KEYRING_PREFIX + key));
  runJob(job);
  if (QKeychain::EntryNotFound == job.error()) {
    m_keyringState = 1;
    return true;
  }
  if (QKeychain::NoError != job.error()) {
    spdlog::warn("Secrets: keyring read failed: {}", job.errorString().toStdString());
    if (1 != m_keyringState) {
      m_keyringState = 0; // Don't wait for a failing keyring again
    }
    return false;
  }
  m_keyringState = 1;
  value          = job.binaryData().toStdString();
  bFound         = true;
  return true;
}

bool
SecretStore::keyringDelete(const std::string& key)
{
  QKeychain::DeletePasswordJob job(KEYRING_SERVICE);
  job.setKey(QString::fromStdString(KEYRING_PREFIX + key));
  runJob(job);
  return (QKeychain::NoError == job.error()) || (QKeychain::EntryNotFound == job.error());
}

///////////////////////////////////////////////////////////////////////////////
// Fallback file
//

json
SecretStore::readFallback(void)
{
  QFile file(m_fallbackFile);
  if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
    return json::object();
  }
  json j = json::parse(file.readAll().toStdString(), nullptr, false);
  return (j.is_discarded() || !j.is_object()) ? json::object() : j;
}

bool
SecretStore::writeFallback(const json& j)
{
  QDir().mkpath(QFileInfo(m_fallbackFile).absolutePath());
  QSaveFile file(m_fallbackFile);
  if (!file.open(QIODevice::WriteOnly)) {
    spdlog::error("Secrets: Unable to write {}", m_fallbackFile.toStdString());
    return false;
  }
  // Owner only before anything is written
  file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
  const std::string data = j.dump(2);
  if ((file.write(data.data(), (qint64)data.size()) != (qint64)data.size()) || !file.commit()) {
    spdlog::error("Secrets: Unable to write {}", m_fallbackFile.toStdString());
    return false;
  }
  return restrictToOwner(m_fallbackFile);
}

///////////////////////////////////////////////////////////////////////////////
// saveSecrets
//

json
SecretStore::saveSecrets(const json& conn)
{
  if (!conn.is_object() || !conn.contains("uuid") || !conn["uuid"].is_string() ||
      conn["uuid"].get<std::string>().empty()) {
    return conn;
  }
  const std::string uuid = conn["uuid"].get<std::string>();

  json secrets = json::object();
  for (auto it = conn.begin(); it != conn.end(); ++it) {
    if (isSecretField(conn, it.key()) && !isEmptyValue(it.value())) {
      secrets[it.key()] = it.value();
    }
  }

  json out = withoutSecrets(conn);

  if (secrets.empty()) {
    if (m_stored.count(uuid)) {
      removeSecrets(uuid);
    }
    return out;
  }

  const std::string value = secrets.dump();

  // Unchanged since last time
  auto it = m_stored.find(uuid);
  if ((it != m_stored.end()) && (it->second == value)) {
    out[MARKER] = m_storedWhere[uuid];
    return out;
  }

  if (isKeyringAvailable() && keyringWrite(uuid, value)) {
    // Not also in the fallback file
    json f = readFallback();
    if (f.contains(uuid)) {
      f.erase(uuid);
      writeFallback(f);
    }
    m_stored[uuid]      = value;
    m_storedWhere[uuid] = "keyring";
    out[MARKER]         = "keyring";
    return out;
  }

  json f  = readFallback();
  f[uuid] = secrets;
  if (writeFallback(f)) {
    m_stored[uuid]      = value;
    m_storedWhere[uuid] = "file";
    out[MARKER]         = "file";
    return out;
  }

  spdlog::error("Secrets: Unable to store the secrets of connection {}; kept in the configuration file", uuid);
  return conn;
}

///////////////////////////////////////////////////////////////////////////////
// loadSecrets
//

bool
SecretStore::loadSecrets(json& conn)
{
  if (!conn.is_object() || !conn.contains(MARKER)) {
    return true; // Nothing stored elsewhere
  }
  const std::string where = conn[MARKER].is_string() ? conn[MARKER].get<std::string>() : "";
  conn.erase(MARKER);
  if (!conn.contains("uuid") || !conn["uuid"].is_string()) {
    return false;
  }
  const std::string uuid = conn["uuid"].get<std::string>();

  json secrets;
  bool bFound = false;

  if ("keyring" == where) {
    std::string value;
    if (keyringRead(uuid, value, bFound) && bFound) {
      secrets = json::parse(value, nullptr, false);
    }
  }

  // The fallback file, also if the keyring is gone
  if (!bFound) {
    json f = readFallback();
    if (f.contains(uuid) && f[uuid].is_object()) {
      secrets = f[uuid];
      bFound  = true;
    }
  }

  if (!bFound || !secrets.is_object()) {
    spdlog::warn("Secrets: The secrets of connection {} were not found ({})", uuid, where);
    return false;
  }

  for (auto it = secrets.begin(); it != secrets.end(); ++it) {
    conn[it.key()] = it.value();
  }

  // Secrets in the fallback file move to the keyring on the next save once
  // a keyring is available (for example after installing one)
  if (("keyring" == where) || !isKeyringAvailable()) {
    m_stored[uuid]      = secrets.dump();
    m_storedWhere[uuid] = ("keyring" == where) ? "keyring" : "file";
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeSecrets
//

void
SecretStore::removeSecrets(const std::string& uuid)
{
  if (isKeyringAvailable()) {
    keyringDelete(uuid);
  }
  json f = readFallback();
  if (f.contains(uuid)) {
    f.erase(uuid);
    writeFallback(f);
  }
  m_stored.erase(uuid);
  m_storedWhere.erase(uuid);
}
