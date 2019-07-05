/*
 * CMRI Protocole parser
 */

#ifndef __CMRI_PROTOCOL_PARSER_H__
#define __CMRI_PROTOCOL_PARSER_H__

#include <Arduino.h>
#include "CMRIBoard.h"

class CMRIParser {

  friend class CMRIBoard;

public:
  typedef enum {
    WRONG_CHAR,
    WRONG_ADDRESS,
    WRONG_TYPE,
    INTERNAL_ERROR
  } CMRIParserError;
private:
  /* Protocol special characters */
  static const unsigned char SYN_char = 0xFF;
  static const unsigned char STX_char = 0x02;
  static const unsigned char ETX_char = 0x03;
  static const unsigned char DLE_char = 0x10;

  /* Address offset */
  static const uint8_t kAdressOffset = 65;

  /* Possible message types */
  static const unsigned char INIT_TYPE = 'I'; /* Init message            */
  static const unsigned char POLL_TYPE = 'P'; /* Poll request message    */
  static const unsigned char RECE_TYPE = 'R'; /* Receive data message    */
  static const unsigned char TRAN_TYPE = 'T'; /* Transmit data message   */

  typedef enum {
    WAIT_SYN1, /* In this state we wait for a first SYN           */
    WAIT_SYN2, /* In this state we wait for a second SYN          */
    WAIT_STX,  /* In this state we wait for STX                   */
    WAIT_ADDR, /* In this state we wait for the address           */
    WAIT_TYPE, /* In this state we wait for the type of message   */
    WAIT_DATA, /* In this state we wait for data in message       */
    WAIT_RAWD, /* In this state we wait for data escaped with DLE */
  } ParsingState;

  ParsingState mParserState;
  uint16_t mLocation;
  CMRIBoard *mCurrentBoard;

  /* Error handlers */
  void (*mErrorHandler)(CMRIParserError, uint16_t);
  void wrongCharError();
  void wrongAddressError();
  void wrongTypeError();
  void internalError();

  /* Methods to manage the receive */
  void beginReceiveFrame(const uint8_t inAddress);
  void sendDataToHost(const uint8_t inData);

public:
  CMRIParser();
  void operator<<(const unsigned char inChar);
};

#endif
