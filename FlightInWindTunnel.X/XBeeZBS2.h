/*
 * File:   XBeeZBS2.h
 * Author: Zheng GONG(matthewzhenggong@gmail.com)
 * Modified from XBee-Arduino project<https://code.google.com/p/xbee-arduino/>
 *
 * This file is part of FIWT.
 *
 * FIWT is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

#ifndef XBeeZBS2_H
#define XBeeZBS2_H

#include "SerialStream.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*  set to ATAP value of XBee. AP=2 is recommended */
#define ATAP 2

#define START_BYTE 0x7e
#define ESCAPE 0x7d
#define XON 0x11
#define XOFF 0x13

/*  This value determines the size of the byte array for receiving RX packets */
/*  Most users won't be dealing with packets this large so you can adjust this */
/*  value to reduce memory consumption. But, remember that */
/*  if a RX packet exceeds this size, it cannot be parsed! */

/*  This value is determined by the largest packet size */
#define MAX_PAYLOAD_DATA_SIZE 84
#define MAX_FRAME_DATA_SIZE 104

#define BROADCAST_ADDRESS 0xffff
#define ZB_BROADCAST_ADDRESS 0xfffe

/*  the non-variable length of the frame data (not including frame id or api id or variable data size (e.g. payload, at command set value) */
#define ZB_TX_API_LENGTH 12
#define AT_COMMAND_API_LENGTH 2
#define AT_COMMAND_API_VAL_MAXLEN 16
#define REMOTE_AT_COMMAND_API_LENGTH 13
/*  start/length(2)/api/frameid/checksum bytes */
#define PACKET_OVERHEAD_LENGTH 6
/*  api is always the third byte in packet */
#define API_ID_INDEX 3

#define DEFAULT_FRAME_ID 1
#define NO_RESPONSE_FRAME_ID 0

/**
 * Api Id constants
 */
#define AT_COMMAND_REQUEST 0x08
#define AT_COMMAND_QUEUE_REQUEST 0x09
#define ZB_TX_REQUEST 0x10
#define ZB_EXPLICIT_TX_REQUEST 0x11
#define REMOTE_AT_REQUEST 0x17
#define AT_COMMAND_RESPONSE 0x88
#define MODEM_STATUS_RESPONSE 0x8a
#define ZB_TX_STATUS_RESPONSE 0x8b
#define ZB_RX_RESPONSE 0x90
#define ZB_EXPLICIT_RX_RESPONSE 0x91
#define ZB_IO_SAMPLE_RESPONSE 0x92
#define ZB_IO_NODE_IDENTIFIER_RESPONSE 0x95
#define REMOTE_AT_COMMAND_RESPONSE 0x97


/**
 * TX STATUS constants
 */
#define SUCCESS 0x0
#define CCA_FAILURE 0x2
#define INVALID_DESTINATION_ENDPOINT_SUCCESS 0x15
#define NETWORK_ACK_FAILURE 0x21
#define NOT_JOINED_TO_NETWORK 0x22
#define SELF_ADDRESSED 0x23
#define ADDRESS_NOT_FOUND 0x24
#define ROUTE_NOT_FOUND 0x25
#define PAYLOAD_TOO_LARGE 0x74

/*  modem status */
#define HARDWARE_RESET 0
#define WATCHDOG_TIMER_RESET 1
#define ASSOCIATED 2
#define DISASSOCIATED 3
#define COORDINATOR_REALIGNMENT 5
#define COORDINATOR_STARTED 6

#define ZB_BROADCAST_RADIUS_MAX_HOPS 0

#define ZB_TX_UNICAST 0
#define ZB_TX_BROADCAST 8

#define AT_OK 0
#define AT_ERROR  1
#define AT_INVALID_COMMAND 2
#define AT_INVALID_PARAMETER 3
#define AT_NO_RESPONSE 4


#define NO_ERROR 0
#define CHECKSUM_FAILURE 1
#define PACKET_EXCEEDS_BYTE_ARRAY_LENGTH 2
#define UNEXPECTED_START_BYTE 3

