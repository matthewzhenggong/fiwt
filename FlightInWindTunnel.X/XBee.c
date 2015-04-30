/*
 * File:   XBee.c
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

#include "XBee.h"

#include <string.h>

inline void XBeeResponseReset(XBeeResponse_p _response) {
    _response->_complete = false;
    _response->_errorCode = NO_ERROR;
    _response->_checksum = 0;
    _response->_apiId = 0;
    _response->_msbLength = 0;
    _response->_lsbLength = 0;
    _response->_frameLength = 0;
}

inline uint16_t XBeeResponseGetPacketLength(struct XBeeResponse *_response) {
    return ((_response->_msbLength << 8) & 0xff00) + (_response->_lsbLength & 0xff);
}

void XBeeResetResponse(XBee_p _xbee) {
    _xbee->_pos = 0;
    _xbee->_escape = false;
    _xbee->_checksumTotal = 0;
    _xbee->_frameLength = 0;
    XBeeResponseReset(&_xbee->_response);
}

void XBeeInit(XBee_p _xbee, int AP, SerialStream_p serial) {
    XBeeResetResponse(_xbee);
    _xbee->_nextFrameId = 0;
    _xbee->_serial = serial;
    _xbee->_AP = AP;
}

signed int XBeeReadPacket(XBee_p _xbee) {
    uint8_t b;

    /*  reset previous response */
    if (_xbee->_response._complete || _xbee->_response._errorCode) {
        /*  discard previous packet and start over */
        XBeeResetResponse(_xbee);
    }

    while (_xbee->_serial->available()) {

        b = _xbee->_serial->read();

        if (_xbee->_AP == 2 && _xbee->_pos > 0) {
            if (b == START_BYTE) {
                /*  new packet start before previous packeted completed -- discard previous packet and start over */
                _xbee->_response._errorCode = UNEXPECTED_START_BYTE;
                return -UNEXPECTED_START_BYTE;
            } else if (b == ESCAPE) {
                if (_xbee->_serial->available()) {
                    b = _xbee->_serial->read();
                    b ^= 0x20;
                } else {
                    /*  escape byte.  next byte will be */
                    _xbee->_escape = true;
                    continue;
                }
            }
            if (_xbee->_escape == true) {
                b ^= 0x20;
                _xbee->_escape = false;
            }
        }

        /*  checksum includes all bytes starting with api id */
        if (_xbee->_pos >= API_ID_INDEX) {
            _xbee->_checksumTotal += b;
        }

        switch (_xbee->_pos) {
            case 0:
                if (b == START_BYTE) {
                    ++_xbee->_pos;
                }

                break;
            case 1:
                /*  length msb */
                _xbee->_response._msbLength = b;
                ++_xbee->_pos;

                break;
            case 2:
                /*  length lsb */
                _xbee->_response._lsbLength = b;
                ++_xbee->_pos;
                _xbee->_frameLength = XBeeResponseGetPacketLength(&_xbee->_response);

                break;
            case 3:
                _xbee->_response._apiId = b;
                ++_xbee->_pos;

                break;
            default:
                /*  starts at fifth byte */

                if (_xbee->_pos > MAX_FRAME_DATA_SIZE + 3) {
                    /*  exceed max size.  should never occur */
                    _xbee->_response._errorCode = PACKET_EXCEEDS_BYTE_ARRAY_LENGTH;
                    return -PACKET_EXCEEDS_BYTE_ARRAY_LENGTH;
                }

                /*  check if we're at the end of the packet */
                /*  packet length does not include start, length, or checksum bytes, so add 3 */
                if (_xbee->_pos == (_xbee->_frameLength + 3)) {
                    /*  verify checksum */

                    /* std::cout << "read checksum " << static_cast<unsigned int>(b) << " at pos " << static_cast<unsigned int>(_pos) << std::endl; */

                    if ((_xbee->_checksumTotal & 0xff) == 0xff) {
                        _xbee->_response._checksum = b;
                        _xbee->_response._complete = true;
                    } else {
                        /*  checksum failed */
                        _xbee->_response._errorCode = CHECKSUM_FAILURE;
                        return -CHECKSUM_FAILURE;
                    }

                    /*  minus 4 because we start after start,msb,lsb,api and up to but not including checksum */
                    /*  e.g. if frame was one byte, _pos=4 would be the byte, pos=5 is the checksum, where end stop reading */
                    _xbee->_response._frameLength = _xbee->_pos - 4;

                    /*  reset state vars */
                    _xbee->_pos = 0;

                    return _xbee->_response._apiId;
                } else {
                    /*  add to packet array, starting with the fourth byte of the apiFrame */
                    _xbee->_response._frameData[_xbee->_pos - 4] = b;
                    ++_xbee->_pos;
                }
        }
    }
    return NO_ERROR;
}

