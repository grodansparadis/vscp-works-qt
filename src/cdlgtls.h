// cdlgtls.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2022 Ake Hedman, Grodans Paradis AB
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

#ifndef CDLGTLS_H
#define CDLGTLS_H


#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class CDlgTLS;
}


class CDlgTLS : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgTLS(QWidget *parent = nullptr);
    ~CDlgTLS();

public:
    /*!
        Setters/getters for TLS enable
    */
    bool isTLSEnabled(void);
    void enableTLS(bool bTLS);

    /*!
        Setters/getters for TLS enable
    */
    bool isVerifyPeerEnabled(void);
    void enableVerifyPeer(bool bverifypeer);

    /*!
        Setters/getters for TLS CA file
    */
    QString getCaFile(void);
    void setCaFile(const QString& str);

    /*!
        Setters/getters for TLS CA path
    */
    QString getCaPath(void);
    void setCaPath(const QString& str);

    /*!
        Setters/getters for TLS Cert file
    */
    QString getCertFile(void);
    void setCertFile(const QString& str);

    /*!
        Setters/getters for TLS key file
    */
    QString getKeyFile(void);
    void setKeyFile(const QString& str);

    /*!
        Setters/getters for TLS pw key file
    */
    QString getPwKeyFile(void);
    void setPwKeyFile(const QString& str);

private slots:
    void onSetCaFile(void);
    void onSetCaPath(void);
    void onSetCertFile(void);
    void onSetKeyFile(void);
    void onSetPwKeyFile(void);

private:

    Ui::CDlgTLS *ui;

};


#endif // CDLGTLS_H
