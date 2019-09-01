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

#ifndef __CMRI_BOARD_H__
#define __CMRI_BOARD_H__

#include <Arduino.h>

class CMRIParser;

/*
 * Abstract class for CMRI boards base class
 *
 * This base class manages a list of boards. Each time a board is intancied
 * it is added to the list provided the address is not already in use.
 *
 * A board is seen as 2 vectors of bits, one vector of inputs
 * and one vector of outputs.
 */
class CMRIBoard {
  friend class CMRIParser;
private:
  uint8_t   mAddress;           /* Board address, between 0 and 127             */
  uint8_t   mNumberOfSets;      /* Number of sets in the init message           */
  uint8_t   mIndex;             /* Index used to enumerate data                 */
  uint16_t  mDelay;             /* Delay before receive for lazy hosts          */
  uint16_t  mDelayDate;         /* Date to start waiting for delay              */
  CMRIBoard *mNextBoard;        /* Pointer to next board in the list            */
  static CMRIBoard *sBoardList;

  typedef enum {
    WAIT_INIT,      /* initial state, wait for the init data      */
    WAIT_NDP,       /* wait the Node Definition Parameter         */
    WAIT_DH,        /* wait the transmission delay high           */
    WAIT_DL,        /* wait the transmission delay high           */
    WAIT_NS,        /* wait the number of sets                    */
    DO_INIT,        /* in the process of receiving data sets      */
    WAIT_POLL,      /* wait for a POLL request                    */
    WAIT_ENDPOLL,   /* wait for the end of a POLL request         */
    WAIT_DELAY,     /* wait until the delay elapsed               */
    DO_RECEIVE,     /* do a RECEIVE                               */
    WAIT_TRANSMIT,  /* wait for a TRANSMIT request                */
    DO_TRANSMIT     /* in the process of receiving data from host */
  } ProtocolState;

  ProtocolState mProtocolState;

  void init();
  void poll();
  bool receive(CMRIParser& inParser);
  void transmit();
  void feed(const uint8_t inChar);
  void endOfFrame();

public:
  typedef enum {
    PROTOCOL_ERROR,
    INIT_ERROR,
    WRONG_TYPE,
  } CMRIBoardError;

  /* Error handlers */
  void (*mErrorHandler)(CMRIBoardError, CMRIBoard *);

  void protocolError();
  void initError();
  void wrongTypeError();

protected:
  /* bytes accessor */
  virtual uint8_t &input(const uint16_t inIndex) = 0;
  virtual uint8_t &output(const uint16_t inIndex) = 0;
  virtual void processInitSet(const uint8_t inChar) = 0;

public:
  CMRIBoard(const uint8_t inAddress);
  /* return the type of the board : N, X, M or C */
  virtual uint8_t boardType() const = 0;
  /* return a board from its address. NULL if iot does not exist                */
  static CMRIBoard *boardForAddress(uint8_t inAddress);
  /* returns the number of input bits in the vector                             */
  virtual uint16_t numberOfInputBits() const = 0;
  /* returns the number of output bits in the vector                            */
  virtual uint16_t numberOfOutputBits() const = 0;
  /* returns the number of input bytes in the vector                            */
  uint16_t numberOfInputBytes() const ;
  /* returns the number of output bytes in the vector                           */
  uint16_t numberOfOutputBytes() const;
  /* return the nth input byte if it exists, 0 otherwise. Used to do a receive  */
  uint8_t getInputByte(const uint16_t num) const;
  /* return the nth output byte if it exists, 0 otherwise                       */
  uint8_t getOutputByte(const uint16_t num) const;
  /* set the nth input byte if it exists, does nothing otherwise                */
  void setInputByte(const uint16_t num, const uint8_t val);
  /* set the nth input byte if it exists, does nothing otherwise                */
  void setOutputByte(const uint16_t num, const uint8_t val);
  /* return the nth bit if it exists, 0 otherwise                               */
  uint8_t getOutputBit(const uint16_t num) const;
  /* set the nth bit if it exists, does nothing otherwise                       */
  void setInputBit(const uint16_t num, const uint8_t val);
};

class SMINIBoard : public CMRIBoard {
private:
  uint8_t mInputs[3];   /* 24 input lines  */
  uint8_t mOutputs[6];  /* 48 output lines */
protected:
  /* bytes accessor */
  virtual uint8_t &input(const uint16_t inIndex);
  virtual uint8_t &output(const uint16_t inIndex);
  virtual void processInitSet(const uint8_t inChar);
public:
  /* constructor */
  SMINIBoard(const uint8_t inAddress) : CMRIBoard(inAddress) {}
  /* return the type of the board : M for a SMINI */
  virtual uint8_t boardType() const { return 'M'; }
  /* returns the number of input bits in the vector                             */
  virtual uint16_t numberOfInputBits() { return 24; };
  /* returns the number of output bits in the vector                            */
  virtual uint16_t numberOfOutputBits() { return 48; };
};

#endif
