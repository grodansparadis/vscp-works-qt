// keyvalidation.h
//
// Validation of AES and HMAC-SHA-256 keys entered in the connection
// dialogs. The rules match the ones the VSCP UDP, multicast and tcp/ip
// clients apply when they read their JSON configuration, so a
// configuration that passes here is also accepted by the client.
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
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef KEYVALIDATION_H
#define KEYVALIDATION_H

#include <vscp.h>

#include <QCoreApplication>
#include <QString>

namespace keyvalidation {

/*!
  AES key length in bytes for a VSCP encryption code
  @param encryption VSCP_ENCRYPTION_xxx
  @return 16, 24 or 32 for AES128/192/256, 0 otherwise
*/
inline int
aesKeyBytes(int encryption)
{
  switch (encryption) {
    case VSCP_ENCRYPTION_AES128:
      return 16;
    case VSCP_ENCRYPTION_AES192:
      return 24;
    case VSCP_ENCRYPTION_AES256:
      return 32;
    default:
      return 0;
  }
}

/*!
  Check that a string is exactly nbytes bytes written as hex
*/
inline bool
isHexKey(const QString& str, int nbytes)
{
  if (str.length() != 2 * nbytes) {
    return false;
  }
  for (const QChar ch : str) {
    if (!ch.isDigit() && !((ch >= 'a') && (ch <= 'f')) && !((ch >= 'A') && (ch <= 'F'))) {
      return false;
    }
  }
  return true;
}

/*!
  The AES key and the HMAC key must be independent. The clients compare the
  AES key with the first bytes of the HMAC key.
*/
inline bool
keysIndependent(const QString& aesKey, const QString& hmacKey)
{
  return 0 != QString::compare(aesKey, hmacKey.left(aesKey.length()), Qt::CaseInsensitive);
}

inline QString
tr(const char* str)
{
  return QCoreApplication::translate("keyvalidation", str);
}

/*!
  Validate the security settings of a UDP or multicast connection.
  @param encryption VSCP_ENCRYPTION_xxx
  @param key AES key as hex. Empty means the built in default key.
  @param hmacKey HMAC-SHA-256 key as hex. Empty disables authentication.
  @param requireEncryption Drop received frames that are not encrypted
  @return Empty string if valid, otherwise a description of the problem
*/
inline QString
validateDatagram(int encryption, const QString& key, const QString& hmacKey, bool requireEncryption)
{
  const int nbytes = aesKeyBytes(encryption);

  if (nbytes && !key.isEmpty() && !isHexKey(key, nbytes)) {
    return tr("The encryption key must be exactly %1 hex characters (%2 bytes) for the selected algorithm.")
      .arg(2 * nbytes)
      .arg(nbytes);
  }

  if (!hmacKey.isEmpty()) {
    if (!nbytes) {
      return tr("HMAC authentication needs an AES encryption algorithm.");
    }
    if (key.isEmpty()) {
      return tr("HMAC authentication needs an explicitly configured encryption key.");
    }
    if (!isHexKey(hmacKey, 32)) {
      return tr("The HMAC key must be exactly 64 hex characters (32 bytes).");
    }
    if (!keysIndependent(key, hmacKey)) {
      return tr("The HMAC key and the encryption key must be different.");
    }
  }

  if (requireEncryption && (!nbytes || key.isEmpty())) {
    return tr("'Require encryption' needs an AES encryption algorithm and an explicitly configured key.");
  }

  return QString();
}

/*!
  Validate the authenticated binary mode settings of a tcp/ip connection.
  Encryption over tcp/ip is always authenticated, so both keys are needed.
  @param binaryMode Binary mode enabled
  @param encryption VSCP_ENCRYPTION_xxx
  @param key AES key as hex
  @param hmacKey HMAC-SHA-256 key as hex
  @return Empty string if valid, otherwise a description of the problem
*/
inline QString
validateTcpBinary(bool binaryMode, int encryption, const QString& key, const QString& hmacKey)
{
  if (VSCP_ENCRYPTION_NONE == encryption) {
    return QString();
  }

  const int nbytes = aesKeyBytes(encryption);
  if (!nbytes) {
    return tr("Unknown encryption algorithm.");
  }
  if (!binaryMode) {
    return tr("Encryption is only available in binary mode.");
  }
  if (!isHexKey(key, nbytes)) {
    return tr("The encryption key must be exactly %1 hex characters (%2 bytes) for the selected algorithm.")
      .arg(2 * nbytes)
      .arg(nbytes);
  }
  if (!isHexKey(hmacKey, 32)) {
    return tr("The HMAC key must be exactly 64 hex characters (32 bytes).");
  }
  if (!keysIndependent(key, hmacKey)) {
    return tr("The HMAC key and the encryption key must be different.");
  }

  return QString();
}

} // namespace keyvalidation

#endif // KEYVALIDATION_H
