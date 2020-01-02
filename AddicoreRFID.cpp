// the sensor communicates using SPI, so include the library:
#include <Arduino.h> 
#include <SPI.h>

#include "AddicoreRFID.h"

//Maximum length of the array
#define MAX_LEN 16

/*
/////////////////////////////////////////////////////////////////////
//set the pin
/////////////////////////////////////////////////////////////////////
const int _chipSelectPin = 10;
const int _resetPin = 5;
*/

//4 bytes card serial number, the first 5 bytes for the checksum byte
//byte serNum[5];

//Maximum length of the array
#define MAX_LEN 16
byte str[MAX_LEN];


//-----------------------------------------------

byte  writeData[16]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100};  //Initialization 100 dollars
byte  moneyConsume = 18 ;  //Consumption of 18
byte  moneyAdd = 10 ;  //Recharge 10
//Sector A password, 16 sectors, each sector password 6Byte
 byte sectorKeyA[16][16] = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                             {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                             //{0x19, 0x84, 0x07, 0x15, 0x76, 0x14},
                             {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                            };
 byte sectorNewKeyA[16][16] = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                                {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff,0x07,0x80,0x69, 0x19,0x84,0x07,0x15,0x76,0x14},
                                 //you can set another ket , such as  " 0x19, 0x84, 0x07, 0x15, 0x76, 0x14 "
                                 //{0x19, 0x84, 0x07, 0x15, 0x76, 0x14, 0xff,0x07,0x80,0x69, 0x19,0x84,0x07,0x15,0x76,0x14},
                                 // but when loop, please set the  sectorKeyA, the same key, so that RFID module can read the card
                                {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xff,0x07,0x80,0x69, 0x19,0x33,0x07,0x15,0x34,0x14},
                               };



AddicoreRFID::AddicoreRFID()
{
	Advanced_Setup_AddicoreRFID(DEFAULT_CHIP_SELECT_PIN, DEFAULT_RESET_PIN);
}

AddicoreRFID::AddicoreRFID( byte chipSelectPin, byte resetPin)
{
	Advanced_Setup_AddicoreRFID(chipSelectPin, resetPin);
}

/*
 * Function Name: Setup_AddicoreRFID
 * Function Description: Performs necessary items to setup AddicoreRFID module
 * Input: None
 * Return value: None
*/
void AddicoreRFID::Setup_AddicoreRFID(void)
{	
	Advanced_Setup_AddicoreRFID(DEFAULT_CHIP_SELECT_PIN, DEFAULT_RESET_PIN);
}

/*
 * Function Name: Advanced_Setup_AddicoreRFID
 * Function Description: Performs necessary items to setup AddicoreRFID module with ability to define
 * Input Parameters: chipSelectPin - Arduino pin connected to the AddicoreRFID module's SDA pin
                     resetPin - Arduino pin connected to the AddicoreRFID module's RST pin
 * Return value: None
*/
void AddicoreRFID::Advanced_Setup_AddicoreRFID(byte chipSelectPin, byte resetPin)
{
	_chipSelectPin = chipSelectPin;
	pinMode(_chipSelectPin, OUTPUT);
	digitalWrite(_chipSelectPin, HIGH);  // Keeps AddicoreRFID module disabled for now
	
	_resetPin = resetPin;
	pinMode(_resetPin, OUTPUT);
	digitalWrite(_resetPin, LOW);        // keep AddicoreRFID module powered down for now
	
	SPI.begin();
}


