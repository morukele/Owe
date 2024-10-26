#include <stdlib.h>
#include <string.h>
#include "mqtt.h"
#include "pack.h"

static size_t unpack_mqtt_connect(const unsigned char *, union mqtt_header *, union mqtt_packet *);
static size_t unpack_mqtt_publish(const unsigned char *, union mqtt_header *, union mqtt_packet *);
static size_t unpack_mqtt_subscribe(const unsigned char *, union mqtt_header *, union mqtt_packet *);
static size_t unpack_mqtt_unsubscribe(const unsigned char *, union mqtt_header *, union mqtt_packet *);
static size_t unpack_mqtt_ack(const unsigned char *, union mqtt_header *, union mqtt_packet *);

static unsigned char *pack_mqtt_header(const union mqtt_header *);
static unsigned char *pack_mqtt_ack(const union mqtt_header *);
static unsigned char *pack_mqtt_connack(const union mqtt_header *);
static unsigned char *pack_mqtt_suback(const union mqtt_header *);
static unsigned char *pack_mqtt_publish(const union mqtt_header *);

/*
 * MQTT v3.1.1 standard, Remaining lenght field on the fixed header can be at most 4 bytes.
 */
static const int MAX_LEN_BYTES = 4;

/*
 * Encode Remaining Lenght on a MQTT packet header, comprised of Variable Header and Payload
 * if present. It does not take into account the types required to store itself. Refer to
 * MQTT v3.1.1 algorithm for the implementation.
 */
int mqtt_encode_length(unsigned char *buf, size_t len)
{
    int bytes = 0;
    do
    {
        if (bytes + 1 > MAX_LEN_BYTES) // check if maximum allowed bytes have been exceeded
            return bytes;
        short d = len % 128; // Get lowest 7 bits of the lenght
        len /= 128;          // Divide lenght by 128 for next iteration
        /* if there are more digits to encode, set the top bit of this digit */
        if (len > 0)
            d |= 128;     // Set MSB to 1 (128 in binary: 1000_0000)
        buf[bytes++] = d; // Store the encoded byte
    } while (len > 0);

    return bytes;
}

/*
 * Decode Remaining Lenght comprised of Variable Header and Payload if present.
 * It does not take into account the bytes for storing lenght. Refer to MQTT v3.1.1
 * algorithm for implementation suggestions.
 *
 * TODO Handle cases were multiplier > 128 * 128 * 128
 */
unsigned long long mqtt_decode_length(const unsigned char **buf)
{
    char c;
    int multiplier = 1;
    unsigned long long value = 0LL;
    do
    {
        c = **buf;                       // Get byte from buffer
        value += (c & 127) * multiplier; // Add 7 bits to value
        multiplier *= 128;               // Increase multiplier for next byte
        (*buf)++;                        // Move to next byte
    } while ((c & 128) != 0); // Continue if MSB is 1
    return value;
}