
#include "ssdp.h"

#include "lwip/igmp.h"

#include <string>

#include "ssdp_helpers.h"

#include "fnWiFi.h"
#include "IPAddress.h"

#include "../../include/debug.h"

SSDPDeviceClass SSDPDevice;

static void ssdp_service_task(void* arg)
{
    while ( true ) 
    {
        SSDPDevice.service();
        taskYIELD();
    }
}


void SSDPDeviceClass::start()
{
    // Start task
    xTaskCreatePinnedToCore(ssdp_service_task, "ssdp_service_task", 4096, NULL, 10, NULL, 0);
}

SSDPDeviceClass::SSDPDeviceClass() :
	m_server(0),
	m_port(80),
	m_ttl(SSDP_MULTICAST_TTL)
{
	m_uuid[0] = '\0';
	m_modelNumber[0] = '\0';
	sprintf(m_deviceType, "urn:schemas-upnp-org:device:Basic:1");
	m_friendlyName[0] = '\0';
	m_presentationURL[0] = '\0';
	m_serialNumber[0] = '\0';
	m_modelName[0] = '\0';
	m_modelURL[0] = '\0';
	m_manufacturer[0] = '\0';
	m_manufacturerURL[0] = '\0';
	sprintf(m_schemaURL, "device.xml");

    uint64_t chipId = 0LL;
    esp_efuse_mac_get_default((uint8_t*) (&chipId));

	sprintf(m_uuid, "38323636-4558-4dda-9188-cda0e6%02x%02x%02x",
		(uint16_t)((chipId >> 16) & 0xff),
		(uint16_t)((chipId >> 8) & 0xff),
		(uint16_t)chipId & 0xff);

	for (int i = 0; i < SSDP_QUEUE_SIZE; i++) {
		m_queue[i].time = 0;
	}
}

void SSDPDeviceClass::update() {
	postNotifyUpdate();
}

bool SSDPDeviceClass::readLine(std::string &value) {
	char buffer[65];
	int bufferPos = 0;

	while (1) {
		int c = m_server->read();

		if (c < 0) {
			buffer[bufferPos] = '\0';

			break;
		}
		if (c == '\r' && m_server->peek() == '\n') {
			m_server->read();

			buffer[bufferPos] = '\0';

			break;
		}
		if (bufferPos < 64) {
			buffer[bufferPos++] = c;
		}
	}

	value = std::string(buffer);

	return bufferPos > 0;
}

bool SSDPDeviceClass::readKeyValue(std::string &key, std::string &value) {
	char buffer[65];
	int bufferPos = 0;

	while (1) {
		int c = m_server->read();

		if (c < 0) {
			if (bufferPos == 0) return false;

			buffer[bufferPos] = '\0';

			break;
		}
		if (c == ':') {
			buffer[bufferPos] = '\0';

			while (m_server->peek() == ' ') m_server->read();

			break;
		}
		else if (c == '\r' && m_server->peek() == '\n') {
			m_server->read();

			if (bufferPos == 0) return false;

			buffer[bufferPos] = '\0';

			key = std::string();
			value = std::string(buffer);

			return true;
		}
		if (bufferPos < 64) {
			buffer[bufferPos++] = c;
		}
	}

	key = std::string(buffer);
	
	readLine(value);

	return true;
}

void SSDPDeviceClass::postNotifyALive() {
	unsigned long time = esp_timer_get_time();

	post(NOTIFY_ALIVE_INIT, ROOT_FOR_ALL, SSDP_MULTICAST_ADDR, SSDP_PORT, time + 10);
	post(NOTIFY_ALIVE_INIT, ROOT_BY_UUID, SSDP_MULTICAST_ADDR, SSDP_PORT, time + 55);
	post(NOTIFY_ALIVE_INIT, ROOT_BY_TYPE, SSDP_MULTICAST_ADDR, SSDP_PORT, time + 80);

	post(NOTIFY_ALIVE_INIT, ROOT_FOR_ALL, SSDP_MULTICAST_ADDR, SSDP_PORT, time + 210);
	post(NOTIFY_ALIVE_INIT, ROOT_BY_UUID, SSDP_MULTICAST_ADDR, SSDP_PORT, time + 255);
	post(NOTIFY_ALIVE_INIT, ROOT_BY_TYPE, SSDP_MULTICAST_ADDR, SSDP_PORT, time + 280);

	post(NOTIFY_ALIVE, ROOT_FOR_ALL, SSDP_MULTICAST_ADDR, SSDP_PORT, time + 610);
	post(NOTIFY_ALIVE, ROOT_BY_UUID, SSDP_MULTICAST_ADDR, SSDP_PORT, time + 655);
	post(NOTIFY_ALIVE, ROOT_BY_TYPE, SSDP_MULTICAST_ADDR, SSDP_PORT, time + 680);
}

