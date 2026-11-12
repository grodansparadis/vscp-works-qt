// secretstore.h
//
// Keeps the secrets of connections (passwords, encryption keys, ...) out of
// the configuration file. They are stored in the operating system keyring
// (GNOME Keyring / KWallet through the Secret Service, Windows Credential
// Manager, macOS Keychain). If no keyring is available they are stored in
// a separate file that only the user can read.
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

#ifndef SECRETSTORE_H
#define SECRETSTORE_H

#include <QString>

#include <map>
#include <string>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*!
  Secret storage for connection settings.

  A connection object keeps its secrets in memory as usual. When the
  configuration is written, saveSecrets() moves them out of the object
  that goes to disk and records where they are ("secrets": "keyring" or
  "file"). When the configuration is read, loadSecrets() puts them back.
*/
class SecretStore {

public:
  /// Name of the field that tells where the secrets of a connection are
  static const char* const MARKER;

  /*!
    @param configDir Folder of the configuration file. The fallback
    secrets file is created there.
  */
  explicit SecretStore(const QString& configDir);
  ~SecretStore();

  /// True if the connection field holds a secret
  static bool isSecretField(const json& conn, const std::string& field);

  /*!
    Copy of a connection without secrets (and without the storage marker),
    for export, logging and the configuration file.
  */
  static json withoutSecrets(const json& conn);

  /// True if the connection has a secret with a value
  static bool hasSecrets(const json& conn);

  /*!
    Store the secrets of a connection and return the copy of it that
    should be written to the configuration file.
    @param conn The connection, with its secrets
    @return Connection without secrets, with the storage marker if it has
    secrets. If the secrets could not be stored anywhere the connection is
    returned unchanged so nothing is lost.
  */
  json saveSecrets(const json& conn);

  /*!
    Put the stored secrets of a connection back into it.
    Connections from an old configuration file have their secrets in the
    object itself; they are left as they are.
    @param conn Connection read from the configuration file
    @return false if the connection refers to secrets that could not be read
  */
  bool loadSecrets(json& conn);

  /// Remove the stored secrets of a connection
  void removeSecrets(const std::string& uuid);

  /// True if the operating system keyring is used
  bool isKeyringAvailable(void);

  /// Path of the fallback secrets file
  QString fallbackFile(void) const { return m_fallbackFile; }

  /*!
    Make a file readable and writable by its owner only (0600)
    @return true on success
  */
  static bool restrictToOwner(const QString& path);

private:
  // Keyring access (synchronous). Return false if the keyring failed.
  bool keyringWrite(const std::string& key, const std::string& value);
  bool keyringRead(const std::string& key, std::string& value, bool& bFound);
  bool keyringDelete(const std::string& key);

  // Fallback file access
  json readFallback(void);
  bool writeFallback(const json& j);

  QString m_fallbackFile;

  // -1 = not known yet, 0 = no keyring, 1 = keyring works
  int m_keyringState;

  // Secrets as last stored, by connection uuid, to avoid rewriting them on
  // every save
  std::map<std::string, std::string> m_stored;

  // Where the secrets of each connection are ("keyring" or "file")
  std::map<std::string, std::string> m_storedWhere;
};

#endif // SECRETSTORE_H
