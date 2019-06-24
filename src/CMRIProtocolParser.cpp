/*
 * CMRI Protocole parser
 */

#include "CMRIProtocolParser.h"

CMRIProtocolParser::CMRIProtocolParser() :
  mState(WAIT_SYN1)
{
}

CMRIProtocolParser::operator<<(const unsigned char inChar)
{
  switch (mState)
  {
    case WAIT_SYN1:
      if (inChar == SYN_char) mState = WAIT_SYN2;
      else wrongCharError();
      break;
    case WAIT_SYN2:
      if (inChar == SYN_char) mState = WAIT_STX;
      else wrongCharError();
      break;
    case WAIT_STX:
      if (inChar == STX_char) mState = WAIT_ADDR;
      else wrongCharError();
      break;
    case WAIT_ADDR:
      /* An address should be between 65 and 127 + 65 */
      if (inChar >= kAdressOffset && inChar <= 127 + kAdressOffset) {
        mAdress = inChar - kAdressOffset;
        mState = WAIT_TYPE;
      }
      else wrongAddressError();
      break;
    case WAIT_TYPE:
      switch (inChar) {
        case INIT_TYPE:
        case POLL_TYPE:
        case RECE_TYPE:
        case TRAN_TYPE:
          mType = inChar;
          break;
        default:
          wrongTypeError();
          break;
      }
      break;
    case WAIT_DATA:
      switch (inChar) {
        case DLE_char:
          mState = WAIT_RAWD;
          break;
        case ETX_char:
          /* End of message, go back to initial state */
          mState = WAIT_SYN1;
          break;
        case SYN_char:
        case STX_char:
          wrongCharError();
          break;
      }
      break;
    case WAIT_RAWD:
      switch (inChar) {
        case DLE_char:
        case ETX_char:
        case SYN_char:
        case STX_char:
          mState = WAIT_DATA;
          break;
        default:
          wrongCharError();
          break;
      }
    default:
      internalError();
      break;
  }
}
