/*
 * AddicoreRFID.h - an Arduino library for use with the Addicore RFID-RC522 Module which you
   can acquire at www.addicore.com
 * AUTHORS: Craig Thompson, Aaron Norris, Alex McKay
 * Created: August 2014
 * Updated: January 2020
 
 ===========================================================================
 Copyright (c) 2020 Addicore.  All rights reserved.
 
 The AddicoreRFID Arduino Library is free software: you can redistribute it
 and/or modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 The AddicoreRFID Arduino Library is distributed in the hope that it will be
 useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with The Arduino TLC5940 Library.  If not, see
 <http://www.gnu.org/licenses/> or write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA 
 
 ===========================================================================
 GLOSSARY:
   CRC - cyclic redundancy check. is an error-detecting code commonly used 
         in digital networks and storage devices to detect accidental 
         changes to raw data. Blocks of data entering these systems get a 
         short check value attached, based on the remainder of a polynomial 
         division of their contents; on retrieval the calculation is 
         repeated, and corrective action can be taken against presumed data 
         corruption if the check values do not match.
         CRCs are so called because the check (data verification) value is a 
         redundancy (it expands the message without adding information) and 
         the algorithm is based on cyclic codes. CRCs are popular because 
         they are simple to implement in binary hardware, easy to analyze 
         mathematically, and particularly good at detecting common errors 
         caused by noise in transmission channels. Because the check value 
         has a fixed length, the function that generates it is occasionally 
         used as a hash function.
*/

#ifndef AddicoreRFID_h
#define AddicoreRFID_h

#include <Arduino.h>
#include <SPI.h>
#include <inttypes.h>

//#define   uchar   unsigned char
#define uint    unsigned int

//AddicoreRFID Primary Coupling Device (PCD according to the ISO14443) Commands
#define PCD_IDLE                0x00  // no action, cancels current command execution
#define PCD_MEM                 0x01  // stores 25 bytes into the internal buffer
#define PCD_GENRANDOMID         0x02  // generates a 10-byte random ID number
#define PCD_CALCCRC             0x03  // activates the CRC calculation or performs a self test
#define PCD_TRANSMIT            0x04  // Transmit data
#define PCD_NOCMDCHANGE         0x07  // no command change, can be used to modify the CommandReg register bits without affecting the command, for example, the PowerDown bit
#define PCD_RECEIVE             0x08  // activates the receiver circuits (receive data)
#define PCD_TRANSCEIVE          0x0C  // transmits data from FIFO buffer to antenna and automatically activates the receiver after transmission (transmit and receive data)
#define PCD_MFAUTHENT           0x0E  // performs the MIFARE standard authentication as a reader (authentication)
#define PCD_SOFTRESET           0x0F  // resets the MFRC522

// AddicoreRFID Proximity Integrated Circuit Card (PICC) Commands
#define PICC_REQIDL             0x26  // search the antenna area. PCD does not enter hibernation
#define PICC_REQALL             0x52  // find all the cards in antenna area
#define PICC_ANTICOLL           0x93  // anti-collision
#define PICC_SElECTTAG          0x93  // Select card
#define PICC_AUTHENT1A          0x60  // authentication with key A
#define PICC_AUTHENT1B          0x61  // authentication with key B
#define PICC_READ               0x30  // reads one memory block
#define PICC_WRITE              0xA0  // writes one memory block
#define PICC_DECREMENT          0xC0  // decrements the contents of a block and stores the result in the internal data register
#define PICC_INCREMENT          0xC1  // increments the contents of a block and stores the result in the internal data register
#define PICC_RESTORE            0xC2  // reads the contents of a block into the internal data register
#define PICC_TRANSFER           0xB0  // writes the contents of the internal data register to a block
#define PICC_HALT               0x50  // Sleep the card

// AddicoreRFID PICC Responses
#define MI_ACK                  0xA   // The 4-bit acknowledgement returned from a MIFARE Classic PICC
#define MI_ATQA                 0x04 //The 16-bit ATQA (Answer To Request acc. to ISO/IEC 14443-4) response from PICC
#define MI_SAK                  0x08  // The 8-bit SAK (Select Acknowledge, Type A) response from PICC

