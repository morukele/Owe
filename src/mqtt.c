#include <stdlib.h>
#include <string.h>
#include "mqtt.h"

static size_t unpack_mqtt_connect(const unsigned char *, union mqtt_header *, union mqtt_packet *);