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

    /*!
        Return a JSON representation of connection
        @return JSON representation as string
    */
    virtual std::string toJSON(void) = 0;

    /*!
        Set member variables from JSON representation of connection
        @param config JSON representation as string
        @return True on success, false on failure.
    */
    virtual bool fromJSON(const std::string& config) = 0;

protected:

    // Name for connection
    std::string m_name;

    // Connection type
    connection_type m_type;

    // Input filter

};

// ----------------------------------------------------------------------------

class no_connection : public connection {

public:

    no_connection();
    ~no_connection();

    /*!
        Return a JSON representation of connection
        @return JSON representation as string
    */
    virtual std::string toJSON(void);

    /*!
        Set member variables from JSON representation of connection
        @param config JSON representation as string
        @return True on success, false on failure.
    */
    virtual bool fromJSON(const std::string& config);

private:

};

class tcpip_connection : public connection {

public:

    tcpip_connection();
    ~tcpip_connection();

    /*!
        Return a JSON representation of connection
        @return JSON representation as string
    */
    virtual std::string toJSON(void);

    /*!
        Set member variables from JSON representation of connection
        @param config JSON representation as string
        @return True on success, false on failure.
    */
    virtual bool fromJSON(const std::string& config);

private:

    /// VSCP tcp/ip host to connect to
    std::string m_host;

    /// port of host 
    short m_port;

    /// Username to login with
    std::string m_user;

    /// Password to login with
    std::string m_password;

    /// Timeout for connection in milliseconds
    uint32_t m_connectTimeout;

    /// Timeout for response in milliseconds
    uint32_t m_responseTimeout;

};

// ----------------------------------------------------------------------------

class canal_connection : public connection {

public:

    canal_connection();
    ~canal_connection();

    /*!
        Return a JSON representation of connection
        @return JSON representation as string
    */
    virtual std::string toJSON(void);

    /*!
        Set member variables from JSON representation of connection
        @param config JSON representation as string
        @return True on success, false on failure.
    */
    virtual bool fromJSON(const std::string& config);

private:

    /// Path to driver
    std::string m_path;

    /// Configuration string
    std::string m_config;

    /// Configuration flags
    uint32_t m_flags;

};

// ----------------------------------------------------------------------------

class socketcan_connection : public connection {

public:

    socketcan_connection();
    ~socketcan_connection();

    /*!
        Return a JSON representation of connection
        @return JSON representation as string
    */
    virtual std::string toJSON(void);

    /*!
        Set member variables from JSON representation of connection
        @param config JSON representation as string
        @return True on success, false on failure.
    */
    virtual bool fromJSON(const std::string& config);

private:

    std::string m_device;

};

// ----------------------------------------------------------------------------

class ws1_connection : public connection {

public:

    ws1_connection();
    ~ws1_connection();

    /*!
        Return a JSON representation of connection
        @return JSON representation as string
    */
    virtual std::string toJSON(void);

    /*!
        Set member variables from JSON representation of connection
        @param config JSON representation as string
        @return True on success, false on failure.
    */
    virtual bool fromJSON(const std::string& config);

private:

};

// ----------------------------------------------------------------------------

class ws2_connection : public connection {

public:

    ws2_connection();
    ~ws2_connection();

    /*!
        Return a JSON representation of connection
        @return JSON representation as string
    */
    virtual std::string toJSON(void);

    /*!
        Set member variables from JSON representation of connection
        @param config JSON representation as string
        @return True on success, false on failure.
    */
    virtual bool fromJSON(const std::string& config);

private:

};

// ----------------------------------------------------------------------------

class mqtt_connection : public connection {

public:

    mqtt_connection();
    ~mqtt_connection();

    /*!
        Return a JSON representation of connection
        @return JSON representation as string
    */
    virtual std::string toJSON(void);

    /*!
        Set member variables from JSON representation of connection
        @param config JSON representation as string
        @return True on success, false on failure.
    */
    virtual bool fromJSON(const std::string& config);

private:

};

// ----------------------------------------------------------------------------

class udp_connection : public connection {

public:

    udp_connection();
    ~udp_connection();

    /*!
        Return a JSON representation of connection
        @return JSON representation as string
    */
    virtual std::string toJSON(void);

    /*!
        Set member variables from JSON representation of connection
        @param config JSON representation as string
        @return True on success, false on failure.
    */
    virtual bool fromJSON(const std::string& config);

private:

};

// ----------------------------------------------------------------------------

class multicast_connection : public connection {

public:

    multicast_connection();
    ~multicast_connection();

    /*!
        Return a JSON representation of connection
        @return JSON representation as string
    */
    virtual std::string toJSON(void);

    /*!
        Set member variables from JSON representation of connection
        @param config JSON representation as string
        @return True on success, false on failure.
    */
    virtual bool fromJSON(const std::string& config);

private:

};


// ----------------------------------------------------------------------------


class rest_connection : public connection {

public:

    rest_connection();
    ~rest_connection();

    /*!
        Return a JSON representation of connection
        @return JSON representation as string
    */
    virtual std::string toJSON(void);

    /*!
        Set member variables from JSON representation of connection
        @param config JSON representation as string
        @return True on success, false on failure.
    */
    virtual bool fromJSON(const std::string& config);
    
private:

};


// ----------------------------------------------------------------------------


class rawcan_connection : public connection {

public:

    rawcan_connection();
    ~rawcan_connection();

    /*!
        Return a JSON representation of connection
        @return JSON representation as string
    */
    virtual std::string toJSON(void);

    /*!
        Set member variables from JSON representation of connection
        @param config JSON representation as string
        @return True on success, false on failure.
    */
    virtual bool fromJSON(const std::string& config);
    
private:

};


// ----------------------------------------------------------------------------


class rawmqtt_connection : public connection {

public:

    rawmqtt_connection();
    ~rawmqtt_connection();

    /*!
        Return a JSON representation of connection
        @return JSON representation as string
    */
    virtual std::string toJSON(void);

    /*!
        Set member variables from JSON representation of connection
        @param config JSON representation as string
        @return True on success, false on failure.
    */
    virtual bool fromJSON(const std::string& config);
    
private:

};

#endif