void XBeeSendByte(XBee_p _xbee, uint8_t b) {

    if (_xbee->_AP == 2 && (b == START_BYTE || b == ESCAPE || b == XON || b == XOFF)) {
        /* std::cout << "escaping byte [" << toHexString(b) << "] " << std::endl; */
        _xbee->_serial->write(ESCAPE);
        _xbee->_serial->write(b ^ 0x20);
    } else {
        _xbee->_serial->write(b);
    }
}

void XBeeRequestSend(XBee_p _xbee) {
    uint8_t *i, *frame_end;
    XBeeRequest_p request;

    request = &(_xbee->_request);

    /*  the new new deal */

    _xbee->_serial->write(START_BYTE);

    /*  send length */
    uint8_t msbLen = ((request->_frameLen + 2u) >> 8) & 0xff;
    uint8_t lsbLen = (request->_frameLen + 2u) & 0xff;

    XBeeSendByte(_xbee, msbLen);
    XBeeSendByte(_xbee, lsbLen);

    /*  api id */
    XBeeSendByte(_xbee, request->_apiId);
    XBeeSendByte(_xbee, request->_frameId);

    uint8_t checksum = 0;

    /*  compute checksum, start at api id */
    checksum += request->_apiId;
    checksum += request->_frameId;

    /* std::cout << "frame length is " << static_cast<unsigned int>(request.getFrameDataLength()) << std::endl; */

    for (i = request->_frameData, frame_end = request->_frameData + request->_frameLen; i != frame_end; ++i) {
        /* std::cout << "sending byte [" << static_cast<unsigned int>(i) << "] " << std::endl; */
        XBeeSendByte(_xbee, *i);
        checksum += *i;
    }

    /*  perform 2s complement */
    checksum = 0xff - checksum;

    /* std::cout << "checksum is " << static_cast<unsigned int>(checksum) << std::endl; */

    /*  send checksum */
    XBeeSendByte(_xbee, checksum);

    /* send one dummy byte*/
    _xbee->_serial->write(0xFE);
    /*  send packet */
    _xbee->_serial->flush();
}

uint8_t XBeeGetNextFrameId(XBee_p _xbee) {
    ++_xbee->_nextFrameId;

    if (_xbee->_nextFrameId == 0u) {
        /*  can't send 0 because that disables status response */
        _xbee->_nextFrameId = 1u;
    }

    return _xbee->_nextFrameId;
}

void XBeeAtCommandRequest(XBee_p _xbee, AtCommandRequest_p from,
        uint8_t frameId, bool queue) {
    XBeeRequest_p to;
    to = &(_xbee->_request);
    if (queue) {
        to->_apiId = AT_COMMAND_QUEUE_REQUEST;
    } else {
        to->_apiId = AT_COMMAND_REQUEST;
    }
    to->_frameId = frameId;
    to->_frameLen = AT_COMMAND_API_LENGTH + from->_commandValueLength;
    to->_frameData[0] = from->_command[0];
    to->_frameData[1] = from->_command[1];
    memcpy(to->_frameData + 2, from->_commandValue, from->_commandValueLength);
    XBeeRequestSend(_xbee);
}

