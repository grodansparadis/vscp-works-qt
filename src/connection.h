// connection.h
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

#ifndef VSCPWORKS_CONNECTIONS_H
#define VSCPWORKS_CONNECTIONS_H

#include <string>

#include "connection_types.h"


/*!
    Encapsulates one connection
*/
class connection {

public:    

    /*!
        Constructor
    */
    connection();

    /*!
        Destructor
    */
    ~connection();

    /*!
        Set connection type
        @param type The connection type to set
    */
    void setType(connection_type type) { m_type = type; };

    /*!
        Set connection name
        @param name The name of the connection to set
    */
    void setName(const std::string& name) { m_name = name; };
    
    /*!
        Get connection type
        @return Type for the connection
    */
    connection_type getType(void);

    /*!
        Get connection type
        @return name of the connection
    */
    std::string getName(void) { return m_name; };

protected:

    // Name for connection
    std::string m_name;

    // Connection type
    connection_type m_type;
};

// ----------------------------------------------------------------------------

class no_connection : public connection {

public:

    no_connection();
    ~no_connection();

private:

};

class tcpip_connection : public connection {

public:

    tcpip_connection();
    ~tcpip_connection();

private:

};

// ----------------------------------------------------------------------------

class canal_connection : public connection {

public:

    canal_connection();
    ~canal_connection();

private:

};

// ----------------------------------------------------------------------------

class socketcan_connection : public connection {

public:

    socketcan_connection();
    ~socketcan_connection();

private:

};

// ----------------------------------------------------------------------------

class ws1_connection : public connection {

public:

    ws1_connection();
    ~ws1_connection();

private:

};

// ----------------------------------------------------------------------------

class ws2_connection : public connection {

public:

    ws2_connection();
    ~ws2_connection();

private:

};

// ----------------------------------------------------------------------------

class mqtt_connection : public connection {

public:

    mqtt_connection();
    ~mqtt_connection();

private:

};

// ----------------------------------------------------------------------------

class udp_connection : public connection {

public:

    udp_connection();
    ~udp_connection();

private:

};

// ----------------------------------------------------------------------------

class multicast_connection : public connection {

public:

    multicast_connection();
    ~multicast_connection();

private:

};

#endif