/*
 * Function Name: AddicoreRFID_Init
 * Description: Initialize the AddicoreRFID module
 * Input: None
 * Return value: None
*/
void AddicoreRFID::AddicoreRFID_Init(void)
{
	digitalWrite(_chipSelectPin, LOW);  // Selects the SPI connected AddicoreRFID module
	if (digitalRead(_resetPin) == LOW) { //The AddicoreRFID is in power down mode
		digitalWrite(_resetPin, HIGH);	// Hard reset AddicoreRFID
		delay(50);                      // Allows for 37.74us oscillator start-up delay
	}
	else {
		AddicoreRFID_Reset();           // Soft reset the AddicoreRFID
	}
	 	
	//Timer: TPrescaler*TreloadVal/6.78MHz = 24ms
    Write_AddicoreRFID(TModeReg, 0x8D);		//Tauto=1; f(Timer) = 6.78MHz/TPreScaler
    Write_AddicoreRFID(TPrescalerReg, 0x3E);	//TModeReg[3..0] + TPrescalerReg
    Write_AddicoreRFID(TReloadRegL, 30);           
    Write_AddicoreRFID(TReloadRegH, 0);
	
	Write_AddicoreRFID(TxAutoReg, 0x40);		//100%ASK
	Write_AddicoreRFID(ModeReg, 0x3D);		//CRC Initial value 0x6363	???

	//ClearBitMask(Status2Reg, 0x08);		//MFCrypto1On=0
	//Write_AddicoreRFID(RxSelReg, 0x86);		//RxWait = RxSelReg[5..0]
	//Write_AddicoreRFID(RFCfgReg, 0x7F);   		//RxGain = 48dB

	AntennaOn();		//Open the antenna
}


/*
 * Function Name: Write_AddicoreRFID
 * Function Description: To a certain AddicoreRFID register to write a byte of data
 * Input Parameters: addr - register address; val - the value to be written
 * Return value: None
 */
void AddicoreRFID::Write_AddicoreRFID(byte addr, byte val)
{
	digitalWrite(_chipSelectPin, LOW);  // Selects the SPI connected AddicoreRFID module

	//Address Format: 0XXXXXX0, the left most "0" indicates a write
	SPI.transfer((addr<<1)&0x7E);	
	SPI.transfer(val);
	
	digitalWrite(_chipSelectPin, HIGH);
}


/*
 * Function Name: Read_AddicoreRFID
 * Description: From a certain AddicoreRFID read a byte of data register
 * Input Parameters: addr - register address
 * Returns: a byte of data read from the
 */
byte AddicoreRFID::Read_AddicoreRFID(byte addr)
{
	byte val;

	digitalWrite(_chipSelectPin, LOW);

	//Address Format: 1XXXXXX0, the first "1" indicates a read
	SPI.transfer(((addr<<1)&0x7E) | 0x80);	
	val =SPI.transfer(0x00);
	
	digitalWrite(_chipSelectPin, HIGH);
	
	return val;	
}

/*
 * Function Name: SetBitMask
 * Description: Set RC522 register bit
 * Input parameters: reg - register address; mask - set value
 * Return value: None
 */
void AddicoreRFID::SetBitMask(byte reg, byte mask)  
{
    byte tmp;
    tmp = Read_AddicoreRFID(reg);
    Write_AddicoreRFID(reg, tmp | mask);  // set bit mask
}


/*
 * Function Name: ClearBitMask
 * Description: clear RC522 register bit
 * Input parameters: reg - register address; mask - clear bit value
 * Return value: None
*/
void AddicoreRFID::ClearBitMask(byte reg, byte mask)  
{
    byte tmp;
    tmp = Read_AddicoreRFID(reg);
    Write_AddicoreRFID(reg, tmp & (~mask));  // clear bit mask
} 


/*
 * Function Name: AntennaOn
 * Description: Open antennas, each time you start or shut down the natural barrier between the transmitter should be at least 1ms interval
 * Input: None
 * Return value: None
 */
void AddicoreRFID::AntennaOn(void)
{
	byte temp;

	temp = Read_AddicoreRFID(TxControlReg);
	if (!(temp & 0x03))
	{
		SetBitMask(TxControlReg, 0x03);
	}
}


/*
  * Function Name: AntennaOff
  * Description: Close antennas, each time you start or shut down the natural barrier between the transmitter should be at least 1ms interval
  * Input: None
  * Return value: None
 */
