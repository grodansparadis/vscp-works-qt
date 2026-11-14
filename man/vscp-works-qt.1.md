% VSCP-WORKS-QT(1) VSCP Works+
% Åke Hedman, Grodans Paradis AB
% November 2026

# NAME

vscp-works-qt - VSCP Works+, the toolbox for VSCP systems

# SYNOPSIS

**vscp-works-qt** [**\-\-help**] [**\-\-version**]

# DESCRIPTION

**vscp-works-qt** (VSCP Works+) is the graphical toolbox for VSCP systems. It
connects to VSCP nodes and servers over the VSCP transports (tcp/ip, CANAL
drivers, SocketCAN, MQTT, UDP, multicast, websockets and Level II drivers),
and with it you can

* watch, filter and send events in a session window,
* scan for nodes and show their GUID, MDF and standard registers,
* read and write registers, remote variables and the decision matrix of a
  node, using the node's MDF (Module Description File),
* update the firmware of nodes with the VSCP bootloader,
* look up classes and types in the VSCP event database.

Connections are made and saved in the program. Their settings, including TLS
and encryption keys, are stored in the configuration file below.

# OPTIONS

**-h**, **\-\-help**
:   Show the options.

**-v**, **\-\-version**
:   Show the version.

# FILES

*~/.config/VSCP/vscpworks+.json*
:   Settings and the saved connections.

*~/.local/share/VSCP/vscpworks+/*
:   Program data: the VSCP event database (*vscp_events.sqlite3*), the
    program's own database, saved receive and transmit sets (*rxsets*,
    *txsets*) and a cache folder.

*~/.local/share/VSCP/logs/vscpworks+/vscpworks.log*
:   Log file.

# SEE ALSO

**vscpsim**(1), **vscp-scan**(1), **vscp-dump**(1), **vscpd**(8)

The VSCP specification: <https://grodansparadis.github.io/vscp-doc-spec>

The source code and the manual: <https://github.com/grodansparadis/vscp-works-qt>

# COPYRIGHT

Copyright 2000-2026 Åke Hedman, Grodans Paradis AB. MIT license.
