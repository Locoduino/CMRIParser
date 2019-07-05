/*
 * CMRI Protocole parser
 */

#include "CMRIParser.h"

CMRIParser::CMRIParser() :
  mParserState(WAIT_SYN1),
  mLocation(0),
  mCurrentBoard(NULL),
  mErrorHandler(NULL)
{
}

void CMRIParser::operator<<(const unsigned char inChar)
{
  mLocation++;
  switch (mParserState)
  {
    case WAIT_SYN1:
      if (inChar == SYN_char) mParserState = WAIT_SYN2;
      else wrongCharError();
      break;
    case WAIT_SYN2:
      if (inChar == SYN_char) mParserState = WAIT_STX;
      else wrongCharError();
      break;
    case WAIT_STX:
      if (inChar == STX_char) mParserState = WAIT_ADDR;
      else wrongCharError();
      break;
    case WAIT_ADDR:
      /* An address should be between 65 and 127 + 65 */
      if (inChar >= kAdressOffset && inChar <= 127 + kAdressOffset) {
        uint8_t address = inChar - kAdressOffset;
        mCurrentBoard = CMRIBoard::boardForAddress(address);
        mParserState = WAIT_TYPE;
      }
      else wrongAddressError();
      break;
    /* type of frame coming from the host */
    case WAIT_TYPE:
      mParserState = WAIT_DATA;
      switch (inChar) {
        case INIT_TYPE:
          mCurrentBoard->init();
          break;
        case POLL_TYPE:
          mCurrentBoard->poll();
          break;
        case TRAN_TYPE:
          mCurrentBoard->transmit();
          break;
        default:
          /* reset the state */
          mParserState = WAIT_TYPE;
          wrongTypeError();
          break;
      }
      break;
    case WAIT_DATA:
      switch (inChar) {
        case DLE_char:
          mParserState = WAIT_RAWD;
          break;
        case ETX_char:
          /* End of message, go back to initial state */
          mCurrentBoard->endOfFrame();
          mLocation = 0;
          mParserState = WAIT_SYN1;
          break;
        case SYN_char:
        case STX_char:
          wrongCharError();
          break;
        default:
          mCurrentBoard->feed(inChar);
          break;
      }
      break;
    case WAIT_RAWD:
      switch (inChar) {
        case DLE_char:
        case ETX_char:
        case SYN_char:
        case STX_char:
          mCurrentBoard->feed(inChar);
          mParserState = WAIT_DATA;
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

void CMRIParser::wrongCharError()
{
  if (mErrorHandler != NULL) {
    mErrorHandler(WRONG_CHAR, mLocation);
  }
}

void CMRIParser::wrongAddressError()
{
  if (mErrorHandler != NULL) {
    mErrorHandler(WRONG_ADDRESS, mLocation);
  }
}

void CMRIParser::wrongTypeError()
{
  if (mErrorHandler != NULL) {
    mErrorHandler(WRONG_TYPE, mLocation);
  }
}

void CMRIParser::internalError()
{
  if (mErrorHandler != NULL) {
    mErrorHandler(INTERNAL_ERROR, mLocation);
  }
}