void AddicoreRFID::AntennaOff(void)
{
	ClearBitMask(TxControlReg, 0x03);
}


/*
 * Function Name: AddicoreRFID_Reset
 * Description: Perform soft reset of AddicoreRFID Module
 * Input: None
 * Return value: None
 */
void AddicoreRFID::AddicoreRFID_Reset(void)
{
    Write_AddicoreRFID(CommandReg, PCD_SOFTRESET);
}


/*
 * Function Name: AddicoreRFID_Request
 * Description: Find cards, read the card type number
 * Input parameters: reqMode - find cards way
 *			 TagType - Return Card Type
 *			 	0x4400 = Mifare_UltraLight
 *				0x0400 = Mifare_One(S50)
 *				0x0200 = Mifare_One(S70)
 *				0x0800 = Mifare_Pro(X)
 *				0x4403 = Mifare_DESFire
 * Return value: the successful return MI_OK
 */
byte AddicoreRFID::AddicoreRFID_Request(byte reqMode, byte *TagType)
{
	byte status;  
	//uint backBits;			//The received data bits

	Write_AddicoreRFID(BitFramingReg, 0x07);		//TxLastBists = BitFramingReg[2..0]	???
	
	TagType[0] = reqMode;
	status = AddicoreRFID_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &_RxBits);
	
	if ((status != MI_OK) || (_RxBits != 0x10))
	{    
		status = MI_ERR;
	}
   
	return status;
}


/*
 * Function Name: AddicoreRFID_ToCard
 * Description: RC522 and ISO14443 card communication
 * Input Parameters: command - MF522 command word,
 *			 sendData--RC522 sent to the card by the data
 *			 sendLen--Length of data sent	 
 *			 backData--Data returned from the card
 *			 backLen--Returned data bit length
 * Return value: the successful return MI_OK
 */
byte AddicoreRFID::AddicoreRFID_ToCard(byte command, byte *sendData, byte sendLen, byte *backData, uint *backLen)
{
    byte status = MI_ERR;
    byte irqEn = 0x00;
    byte waitIRq = 0x00;
    byte lastBits;
    byte n;
    uint i;

    switch (command)
    {
        case PCD_MFAUTHENT:		//Certification cards close
		{
			irqEn = 0x12;
			waitIRq = 0x10;
			break;
		}
		case PCD_TRANSCEIVE:	//Transmit FIFO data
		{
			irqEn = 0x77;
			waitIRq = 0x30;
			break;
		}
		default:
			break;
    }
   
    Write_AddicoreRFID(ComIrqReg, irqEn|0x80);	//Interrupt request
    ClearBitMask(ComIrqReg, 0x80);			//Clear all interrupt request bit
    SetBitMask(FIFOLevelReg, 0x80);			//FlushBuffer=1, FIFO Initialization
    
	Write_AddicoreRFID(CommandReg, PCD_IDLE);	//NO action; Cancel the current command???

	//Writing data to the FIFO
    for (i=0; i<sendLen; i++)
    {   
		Write_AddicoreRFID(FIFODataReg, sendData[i]);    
	}

	//Execute the command
	Write_AddicoreRFID(CommandReg, command);
    if (command == PCD_TRANSCEIVE)
    {    
		SetBitMask(BitFramingReg, 0x80);		//StartSend=1,transmission of data starts  
	}   
    
	//Waiting to receive data to complete
	i = 2000;	//i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms???
    do 
    {
		//ComIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        n = Read_AddicoreRFID(ComIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitIRq));

    ClearBitMask(BitFramingReg, 0x80);			//StartSend=0
	
    if (i != 0)
    {    
        if(!(Read_AddicoreRFID(ErrorReg) & 0x1B))	//BufferOvfl Collerr CRCErr ProtecolErr
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {   
				status = MI_NO_TAG_ERR;			//??   
			}

            if (command == PCD_TRANSCEIVE)
            {
               	n = Read_AddicoreRFID(FIFOLevelReg);
              	lastBits = Read_AddicoreRFID(ControlReg) & 0x07;
                if (lastBits)
                {   
					*backLen = (n-1)*8 + lastBits;   
				}
                else
                {   
					*backLen = n*8;   
				}

                if (n == 0)
                {   
					n = 1;    
				}
                if (n > MAX_LEN)
                {   
					n = MAX_LEN;   
				}
				
				//Reading the received data in FIFO
                for (i=0; i<n; i++)
                {   
					backData[i] = Read_AddicoreRFID(FIFODataReg);    
				}
            }
        }
        else
        {   
			status = MI_ERR;  
		}
        
    }
	
    //SetBitMask(ControlReg,0x80);           //timer stops
    //Write_AddicoreRFID(CommandReg, PCD_IDLE); 

    return status;
}