// AddicoreRFID Default Definable Pins
#define DEFAULT_CHIP_SELECT_PIN         10
#define DEFAULT_RESET_PIN               5

//AddicoreRFID error codes
#define MI_OK                   0
#define MI_NO_TAG_ERR           1
#define MI_ERR                  2

//Maximum length of the array
#define MAX_LEN 16


////////////////////////////////////////////////////////////////
// AddicoreRFID Registers                   
////////////////////////////////////////////////////////////////
/* The information below regarding the MFRC522 registers is from 
   the MFRC522 Manufacturer's Datasheet (Rev. 3.6 — 14 December 2011), Table 20
*/
//Page 0: Command and Status
#define     Reserved00          0x00  // reserved for future use
#define     CommandReg          0x01  // starts and stops command execution
#define     ComIEnReg           0x02  // enable and disable the passing of interrupt requests to IRq pin
#define     DivlEnReg           0x03  // enable and disable interrupt request control bits
#define     ComIrqReg           0x04  // interrupt request bits
#define     DivIrqReg           0x05  // interrupt request bits
#define     ErrorReg            0x06  // error bits showing the error status of the last command executed
#define     Status1Reg          0x07  // communication status bits
#define     Status2Reg          0x08  // receiver and transmitter status bits
#define     FIFODataReg         0x09  // input and output of 64 byte FIFO buffer
#define     FIFOLevelReg        0x0A  // number of bytes stored in the FIFO buffer
#define     WaterLevelReg       0x0B  // level for FIFO underflow and overflow warning
#define     ControlReg          0x0C  // miscellaneous control registers
#define     BitFramingReg       0x0D  // adjustments for bit-oriented frames
#define     CollReg             0x0E  // bit position of the first bit-collision detected on the RF interface
#define     Reserved01          0x0F  // reserved for future use
//Page 1: Command     
#define     Reserved10          0x10  // reserved for future use
#define     ModeReg             0x11  // defines general modes for transmitting and receiving
#define     TxModeReg           0x12  // defines transmission data rate and framing
#define     RxModeReg           0x13  // defines reception data rate and framing
#define     TxControlReg        0x14  // controls the logical behavior of the antenna driver pins TX1 and TX2
#define     TxAutoReg           0x15  // controls the setting of the transmission modulation
#define     TxSelReg            0x16  // selects the internal sources for the antenna driver
#define     RxSelReg            0x17  // selects internal receiver settings
#define     RxThresholdReg      0x18  // selects thresholds for the bit decoder
#define     DemodReg            0x19  // defines demodulator settings
#define     Reserved11          0x1A  // reserved for future use
#define     Reserved12          0x1B  // reserved for future use
#define     MfTxReg             0x1C  // controls some MIFARE communication transmit parameters
#define     MfRxReg             0x1D  // controls some MIFARE communication receive parameters
#define     Reserved13          0x1E  // reserved for future use
#define     SerialSpeedReg      0x1F  // selects the speed of the serial UART interface
//Page 2: Configuration  
#define     Reserved20          0x20  // reserved for future use
#define     CRCResultRegM       0x21  // shows the MSB values of the CRC calculation
#define     CRCResultRegL       0x22  // shows the LSB values of the CRC calculation
#define     Reserved21          0x23  // reserved for future use
#define     ModWidthReg         0x24  // controls the ModWidth setting
#define     Reserved22          0x25  // reserved for future use
#define     RFCfgReg            0x26  // configures the receiver gain
#define     GsNReg              0x27  // selects the conductance of the antenna driver pins TX1 and TX2 for modulation
#define     CWGsPReg            0x28  // defines the conductance of the p-driver output during periods of no modulation
#define     ModGsPReg           0x29  // defines the conductance of the p-driver output during periods of modulation
#define     TModeReg            0x2A  // defines settings for the internal timer
#define     TPrescalerReg       0x2B  // defines settings for the internal timer
#define     TReloadRegH         0x2C  // defines the higher 8 bits of the 16-bit timer reload value
#define     TReloadRegL         0x2D  // defines the lower 8 bits of the 16-bit timer reload value
#define     TCounterValueRegH   0x2E  // shows the higher 8 bits of the 16-bit timer value
#define     TCounterValueRegL   0x2F  // shows the lower 8 bits of the 16-bit timer value
//Page 3: Test Registers    
#define     Reserved30          0x30  // reserved for future use
#define     TestSel1Reg         0x31  // general test signal configuration
#define     TestSel2Reg         0x32  // general test signal configuration and PRBS control
#define     TestPinEnReg        0x33  // enables pin output driver on pins D1 to D7
#define     TestPinValueReg     0x34  // defines the values for D1 to D7 when it is used as an I/O bus
#define     TestBusReg          0x35  // shows the status of the internal test bus
#define     AutoTestReg         0x36  // controls the digital self test
#define     VersionReg          0x37  // shows the software version
#define     AnalogTestReg       0x38  // controls the pins AUX1 and AUX2
#define     TestDAC1Reg         0x39  // defines the test value for TestDAC1
#define     TestDAC2Reg         0x3A  // defines the test value for TestDAC2
#define     TestADCReg          0x3B  // shows the value of ADC I and Q channels
#define     Reserved31          0x3C  // reserved for production tests
#define     Reserved32          0x3D  // reserved for production tests
#define     Reserved33          0x3E  // reserved for production tests
#define     Reserved34          0x3F  // reserved for production tests

                               
class AddicoreRFID
{
private:
    byte _chipSelectPin;                                // Arduino pin connected to AddicoreRFID SDA pin (active low)
    byte _resetPin;                                     // Arduino pin connected to AddicoreRFID RST (reset) pin (active low)
    uint _RxBits;                                       // The number of received data bits
public:
    AddicoreRFID();
    AddicoreRFID( byte chipSelectPin, byte resetPin);
    void Setup_AddicoreRFID(void);
    void Advanced_Setup_AddicoreRFID(byte chipSelectPin, byte resetPin);
    void AddicoreRFID_Init(void);                       // Initialize the AddicoreRFID Module
    void Write_AddicoreRFID(byte addr, byte val);       // To a certain AddicoreRFID register to write a byte of data
    byte Read_AddicoreRFID(byte addr);              // From a certain AddicoreRFID read a byte of data register
    void SetBitMask(byte reg, byte mask);               // Set RC522 register bit
    void ClearBitMask(byte reg, byte mask);         // clear RC522 register bit
    void AntennaOn(void);                               // Open antennas, each time you start or shut down the natural barrier between the transmitter should be at least 1ms interval
    void AntennaOff(void);                              // Close antennas, each time you start or shut down the natural barrier between the transmitter should be at least 1ms interval
    void AddicoreRFID_Reset(void);                      // Soft reset of AddicoreRFID module
    byte AddicoreRFID_Request(byte reqMode, byte *TagType); // Find cards, read the card type number
    byte AddicoreRFID_ToCard(byte command, byte *sendData, byte sendLen, byte *backData, uint *backLen);    // RC522 and ISO14443 card communication
    byte AddicoreRFID_Anticoll(byte *serNum);           // Anti-collision detection, reading selected card serial number card
    void CalulateCRC(byte *pIndata, byte len, byte *pOutData);  // CRC calculation with MF522
    uint AddicoreRFID_SelectTag(byte *serNum);          // Selection card, read the card memory capacity
    byte AddicoreRFID_Auth(byte authMode, byte BlockAddr, byte *Sectorkey, byte *serNum);   // Verify card password
    byte AddicoreRFID_Read(byte blockAddr, byte *recvData); // Read block data
    byte AddicoreRFID_Write(byte blockAddr, byte *_writeData);  // Write block data
    void AddicoreRFID_Halt(void);                       // Command card into hibernation
    byte getNumRxBits(void);                                //Returns the number of received data bits
    byte serNum[];

};

#endif
