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

#include <nlohmann/json.hpp>

#include "connection.h"

// for convenience
using json = nlohmann::json;

///////////////////////////////////////////////////////////////////////////////
// connection
//

connection::connection()
{
    m_type = none;
}

///////////////////////////////////////////////////////////////////////////////
// ~connection
//

connection::~connection()
{
    
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// no_connection
//

no_connection::no_connection()
{
    m_type = none;
}

///////////////////////////////////////////////////////////////////////////////
// ~no_connection
//

no_connection::~no_connection()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// toJSON
//

std::string no_connection::toJSON(void) 
{
    std::string rv;

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// fromJSON
//

bool no_connection::fromJSON(const std::string& config)
{
    return true;
}

// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// tcpip_connection
//

tcpip_connection::tcpip_connection()
{
    m_type = tcpip;
}

///////////////////////////////////////////////////////////////////////////////
// ~no_connection
//

tcpip_connection::~tcpip_connection()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// toJSON
//

std::string tcpip_connection::toJSON(void) 
{
    json j;

    j["name"] = m_name;
    j["type"] = m_type;

    return j.dump();
}


///////////////////////////////////////////////////////////////////////////////
// fromJSON
//

bool tcpip_connection::fromJSON(const std::string& config)
{
    return true;
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// canal_connection
//

canal_connection::canal_connection()
{
    m_type = canal;
}

///////////////////////////////////////////////////////////////////////////////
// ~canal_connection
//

canal_connection::~canal_connection()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// toJSON
//

std::string canal_connection::toJSON(void) 
{
    json j;

    j["name"] = m_name;
    j["type"] = m_type;
    
    return j.dump();
}


///////////////////////////////////////////////////////////////////////////////
// fromJSON
//

bool canal_connection::fromJSON(const std::string& config)
{
    return true;
}

// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// socketcan_connection
//

socketcan_connection::socketcan_connection()
{
    m_type = socketcan;
}

///////////////////////////////////////////////////////////////////////////////
// ~socketcan_connection
//

socketcan_connection::~socketcan_connection()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// toJSON
//

std::string socketcan_connection::toJSON(void) 
{
    json j;

    j["name"] = m_name;
    j["type"] = m_type;
    
    return j.dump();
}


///////////////////////////////////////////////////////////////////////////////
// fromJSON
//

bool socketcan_connection::fromJSON(const std::string& config)
{
    return true;
}

// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// ws1_connection
//

ws1_connection::ws1_connection()
{
    m_type = ws1;
}

///////////////////////////////////////////////////////////////////////////////
// ~ws1_connection
//

ws1_connection::~ws1_connection()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// toJSON
//

std::string ws1_connection::toJSON(void) 
{
    json j;

    j["name"] = m_name;
    j["type"] = m_type;
    
    return j.dump();
}


///////////////////////////////////////////////////////////////////////////////
// fromJSON
//

bool ws1_connection::fromJSON(const std::string& config)
{
    return true;
}

// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// ws2_connection
//

ws2_connection::ws2_connection()
{
    m_type = ws2;
}

///////////////////////////////////////////////////////////////////////////////
// ~ws2_connection
//

ws2_connection::~ws2_connection()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// toJSON
//

std::string ws2_connection::toJSON(void) 
{
    json j;

    j["name"] = m_name;
    j["type"] = m_type;
    
    return j.dump();
}


///////////////////////////////////////////////////////////////////////////////
// fromJSON
//

bool ws2_connection::fromJSON(const std::string& config)
{
    return true;
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// mqtt_connection
//

mqtt_connection::mqtt_connection()
{
    m_type = mqtt;
}

///////////////////////////////////////////////////////////////////////////////
// ~mqtt_connection
//

mqtt_connection::~mqtt_connection()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// toJSON
//

std::string mqtt_connection::toJSON(void) 
{
    json j;

    j["name"] = m_name;
    j["type"] = m_type;
    
    return j.dump();
}


///////////////////////////////////////////////////////////////////////////////
// fromJSON
//

bool mqtt_connection::fromJSON(const std::string& config)
{
    return true;
}

// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// udp_connection
//

udp_connection::udp_connection()
{
    m_type = udp;
}

///////////////////////////////////////////////////////////////////////////////
// ~udp_connection
//

udp_connection::~udp_connection()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// toJSON
//

std::string udp_connection::toJSON(void) 
{
    json j;

    j["name"] = m_name;
    j["type"] = m_type;
    
    return j.dump();
}


///////////////////////////////////////////////////////////////////////////////
// fromJSON
//

bool udp_connection::fromJSON(const std::string& config)
{
    return true;
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// multicast_connection
//

multicast_connection::multicast_connection()
{
    m_type = multicast;
}

///////////////////////////////////////////////////////////////////////////////
// ~multicast_connection
//

multicast_connection::~multicast_connection()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// toJSON
//

std::string multicast_connection::toJSON(void) 
{
    json j;

    j["name"] = m_name;
    j["type"] = m_type;
    
    return j.dump();
}


///////////////////////////////////////////////////////////////////////////////
// fromJSON
//

bool multicast_connection::fromJSON(const std::string& config)
{
    return true;
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// rest_connection
//

rest_connection::rest_connection()
{
    m_type = multicast;
}

///////////////////////////////////////////////////////////////////////////////
// ~rest_connection
//

rest_connection::~rest_connection()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// toJSON
//

std::string rest_connection::toJSON(void) 
{
    json j;

    j["name"] = m_name;
    j["type"] = m_type;
    
    return j.dump();
}


///////////////////////////////////////////////////////////////////////////////
// fromJSON
//

bool rest_connection::fromJSON(const std::string& config)
{
    return true;
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// rawcan_connection
//

rawcan_connection::rawcan_connection()
{
    m_type = multicast;
}

///////////////////////////////////////////////////////////////////////////////
// ~rawcan_connection
//

rawcan_connection::~rawcan_connection()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// toJSON
//

std::string rawcan_connection::toJSON(void) 
{
    json j;

    j["name"] = m_name;
    j["type"] = m_type;
    
    return j.dump();
}


///////////////////////////////////////////////////////////////////////////////
// fromJSON
//

bool rawcan_connection::fromJSON(const std::string& config)
{
    return true;
}


// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// rawmqtt_connection
//

rawmqtt_connection::rawmqtt_connection()
{
    m_type = multicast;
}

///////////////////////////////////////////////////////////////////////////////
// ~rawcan_connection
//

rawmqtt_connection::~rawmqtt_connection()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// toJSON
//

std::string rawmqtt_connection::toJSON(void) 
{
    json j;

    j["name"] = m_name;
    j["type"] = m_type;
    
    return j.dump();
}


///////////////////////////////////////////////////////////////////////////////
// fromJSON
//

bool rawmqtt_connection::fromJSON(const std::string& config)
{
    return true;
}
