/*
 * CMRI Protocole parser
 */

#ifndef __CMRI_PROTOCOL_PARSER_H__
#define __CMRI_PROTOCOL_PARSER_H__

class CMRIProtocolParser {

  /* Protocol special characters */
  const unsigned char SYN_char = 0xFF;
  const unsigned char STX_char = 0x02;
  const unsigned char ETX_char = 0x03;
  const unsigned char DLE_char = 0x10;

  /* Address offset */
  const uint8_t kAdressOffset = 65;

  /* Possible message types */
  const unsigned char INIT_TYPE = 'I'; /* Init message            */
  const unsigned char POLL_TYPE = 'P'; /* Poll request message    */
  const unsigned char RECE_TYPE = 'R'; /* Receive data message    */
  const unsigned char TRAN_TYPE = 'T'; /* Transmit data message   */

  typedef enum {
    WAIT_SYN1, /* In this state we wait for a first SYN           */
    WAIT_SYN2, /* In this state we wait for a second SYN          */
    WAIT_STX,  /* In this state we wait for STX                   */
    WAIT_ADDR, /* In this state we wait for the address           */
    WAIT_TYPE, /* In this state we wait for the type of message   */
    WAIT_DATA, /* In this state we wait for data in message       */
    WAIT_RAWD, /* In this state we wait for data escaped with DLE */
  } ParsingState;

  ParsingState mState;

public:
  CMRIProtocolParser();
  void operator<<(const unsigned char inChar);
};

#endif