void SSDPDeviceClass::postNotifyUpdate() {
	unsigned long time = esp_timer_get_time();

	post(NOTIFY_UPDATE, ROOT_FOR_ALL, SSDP_MULTICAST_ADDR, SSDP_PORT, time + 10);
	post(NOTIFY_UPDATE, ROOT_BY_UUID, SSDP_MULTICAST_ADDR, SSDP_PORT, time + 55);
	post(NOTIFY_UPDATE, ROOT_BY_TYPE, SSDP_MULTICAST_ADDR, SSDP_PORT, time + 80);
}

void SSDPDeviceClass::postResponse(long mx) {
	unsigned long time = esp_timer_get_time();
	//unsigned long delay = random(0, mx) * 900L; // 1000 ms - 100 ms
	int delay = 100 + ( std::rand() % ( 1000 - 100 + 1 ) ); // 1000 ms - 100 ms

	IPAddress address = m_server->remoteIP();
	uint16_t port = m_server->remotePort();

	post(RESPONSE, ROOT_FOR_ALL, address, port, time + delay / 3);
	post(RESPONSE, ROOT_BY_UUID, address, port, time + delay / 3 * 2);
	post(RESPONSE, ROOT_BY_TYPE, address, port, time + delay);
}

void SSDPDeviceClass::postResponse(ssdp_udn_t udn, long mx) {
	int delay = 100 + ( std::rand() % ( mx - 100 + 1 ) ); // 1000 ms - 100 ms
	post(RESPONSE, udn, m_server->remoteIP(), m_server->remotePort(), esp_timer_get_time() + delay); // 1000 ms - 100 ms
}

void SSDPDeviceClass::post(ssdp_message_t type, ssdp_udn_t udn, IPAddress address, uint16_t port, unsigned long time) {
	for (int i = 0; i < SSDP_QUEUE_SIZE; i++) {
		if (m_queue[i].time == 0) {
			m_queue[i].type = type;
			m_queue[i].udn = udn;
			m_queue[i].address = address;
			m_queue[i].port = port;
			m_queue[i].time = time;

			break;
		}
	}
}

void SSDPDeviceClass::send(ssdp_send_parameters_t *parameters) {
	uint8_t buffer[1460];
	unsigned int ip = fnWiFi.localIP();
	
	const char *typeTemplate;
	const char *uri = 0, *usn1 = 0, *usn2 = 0, *usn3 = 0;

	switch (parameters->type) {
		case NOTIFY_ALIVE_INIT:
		case NOTIFY_ALIVE:
			typeTemplate = SSDP_NOTIFY_ALIVE_TEMPLATE;
			break;
		case NOTIFY_UPDATE:
			typeTemplate = SSDP_NOTIFY_UPDATE_TEMPLATE;
			break;
		default: // RESPONSE
			typeTemplate = SSDP_RESPONSE_TEMPLATE;
			break;
	}

	std::string uuid = "uuid:" + std::string(m_uuid);

	switch (parameters->udn) {
		case ROOT_FOR_ALL:
			uri = "upnp:rootdevice";
			usn1 = uuid.c_str();
			usn2 = "::";
			usn3 = "upnp:rootdevice";
			break;
		case ROOT_BY_UUID:
			uri = uuid.c_str();
			usn1 = uuid.c_str();
			usn2 = "";
			usn3 = "";
			break;
		case ROOT_BY_TYPE:
			uri = m_deviceType;
			usn1 = uuid.c_str();
			usn2 = "::";
			usn3 = m_deviceType;
			break;
	}

	int len = snprintf((char *)buffer, sizeof(buffer),
		SSDP_PACKET_TEMPLATE, typeTemplate,
		SSDP_INTERVAL, m_modelName, m_modelNumber, usn1, usn2, usn3, parameters->type == RESPONSE ? "ST" : "NT", uri,
		LIP2STR(&ip), m_port, m_schemaURL
	);

	if (parameters->address == (uint32_t) SSDP_MULTICAST_ADDR) {
		m_server->beginMulticast(parameters->address, parameters->port);
		m_server->beginMulticastPacket();
	}	else {
		m_server->beginPacket(parameters->address, parameters->port);
	}
	
	m_server->write(buffer, len);
	m_server->endPacket();

	parameters->time = parameters->type == NOTIFY_ALIVE ? parameters->time + SSDP_INTERVAL * 900L : 0; // 1000 ms - 100 ms
}

