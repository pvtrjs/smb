#ifndef MQTT_H
#define MQTT_H

#include <stdio.h>

#define MQTT_HEADER_LEN 2
#define MQTT_ACK_LEN 4

/*
 * Stub bytes, useful for genertic replies, these represent the first byte in
 * the fixed header
 */

#define CONNACK_BYTE 0x20
#define PUBLISH_BYTE 0x30
#define PUBACK_BYTE 0x40
#define PUBREC_BYTE 0x50
#define PUBREL_BYTE 0x60
#define PUBCOMP_BYTE 0x70
#define SUBACK_BYTE 0x90
#define UNSUBACK_BYTE 0xB0
#define PINGRESP_BYTE 0xD0

/*message type*/

enum packet_type {
    CONNECT = 1,
    CONNACK = 2,
    PUBLISH = 3,
    PUBACK = 4,
    PUBREC = 5,
    PUBREL = 6,
    PUBCOMP = 7,
    SUBSCRIBE = 8,
    SUBACK = 9,
    UNSUBSCRIBE = 10,
    UNSUBACK = 11,
    PINGREQ = 12,
    PINGRESP = 13,
    DISCONNECT = 14
};

enum qos_level {
    AT_MOST_ONCE,
    AT_LEAST_ONCE,
    EXACTLY_ONCE
};

/*
# Fixed Header

 | Bit    | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 |--------|---------------|---------------|
 | Byte 1 | MQTT type     |  Flags        |
 |--------|-------------------------------|
 | Byte 2 |                               |
 |  .     |      Remaining Length         |
 |  .     |                               |
 | Byte 5 |                               |

*/

union mqtt_header {
    unsigned char byte;
    struct {
        unsigned retain : 1; // 0x01
        unsigned qos : 2;    // 0x06
        unsigned dup : 1;    // 0x08
        unsigned type : 4;   // 0xF0
    } bits;
};

struct mqtt_connect {
    union mqtt_header header; // Fixed header

    union {
        unsigned char byte;
        struct {
            int reserved : 1; // 0x01
            int clean_session : 1; // 0x02
            int will : 1; // 0x04
            int will_qos : 2; // 0x18
            int will_retain : 1; // 0x20
            int password : 1; // 0x40
            int username : 1; // 0x80
        } bits;
    } flags; // Variable header flags

    struct {
        unsigned short keep_alive; // Keep alive timer in seconds
        unsigned char *client_id;
        unsigned char *username;
        unsigned char *password;
        unsigned char *will_topic;
        unsigned char *will_message;
    } payload;
    
};

struct mqtt_connack {
    union mqtt_header header; // Fixed header

    union mqtt {
        /* data */
        unsigned char byte;
        struct {
            unsigned session_present : 1; // 0x01
            unsigned reserved : 7; // 0xFE
        } bits;
    } flags;

    unsigned char rc; // Return code
};

struct mqtt_subscribe {
    union mqtt_header header;
    unsigned short packet_id; // Packet identifier
    unsigned short tuples_len; // Length of the tuples array
    struct {
        unsigned short topic_len; // Length of the topic
        unsigned char *topic; // Topic name
        unsigned qos; // QoS level for the subscription
    } *tuples;
};

struct mqtt_unsubscribe {
    /* data */
    union mqtt_header header;
    unsigned short packet_id; // Packet identifier
    unsigned short tuples_len; // Length of the tuples array
    struct {
        unsigned short topic_len; // Length of the topic
        unsigned char *topic; // Topic name
    } *tuples;
    
};

struct mqtt_suback{
    union mqtt_header header;
    unsigned short packet_id; // Packet identifier
    unsigned short rcslen; // Length of the return codes array
    unsigned char *rcs; // Array of return codes for each subscription
};

struct mqtt_publish{
    union mqtt_header header; // Fixed header
    unsigned short packet_id; // Packet identifier (optional, used if QoS > 0)
    unsigned short topic_len; // Length of the topic
    unsigned char *topic; // Topic name
    unsigned short payload_len; // Length of the payload
    unsigned char *payload; // Payload data    
};

struct mqtt_ack {
    union mqtt_header header; // Fixed header
    unsigned short packet_id; // Packet identifier
};

typedef struct mqtt_ack mqtt_puback;
typedef struct mqtt_ack mqtt_pubrec;
typedef struct mqtt_ack mqtt_pubrel;
typedef struct mqtt_ack mqtt_pubcomp;
typedef struct mqtt_ack mqtt_unsuback;
typedef union mqtt_header mqtt_pingreq;
typedef union mqtt_header mqtt_pingresp;
typedef union mqtt_header mqtt_disconnect;

union mqtt_packet {
    struct mqtt_ack ack;
    union mqtt_header header;
    struct mqtt_connect connect;
    struct mqtt_connack connack;
    struct mqtt_suback suback;
    struct mqtt_publish publish;
    struct mqtt_subscribe subscribe;
    struct mqtt_unsubscribe unsubscribe;
};

int mqtt_encoder_length(unsigned char*, size_t);
unsigned long long mqtt_decode_length(const unsigned char **);
int unpack_mqtt_packet(const unsigned char *, union mqtt_packet *);
unsigned char *pack_mqtt_packet(const union mqtt_packet *, unsigned);

union mqtt_header *mqtt_packet_header(unsigned char);
struct mqtt_ack *mqtt_packet_ack(unsigned char, unsigned short);
struct mqtt_connack *mqtt_packet_connack(unsigned char, unsigned char, unsigned char);
struct mqtt_suback *mqtt_packet_suback(unsigned char, unsigned short,
                                       unsigned char *, unsigned short);
struct mqtt_publish *mqtt_packet_publish(unsigned char, unsigned short, size_t,
                                         unsigned char *, size_t, unsigned char *);
void mqtt_packet_release(union mqtt_packet *, unsigned);

#endif // MQTT_H