void XBeeZBTxRequest(XBee_p _xbee, ZBTxRequest_p from, uint8_t frameId) {
    XBeeRequest_p to;
    to = &(_xbee->_request);
    to->_apiId = ZB_TX_REQUEST;
    to->_frameId = frameId;
    to->_frameLen = ZB_TX_API_LENGTH + from->_payloadLength;
    memcpy(to->_frameData, from->_addr64, 8);
    to->_frameData[8] = (from->_addr16 >> 8) & 0xff;
    to->_frameData[9] = from->_addr16 & 0xff;
    to->_frameData[10] = from->_broadcastRadius;
    to->_frameData[11] = from->_option;
    memcpy(to->_frameData + 12, from->_payloadPtr, from->_payloadLength);
    XBeeRequestSend(_xbee);
}

void XBeeTxA64Request(XBee_p _xbee, TxA64Request_p from, uint8_t frameId) {
    XBeeRequest_p to;
    to = &(_xbee->_request);
    to->_apiId = TX_A64_REQUEST;
    to->_frameId = frameId;
    to->_frameLen = TX_A64_API_LENGTH + from->_payloadLength;
    memcpy(to->_frameData, from->_addr64, 8);
    to->_frameData[8] = from->_option;
    memcpy(to->_frameData + 9, from->_payloadPtr, from->_payloadLength);
    XBeeRequestSend(_xbee);
}

void XBeeTxA16Request(XBee_p _xbee, TxA16Request_p from, uint8_t frameId) {
    XBeeRequest_p to;
    to = &(_xbee->_request);
    to->_apiId = TX_A16_REQUEST;
    to->_frameId = frameId;
    to->_frameLen = TX_A16_API_LENGTH + from->_payloadLength;
    to->_frameData[0] = (from->_addr16 >> 8) & 0xff;
    to->_frameData[1] = from->_addr16 & 0xff;
    to->_frameData[2] = from->_option;
    memcpy(to->_frameData + 3, from->_payloadPtr, from->_payloadLength);
    XBeeRequestSend(_xbee);
}

void XBeeTxIPv4Request(XBee_p _xbee, TxIPv4Request_p from, uint8_t frameId) {
    XBeeRequest_p to;
    to = &(_xbee->_request);
    to->_apiId = TX_IPV4_REQUEST;
    to->_frameId = frameId;
    to->_frameLen = TX_IPV4_API_LENGTH + from->_payloadLength;
    to->_frameData[0] = (from->_des_addr_msw >> 8) & 0xff;
    to->_frameData[1] = (from->_des_addr_msw) & 0xff;
    to->_frameData[2] = (from->_des_addr_lsw >> 8) & 0xff;
    to->_frameData[3] = (from->_des_addr_lsw) & 0xff;
    to->_frameData[4] = (from->_des_port >> 8) & 0xff;
    to->_frameData[5] = (from->_des_port) & 0xff;
    to->_frameData[6] = (from->_src_port >> 8) & 0xff;
    to->_frameData[7] = (from->_src_port) & 0xff;
    to->_frameData[8] = from->_protocol;
    to->_frameData[9] = from->_option;
    memcpy(to->_frameData + 10, from->_payloadPtr, from->_payloadLength);
    XBeeRequestSend(_xbee);
}


bool XBeeAtCommandResponse(XBee_p _xbee, AtCommandResponse_p to) {
    XBeeResponse_p from;
    from = &_xbee->_response;
    if (from->_complete && from->_errorCode == NO_ERROR && from->_apiId == AT_COMMAND_RESPONSE) {
        to->_frameId = from->_frameData[0];
        to->_command[0] = from->_frameData[1];
        to->_command[1] = from->_frameData[2];
        to->_commandStatus = from->_frameData[3];
        to->_commandValueLength = from->_frameLength - 4;
        memcpy(to->_commandValue, from->_frameData + 4, to->_commandValueLength);
        return true;
    }
    return false;
}