/*
 * Function Name: AddicoreRFID_Anticoll
 * Description: Anti-collision detection, reading selected card serial number card
 * Input parameters: serNum - returns 4 bytes card serial number, the first 5 bytes for the checksum byte
 * Return value: the successful return MI_OK
 */
byte AddicoreRFID::AddicoreRFID_Anticoll(byte *serNum)
{
    byte status;
    byte i;
	byte serNumCheck=0;
    //uint unLen;

    //ClearBitMask(Status2Reg, 0x08);		//TempSensclear
    //ClearBitMask(CollReg,0x80);			//ValuesAfterColl
	Write_AddicoreRFID(BitFramingReg, 0x00);		//TxLastBists = BitFramingReg[2..0]
 
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = AddicoreRFID_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &_RxBits);

    if (status == MI_OK)
	{
		//Check card serial number
		for (i=0; i<4; i++)
		{   
		 	serNumCheck ^= serNum[i];
		}
		if (serNumCheck != serNum[i])
		{   
			status = MI_ERR;    
		}
    }

    //SetBitMask(CollReg, 0x80);		//ValuesAfterColl=1

	//memcpy(serNum, str, 5);
    return status;
} 


/*
 * Function Name: CalulateCRC
 * Description: CRC calculation with MF522
 * Input parameters: pIndata - To read the CRC data, len - the data length, pOutData - CRC calculation results
 * Return value: None
 */
void AddicoreRFID::CalulateCRC(byte *pIndata, byte len, byte *pOutData)
{
    byte i, n;

    ClearBitMask(DivIrqReg, 0x04);			//CRCIrq = 0
    SetBitMask(FIFOLevelReg, 0x80);			//Clear the FIFO pointer
    //Write_AddicoreRFID(CommandReg, PCD_IDLE);

	//Writing data to the FIFO	
    for (i=0; i<len; i++)
    {   
		Write_AddicoreRFID(FIFODataReg, *(pIndata+i));   
	}
    Write_AddicoreRFID(CommandReg, PCD_CALCCRC);

	//Wait CRC calculation is complete
    i = 0xFF;
    do 
    {
        n = Read_AddicoreRFID(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));			//CRCIrq = 1

	//Read CRC calculation result
    pOutData[0] = Read_AddicoreRFID(CRCResultRegL);
    pOutData[1] = Read_AddicoreRFID(CRCResultRegM);
}


/*
 * Function Name: AddicoreRFID_SelectTag
 * Description: Selection card, read the card memory capacity
 * Input parameters: serNum - Incoming card serial number
 * Return value: the successful return of card capacity
 */