#ifdef __cplusplus
extern "C" {
#endif

    struct XBeeResponse {
        uint8_t _apiId;
        uint8_t _msbLength;
        uint8_t _lsbLength;
        uint8_t _checksum;
        size_t _frameLength;
        uint8_t _frameData[MAX_FRAME_DATA_SIZE - 1];
        bool _complete;
        uint8_t _errorCode;
    };
    typedef struct XBeeResponse XBeeResponse_t;
    typedef struct XBeeResponse * XBeeResponse_p;

    struct AtCommandResponse {
        uint8_t _frameId;
        uint8_t _command[AT_COMMAND_API_LENGTH];
        uint8_t _commandStatus;
        uint8_t _commandValue[AT_COMMAND_API_VAL_MAXLEN];
        size_t _commandValueLength;
    };
    typedef struct AtCommandResponse AtCommandResponse_t;
    typedef struct AtCommandResponse * AtCommandResponse_p;

    struct ZBTxStatusResponse {
        uint8_t _frameId;
        uint16_t _addr16;
        uint8_t _retryCount;
        uint8_t _deliveryStatus;
        size_t _discoveryStatus;
    };
    typedef struct ZBTxStatusResponse ZBTxStatusResponse_t;
    typedef struct ZBTxStatusResponse * ZBTxStatusResponse_p;

    struct ZBRxResponse {
        uint8_t _addr64[8];
        uint16_t _addr16;
        uint8_t _option;
        uint8_t _payloadPtr[MAX_PAYLOAD_DATA_SIZE];
        size_t _payloadLength;
    };
    typedef struct ZBRxResponse ZBRxResponse_t;
    typedef struct ZBRxResponse * ZBRxResponse_p;

    struct XBeeRequest {
        uint8_t _apiId;
        uint8_t _frameId;
        size_t _frameLen;
        uint8_t _frameData[MAX_FRAME_DATA_SIZE - 2];
    };
    typedef struct XBeeRequest XBeeRequest_t;
    typedef struct XBeeRequest * XBeeRequest_p;

    struct AtCommandRequest {
        uint8_t _command[AT_COMMAND_API_LENGTH];
        uint8_t _commandValue[AT_COMMAND_API_VAL_MAXLEN];
        size_t _commandValueLength;
    };
    typedef struct AtCommandRequest AtCommandRequest_t;
    typedef struct AtCommandRequest * AtCommandRequest_p;

    struct ZBTxRequest {
        uint8_t _addr64[8];
        uint16_t _addr16;
        uint8_t _broadcastRadius;
        uint8_t _option;
        uint8_t _payloadPtr[MAX_PAYLOAD_DATA_SIZE];
        size_t _payloadLength;
    };
    typedef struct ZBTxRequest ZBTxRequest_t;
    typedef struct ZBTxRequest * ZBTxRequest_p;

    struct XBee {
        bool _escape;
        /*  current packet position for response.  just a state variable for packet parsing and has no relevance for the response otherwise */
        uint8_t _pos;
        uint8_t _checksumTotal;
        size_t _frameLength;
        uint8_t _nextFrameId;
        /*  buffer for incoming RX packets.  holds only the api specific frame data, starting after the api id byte and prior to checksum */
        XBeeResponse_t _response;
        XBeeRequest_t _request;
        SerialStream_p _serial;
    };
    typedef struct XBee XBee_t;
    typedef struct XBee * XBee_p;

    void XBeeInit(XBee_p _xbee, SerialStream_p serial);

    /**
     * Reads all available serial bytes until a packet is parsed, an error occurs, or the buffer is empty.
     * You may call <i>xbee</i>.getResponse().isAvailable() after calling this method to determine if
     * a packet is ready, or <i>xbee</i>.getResponse().isError() to determine if
     * a error occurred.
     * <p/>
     * This method should always return quickly since it does not wait for serial data to arrive.
     * You will want to use this method if you are doing other timely stuff in your loop, where
     * a delay would cause problems.
     * NOTE: calling this method resets the current response, so make sure you first consume the
     * current response
     *
     * @return 0 : no new pack
     *          >0 : new pack id
     *          <0 : error code
     */
    signed int XBeeReadPacket(XBee_p _xbee);

    extern inline void XBeeSetSerial(XBee_p _xbee, SerialStream_p serial) {
        _xbee->_serial = serial;
    }

    /**
     * Returns a sequential frame id between 1 and 255
     */
    uint8_t XBeeGetNextFrameId(XBee_p _xbee);

    void XBeeAtCommandRequest(XBee_p _xbee, AtCommandRequest_p from, uint8_t frameId, bool queue);

    void XBeeZBTxRequest(XBee_p _xbee, ZBTxRequest_p from, uint8_t frameId);

    bool XBeeAtCommandResponse(XBee_p _xbee, AtCommandResponse_p to);

    extern inline uint8_t XBeeModemStatusResponse(XBeeResponse_p from) {
        return from->_frameData[0];
    }

    bool XBeeZBTxStatusResponse(XBee_p _xbee, ZBTxStatusResponse_p to);

    bool XBeeZBRxResponse(XBee_p _xbee, ZBRxResponse_p to);

#ifdef __cplusplus
}
#endif

#endif /* XBeeZBS2_H */

