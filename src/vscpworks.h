// vscpworks.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB
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

#ifndef VSCPWORKS_H
#define VSCPWORKS_H

#include <QApplication>

#define VSCPWORKS_MAJOR_VERSION     0
#define VSCPWORKS_MINOR_VERSION     0
#define VSCPWORKS_RELEASE_VERSION   1
#define VSCPWORKS_BUILD_VERSION     0
#define VSCPWORKS_VERSION_STR       "alfa 0.0.1.0"


enum numerical_base {HEX, DECIMAL, OCTAL, BINARY};

// home folder is used for storage of program configuration
// system folder holds databases etc
#ifdef WIN32
#define DEFAULT_HOME_FOLDER       "c:/programdata/vscp/vscpworks/"
#define DEFAULT_VSCP_SYSTEM_FOLDER  "c:/programdata/vscp/"
#else 
#define DEFAULT_HOME_FOLDER       "~/.vscpworks/"
#define DEFAULT_VSCP_SYSTEM_FOLDER  "/var/lib/vscp/"
#endif

// class connection;

/*!
    Encapsulates VSCP works main settings
*/
class vscpworks : public QApplication {

public:
    /*!
        Constructor
    */
    vscpworks(int &argc, char **argv);

    /*!
        Destructor
    */
    ~vscpworks();

    // ------------------------------------------------------------------------
    // Global Configuration information below
    //   This info is read from a configuration file 
    //   at startup and saved on close. The configuration
    //   file should be placed in the home folder. 
    // ------------------------------------------------------------------------

    // Numerical base for all numericals in system
    numerical_base m_base;

};


#endif