// std::string SSDPDeviceClass::schema() {
// 	uint32_t ip = fnWiFi.localIP();
// 	char schema[2048];
// 	sprintf(schema, SSDP_SCHEMA_TEMPLATE,
// 		LIP2STR(&ip), m_port,
// 		m_deviceType,
// 		m_friendlyName,
// 		m_presentationURL,
// 		m_serialNumber,
// 		m_modelName,
// 		m_modelNumber,
// 		m_modelURL,
// 		m_manufacturer,
// 		m_manufacturerURL,
// 		m_uuid
// 	);
// 	return std::string(schema);
// }

void SSDPDeviceClass::service() {
	IPAddress current = fnWiFi.localIP();

	if (m_last != current) {
		m_last = current;

		for (int i = 0; i < SSDP_QUEUE_SIZE; i++) {
			m_queue[i].time = 0;
		}

		if (current != INADDR_NONE) {
			if (!m_server) m_server = new fnUDP();

			m_server->beginMulticast(SSDP_MULTICAST_ADDR, SSDP_PORT);
			m_server->beginMulticastPacket();

			postNotifyALive();
		}
		else if (m_server) {
			m_server->stop();
		}
	}

	if (m_server && m_server->parsePacket()) {
		std::string value;

		if (readLine(value) && equals(value.c_str(), "M-SEARCH * HTTP/1.1", false)) {
			std::string key, st;
			bool host = false, man = false;
			long mx = 0;

			while (readKeyValue(key, value)) {
				if (equals(key.c_str(), "HOST", false) && equals(value.c_str(), "239.255.255.250:1900")) {
					host = true;
				}
				else if (equals(key.c_str(), "MAN", false) && equals(value.c_str(), "\"ssdp:discover\"", false)) {
					man = true;
				}
				else if (equals(key.c_str(), "ST", false)) {
					st = value;
				}
				else if (equals(key.c_str(), "MX", false)) {
					mx = atoi( value.c_str() );
				}
			}

			if (host && man && mx > 0) {
				if (equals(st.c_str(), "ssdp:all")) {
					postResponse(mx);
				}
				else if (equals(st.c_str(), "upnp:rootdevice")) {
					postResponse(ROOT_FOR_ALL, mx);
				}
				else if (equals(st.c_str(), ("uuid:" + std::string(m_uuid)).c_str())) {
					postResponse(ROOT_BY_UUID, mx);
				}
				else if (equals(st.c_str(), m_deviceType)) {
					postResponse(ROOT_BY_TYPE, mx);
				}
			}
		}

		m_server->flush();
	}
	else 
	{
		unsigned long time = esp_timer_get_time();

		for (int i = 0; i < SSDP_QUEUE_SIZE; i++) {
			if (m_queue[i].time > 0 && m_queue[i].time < time) {
				send(&m_queue[i]);
			}
		}
	}
}

void SSDPDeviceClass::setSchemaURL(const char *url) {
	strlcpy(m_schemaURL, url, sizeof(m_schemaURL));
}

void SSDPDeviceClass::setHTTPPort(uint16_t port) {
	m_port = port;
}

void SSDPDeviceClass::setDeviceType(const char *deviceType) {
	strlcpy(m_deviceType, deviceType, sizeof(m_deviceType));
}

void SSDPDeviceClass::setName(const char *name) {
	strlcpy(m_friendlyName, name, sizeof(m_friendlyName));
}

void SSDPDeviceClass::setURL(const char *url) {
	strlcpy(m_presentationURL, url, sizeof(m_presentationURL));
}

void SSDPDeviceClass::setSerialNumber(const char *serialNumber) {
	strlcpy(m_serialNumber, serialNumber, sizeof(m_serialNumber));
}

void SSDPDeviceClass::setSerialNumber(const uint32_t serialNumber) {
	snprintf(m_serialNumber, sizeof(uint32_t) * 2 + 1, "%08X", serialNumber);
}

void SSDPDeviceClass::setModelName(const char *name) {
	strlcpy(m_modelName, name, sizeof(m_modelName));
}

void SSDPDeviceClass::setModelNumber(const char *num) {
	strlcpy(m_modelNumber, num, sizeof(m_modelNumber));
}

void SSDPDeviceClass::setModelURL(const char *url) {
	strlcpy(m_modelURL, url, sizeof(m_modelURL));
}

void SSDPDeviceClass::setManufacturer(const char *name) {
	strlcpy(m_manufacturer, name, sizeof(m_manufacturer));
}

void SSDPDeviceClass::setManufacturerURL(const char *url) {
	strlcpy(m_manufacturerURL, url, sizeof(m_manufacturerURL));
}

void SSDPDeviceClass::setTTL(const uint8_t ttl) {
	m_ttl = ttl;
}


