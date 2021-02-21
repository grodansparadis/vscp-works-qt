/*******************************************************************************
 * Copyright (c) 2020 Frank Pagliughi <fpagliughi@mindspring.com>
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Frank Pagliughi - initial implementation and documentation
 *******************************************************************************/

#include "mqtt/create_options.h"
#include <cstring>

namespace mqtt {

/////////////////////////////////////////////////////////////////////////////

const MQTTAsync_createOptions create_options::DFLT_C_STRUCT =
			MQTTAsync_createOptions_initializer;


create_options::create_options(int mqttVersion) : create_options()
{
	opts_.MQTTVersion = mqttVersion;
}

create_options::create_options(int mqttVersion, int maxBufferedMessages) : create_options()
{
	opts_.MQTTVersion = mqttVersion;

	if (maxBufferedMessages != 0) {
		opts_.sendWhileDisconnected = to_int(true);
		opts_.maxBufferedMessages = maxBufferedMessages;
	}
}

/////////////////////////////////////////////////////////////////////////////

} // end namespace mqtt

