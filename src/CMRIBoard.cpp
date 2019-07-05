/*
 * CMRI Board emulation
 *
 * Copyright Jean-Luc Béchennec 2019
 *
 * This software is distributed under the GNU Public Licence v2 (GPLv2)
 *
 * Please read the LICENCE file
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "CMRIBoard.h"
#include "CMRIParser.h"

CMRIBoard *CMRIBoard::sBoardList = NULL;

/*
 * CMRIBoard constructor.
 * Build the list
 */
CMRIBoard::CMRIBoard(const uint8_t inAddress) :
  mAddress(inAddress),
  mNextBoard(NULL),
  mProtocolState(WAIT_INIT)
{
  CMRIBoard *currentBoard = sBoardList;
  bool alreadyAllocatedAddress = false;
  while (currentBoard != NULL) {
    if (currentBoard->mAddress == inAddress) {
      alreadyAllocatedAddress = true;
      break;
    }
    currentBoard = currentBoard->mNextBoard;
  }
  if (! alreadyAllocatedAddress) {
    /* Add the board at front */
    mNextBoard = sBoardList;
    sBoardList = this;
  }
}

/*
 * return a board from its address. NULL if iot does not exist
 */
CMRIBoard *CMRIBoard::boardForAddress(uint8_t inAddress)
{
  CMRIBoard *currentBoard = sBoardList;
  while (currentBoard != NULL) {
    if (currentBoard->mAddress == inAddress) break;
    currentBoard = currentBoard->mNextBoard;
  }
  return currentBoard;
}

/*
 * Protocol automaton functions
 */
void CMRIBoard::init()
{
  if (mProtocolState == WAIT_INIT) mProtocolState = WAIT_NDP;
  else protocolError();
}

void CMRIBoard::poll()
{
  if (mProtocolState == WAIT_POLL) {
    mProtocolState = WAIT_ENDPOLL;
  }
  else protocolError();
}

bool CMRIBoard::receive(CMRIParser& inParser)
{
  bool notFinished = true;

  switch (mProtocolState) {
    case WAIT_DELAY:
      if ((micros() - mDelayDate) >= mDelay) {
        mProtocolState = DO_RECEIVE;
        mIndex = 0;
        inParser.beginReceiveFrame(mAddress);
      }
      break;
    case DO_RECEIVE:
      if (mIndex < numberOfInputBytes()) {
        inParser.sendDataToHost(input(mIndex));
        mIndex++;
      }
      else {
        mProtocolState = WAIT_TRANSMIT;
        notFinished = false;
      }
      break;
    default:
      protocolError();
      break;
  }
  return notFinished;
}

void CMRIBoard::transmit()
{
  if (mProtocolState == WAIT_TRANSMIT) {
    mProtocolState = DO_TRANSMIT;
    mIndex = 0;
  }
  else protocolError();
}

void CMRIBoard::endOfFrame()
{
  switch (mProtocolState) {
    case DO_INIT:
      if (mIndex == mNumberOfSets) mProtocolState = WAIT_POLL;
      else initError();
      break;
    case WAIT_ENDPOLL:
      mDelayDate = micros();
      mProtocolState = WAIT_DELAY;
      break;
    case DO_TRANSMIT:
      mProtocolState = WAIT_POLL;
      break;
    default:
      protocolError();
      break;
  }
}

void CMRIBoard::feed(const uint8_t inChar)
{
  switch (mProtocolState) {
    case WAIT_NDP:
      if (inChar == boardType()) mProtocolState = WAIT_DH;
      else wrongTypeError();
      break;
    case WAIT_DH:
      mDelay = ((uint16_t)inChar) << 8;
      mProtocolState = WAIT_DL;
      break;
    case WAIT_DL:
      mDelay |= inChar;
      mProtocolState = WAIT_NS;
      break;
    case WAIT_NS:
      mNumberOfSets = inChar;
      mProtocolState = DO_INIT;
      mIndex = 0;
      break;
    case DO_INIT:
      if (mIndex < mNumberOfSets) {
        processInitSet(inChar);
        mIndex++;
      }
      else protocolError();
      break;
    case DO_TRANSMIT:
      output(mIndex++) = inChar;
      break;
    default:
      protocolError();
      break;
  }
}

/*
 * returns the number of input bytes in the vector
 */
uint16_t CMRIBoard::numberOfInputBytes() const
{
  uint8_t numInputBits = numberOfInputBits();
  return (numInputBits / 8) + ((numInputBits % 8) != 0);
}

/*
 * returns the number of output bytes in the vector
 */
uint16_t CMRIBoard::numberOfOutputBytes() const
{
  uint8_t numOutputBits = numberOfOutputBits();
  return (numOutputBits / 8) + ((numOutputBits % 8) != 0);
}
