// filedownloader.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2023 Ake Hedman, Grodans Paradis AB
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

#ifdef WIN32
#include <pch.h>
#endif

#include "filedownloader.h"

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

///////////////////////////////////////////////////////////////////////////////
// CTor
//

FileDownloader::FileDownloader(QUrl fileUrl, QObject *parent) :
 QObject(parent)
{
    connect( &m_WebCtrl, SIGNAL (finished(QNetworkReply*)),
                this, SLOT (fileDownloaded(QNetworkReply*))
    );

    QNetworkRequest request(fileUrl);
    m_WebCtrl.get(request);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

FileDownloader::~FileDownloader() 
{ 
    ;
}

///////////////////////////////////////////////////////////////////////////////
// fileDownloaded
//

void FileDownloader::fileDownloaded(QNetworkReply* pReply) 
{    
    m_DownloadedData = pReply->readAll();

    //emit a signal
    pReply->deleteLater();
    emit downloaded();
}

///////////////////////////////////////////////////////////////////////////////
// downloadedData
//

QByteArray FileDownloader::downloadedData() const 
{
    return m_DownloadedData;
}