uint AddicoreRFID::AddicoreRFID_SelectTag(byte *serNum)
{
    byte i;
	byte status;
	byte size;
    uint recvBits;
    byte buffer[9]; 

	//ClearBitMask(Status2Reg, 0x08);			//MFCrypto1On=0

    buffer[0] = PICC_SElECTTAG;
    buffer[1] = 0x70;
    for (i=0; i<5; i++)
    {
    	buffer[i+2] = *(serNum+i);
    }
	CalulateCRC(buffer, 7, &buffer[7]);		//??
    status = AddicoreRFID_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
    
    if ((status == MI_OK) && (recvBits == 0x18))
    {   
		size = buffer[0]; 
	}
    else
    {   
		size = 0;    
	}

    return size;
}


/*
 * Function Name: AddicoreRFID_Auth
 * Description: Verify card password
 * Input parameters: authMode - Password Authentication Mode
                 0x60 = A key authentication
                 0x61 = Authentication Key B
             BlockAddr--Block address
             Sectorkey--Sector password
             serNum--Card serial number, 4-byte
 * Return value: the successful return MI_OK
 */
byte AddicoreRFID::AddicoreRFID_Auth(byte authMode, byte BlockAddr, byte *Sectorkey, byte *serNum)
{
    byte status;
    uint recvBits;
    byte i;
	byte buff[12]; 

	//Verify the command block address + sector + password + card serial number
    buff[0] = authMode;
    buff[1] = BlockAddr;
    for (i=0; i<6; i++)
    {    
		buff[i+2] = *(Sectorkey+i);   
	}
    for (i=0; i<4; i++)
    {    
		buff[i+8] = *(serNum+i);   
	}
    status = AddicoreRFID_ToCard(PCD_MFAUTHENT, buff, 12, buff, &recvBits);

    if ((status != MI_OK) || (!(Read_AddicoreRFID(Status2Reg) & 0x08)))
    {   
		status = MI_ERR;   
	}
    
    return status;
}


/*
 * Function Name: AddicoreRFID_Read
 * Description: Read block data
 * Input parameters: blockAddr - block address; recvData - read block data
 * Return value: the successful return MI_OK
 */
byte AddicoreRFID::AddicoreRFID_Read(byte blockAddr, byte *recvData)
{
    byte status;
    uint unLen;

    recvData[0] = PICC_READ;
    recvData[1] = blockAddr;
    CalulateCRC(recvData,2, &recvData[2]);
    status = AddicoreRFID_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

    if ((status != MI_OK) || (unLen != 0x90))
    {
        status = MI_ERR;
    }
    
    return status;
}


/*
 * Function Name: AddicoreRFID_Write
 * Description: Write block data
 * Input parameters: blockAddr - block address; writeData - to 16-byte data block write
 * Return value: the successful return MI_OK
 */
byte AddicoreRFID::AddicoreRFID_Write(byte blockAddr, byte *_writeData)
{
    byte status;
    uint recvBits;
    byte i;
	byte buff[18]; 
    
    buff[0] = PICC_WRITE;
    buff[1] = blockAddr;
    CalulateCRC(buff, 2, &buff[2]);
    status = AddicoreRFID_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

    if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
    {   
		status = MI_ERR;   
	}
        
    if (status == MI_OK)
    {
        for (i=0; i<16; i++)		//Data to the FIFO write 16Byte
        {    
        	buff[i] = *(_writeData+i);   
        }
        CalulateCRC(buff, 16, &buff[16]);
        status = AddicoreRFID_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);
        
		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
        {   
			status = MI_ERR;   
		}
    }
    
    return status;
}


/*
 * Function Name: AddicoreRFID_Halt
 * Description: Command card into hibernation
 * Input: None
 * Return value: None
 */
void AddicoreRFID::AddicoreRFID_Halt(void)
{
	byte status;
    uint unLen;
    byte buff[4]; 

    buff[0] = PICC_HALT;
    buff[1] = 0;
    CalulateCRC(buff, 2, &buff[2]);
 
    status = AddicoreRFID_ToCard(PCD_TRANSCEIVE, buff, 4, buff,&unLen);
}

byte AddicoreRFID::getNumRxBits(void)
{
	return _RxBits;
}