// version.h
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

#ifndef _____VSCP_VERSION_h_____
#define _____VSCP_VERSION_h_____
/*
    MAJOR version with incompatible API changes,
    MINOR version with add functionality in a backwards-compatible manner, and
    RELEASE version with backwards-compatible bug fixes.
    BUILD Just a new build.
*/
// I M P O T A N T ! ! ! Lines below must be located at line
// 39/40/41/42/43/44/45/46 I M P O T A N T ! ! !
#define VSCPWORKS_MAJOR_VERSION     0
#define VSCPWORKS_MINOR_VERSION     0
#define VSCPWORKS_RELEASE_VERSION   1
#define VSCPWORKS_BUILD_VERSION     0   

#define VSCPWORKS_DISPLAY_VERSION   "0.0.1-0 Alpha"

#define VSCPWORKS_COPYRIGHT                                                    \
    "Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB, "                   \
    "https://www.grodansparadis.com"
#define VSCPWORKS_COPYRIGHT_HTML                                               \
    "Copyright © 2000-2021 Åke Hedman, <a "                                    \
    "href=\"mailto:info@grodansparadis.com\">Grodans Paradis AB</a><br><a "    \
    "href=\"https://www.grodansparadis.com\">https://"                         \
    "www.grodansparadis.com</a>"

#define VSCPWORKS_VERSION(major, minor, release)                               \
    (((major) << 16) | ((minor) << 8) | (release))

#endif

