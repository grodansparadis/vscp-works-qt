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

#include "connection.h"

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

// ----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// mqtt_connection
//

mqtt_connection::mqtt_connection()
{
    m_type = mqtt;
}

///////////////////////////////////////////////////////////////////////////////
// ~ws1_connection
//

mqtt_connection::~mqtt_connection()
{
    
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
// ~ws1_connection
//

udp_connection::~udp_connection()
{
    
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
// ~ws1_connection
//

multicast_connection::~multicast_connection()
{
    
}