bool XBeeTxStatusResponse(XBee_p _xbee, TxStatusResponse_p to) {
    XBeeResponse_p from;
    from = &_xbee->_response;
    if (from->_complete && from->_errorCode == NO_ERROR && from->_apiId == TX_STATUS_RESPONSE) {
        to->_frameId = from->_frameData[0];
        to->_deliveryStatus = from->_frameData[1];
        return true;
    }
    return false;
}

bool XBeeZBTxStatusResponse(XBee_p _xbee, ZBTxStatusResponse_p to) {
    XBeeResponse_p from;
    from = &_xbee->_response;
    if (from->_complete && from->_errorCode == NO_ERROR && from->_apiId == ZB_TX_STATUS_RESPONSE) {
        to->_frameId = from->_frameData[0];
        to->_addr16 = (from->_frameData[1] << 8) + from->_frameData[2];
        to->_retryCount = from->_frameData[3];
        to->_deliveryStatus = from->_frameData[4];
        to->_discoveryStatus = from->_frameData[5];
        return true;
    }
    return false;
}

bool XBeeZBRxResponse(XBee_p _xbee, ZBRxResponse_p to) {
    XBeeResponse_p from;
    from = &_xbee->_response;
    if (from->_complete && from->_errorCode == NO_ERROR && from->_apiId == ZB_RX_RESPONSE) {
        memcpy(to->_addr64, from->_frameData, 8);
        to->_addr16 = (from->_frameData[8] << 8) + from->_frameData[9];
        to->_option = from->_frameData[10];
        to->_payloadLength = from->_frameLength - 11;
        memcpy(to->_payloadPtr, from->_frameData + 11, to->_payloadLength);
        return true;
    }
    return false;
}

bool XBeeRxA64Response(XBee_p _xbee, RxA64Response_p to) {
    XBeeResponse_p from;
    from = &_xbee->_response;
    if (from->_complete && from->_errorCode == NO_ERROR && from->_apiId == RX_A64_RESPONSE) {
        memcpy(to->_addr64, from->_frameData, 8);
        to->_rssi = from->_frameData[8];
        to->_option = from->_frameData[9];
        to->_payloadLength = from->_frameLength - 10;
        memcpy(to->_payloadPtr, from->_frameData + 10, to->_payloadLength);
        return true;
    }
    return false;
}

bool XBeeRxA16Response(XBee_p _xbee, RxA16Response_p to) {
    XBeeResponse_p from;
    from = &_xbee->_response;
    if (from->_complete && from->_errorCode == NO_ERROR && from->_apiId == RX_A16_RESPONSE) {
        to->_addr16 = (from->_frameData[0] << 8) + from->_frameData[1];
        to->_rssi = from->_frameData[2];
        to->_option = from->_frameData[3];
        to->_payloadLength = from->_frameLength - 4;
        memcpy(to->_payloadPtr, from->_frameData + 4, to->_payloadLength);
        return true;
    }
    return false;
}

bool XBeeRxIPv4Response(XBee_p _xbee, RxIPv4Response_p to) {
    XBeeResponse_p from;
    from = &_xbee->_response;
    if (from->_complete && from->_errorCode == NO_ERROR && from->_apiId == RX_IPV4_RESPONSE) {
        to->_src_addr_hsw = (from->_frameData[0] << 8) + from->_frameData[1];
        to->_src_addr_lsw = (from->_frameData[2] << 8) + from->_frameData[3];
        to->_port = (from->_frameData[4] << 8) + from->_frameData[5];
        to->_src_port = (from->_frameData[6] << 8) + from->_frameData[7];
        to->_protocol = from->_frameData[8];
        to->_status = from->_frameData[9];
        to->_payloadLength = from->_frameLength - 10;
        memcpy(to->_payloadPtr, from->_frameData + 10, to->_payloadLength);
        return true;
    }
    return false;
}
