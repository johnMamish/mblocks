/**
 *
 */

#ifndef _SERIAL_DECODER_H
#define _SERIAL_DECODER_H

typedef struct SerialDecoderBuffer
{
    char* c;
    int capacity;
    int size;
} SerialDecoderBuffer;

typedef enum SerialResponse
{
    RESPONSE_NONE_YET = -1,
    RESPONSE_UNKNOWN,
    RESPONSE_MECHBRAKE_SUCCESS,
    RESPONSE_MECHBRAKE_FAILURE,
    RESPONSE_MECHBRAKE_TIMEOUT,
    RESPONSE_SMA_RETRACTED,
    RESPONSE_SMA_EXTENDING,
    RESPONSE_SMA_EXTENDED,
    RESPONSE_BLDC_STABLE,
    RESPONSE_BLDC_TIMEOUT,
    RESPONSE_BLDC_ACCEL_COMPLETE,
    RESPONSE_BLDC_COASTING,
    RESPONSE_BLDC_STOPPED,
    RESPONSE_COMMAND_UNRECOGNIZED
} SerialResponse;

typedef struct SerialResponsePairing
{
    const char* c;
    SerialResponse sr;
} SerialResponsePairing;

extern const SerialResponsePairing pairings[];

SerialResponse pushNewChar(SerialDecoderBuffer*, int);

#endif
