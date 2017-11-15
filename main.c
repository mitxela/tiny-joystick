/* Mini MIDI Pitchbend Joystick
 * mitxela.com/projects/tiny_joystick
 */

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>

#include "usbdrv.h"

#ifndef NULL
#define NULL	((void *)0)
#endif


/* ------------------------------------------------------------------------- */

const PROGMEM char deviceDescrMIDI[] = {	/* USB device descriptor */
	18,			/* sizeof(usbDescriptorDevice): length of descriptor in bytes */
	USBDESCR_DEVICE,	/* descriptor type */
	0x10, 0x01,		/* USB version supported */
	0,			/* device class: defined at interface level */
	0,			/* subclass */
	0,			/* protocol */
	8,			/* max packet size */
	USB_CFG_VENDOR_ID,	/* 2 bytes */
	USB_CFG_DEVICE_ID,	/* 2 bytes */
	USB_CFG_DEVICE_VERSION,	/* 2 bytes */
	1,			/* manufacturer string index */
	2,			/* product string index */
	0,			/* serial number string index */
	1,			/* number of configurations */
};

// B.2 Configuration Descriptor
const PROGMEM char configDescrMIDI[] = {	/* USB configuration descriptor */
	9,			/* sizeof(usbDescrConfig): length of descriptor in bytes */
	USBDESCR_CONFIG,	/* descriptor type */
	101, 0,			/* total length of data returned (including inlined descriptors) */
	2,			/* number of interfaces in this configuration */
	1,			/* index of this configuration */
	0,			/* configuration name string index */
	0,
	//USBATTR_BUSPOWER,
	USB_CFG_MAX_BUS_POWER / 2,	/* max USB current in 2mA units */

	// B.3 AudioControl Interface Descriptors
	// The AudioControl interface describes the device structure (audio function topology)
	// and is used to manipulate the Audio Controls. This device has no audio function
	// incorporated. However, the AudioControl interface is mandatory and therefore both
	// the standard AC interface descriptor and the classspecific AC interface descriptor
	// must be present. The class-specific AC interface descriptor only contains the header
	// descriptor.

	// B.3.1 Standard AC Interface Descriptor
	// The AudioControl interface has no dedicated endpoints associated with it. It uses the
	// default pipe (endpoint 0) for all communication purposes. Class-specific AudioControl
	// Requests are sent using the default pipe. There is no Status Interrupt endpoint provided.
	/* AC interface descriptor follows inline: */
	9,			/* sizeof(usbDescrInterface): length of descriptor in bytes */
	USBDESCR_INTERFACE,	/* descriptor type */
	0,			/* index of this interface */
	0,			/* alternate setting for this interface */
	0,			/* endpoints excl 0: number of endpoint descriptors to follow */
	1,			/* */
	1,			/* */
	0,			/* */
	0,			/* string index for interface */

	// B.3.2 Class-specific AC Interface Descriptor
	// The Class-specific AC interface descriptor is always headed by a Header descriptor
	// that contains general information about the AudioControl interface. It contains all
	// the pointers needed to describe the Audio Interface Collection, associated with the
	// described audio function. Only the Header descriptor is present in this device
	// because it does not contain any audio functionality as such.
	/* AC Class-Specific descriptor */
	9,			/* sizeof(usbDescrCDC_HeaderFn): length of descriptor in bytes */
	36,			/* descriptor type */
	1,			/* header functional descriptor */
	0x0, 0x01,		/* bcdADC */
	9, 0,			/* wTotalLength */
	1,			/* */
	1,			/* */

	// B.4 MIDIStreaming Interface Descriptors

	// B.4.1 Standard MS Interface Descriptor
	/* interface descriptor follows inline: */
	9,			/* length of descriptor in bytes */
	USBDESCR_INTERFACE,	/* descriptor type */
	1,			/* index of this interface */
	0,			/* alternate setting for this interface */
	2,			/* endpoints excl 0: number of endpoint descriptors to follow */
	1,			/* AUDIO */
	3,			/* MS */
	0,			/* unused */
	0,			/* string index for interface */

	// B.4.2 Class-specific MS Interface Descriptor
	/* MS Class-Specific descriptor */
	7,			/* length of descriptor in bytes */
	36,			/* descriptor type */
	1,			/* header functional descriptor */
	0x0, 0x01,		/* bcdADC */
	65, 0,			/* wTotalLength */

	// B.4.3 MIDI IN Jack Descriptor
	6,			/* bLength */
	36,			/* descriptor type */
	2,			/* MIDI_IN_JACK desc subtype */
	1,			/* EMBEDDED bJackType */
	1,			/* bJackID */
	0,			/* iJack */

	6,			/* bLength */
	36,			/* descriptor type */
	2,			/* MIDI_IN_JACK desc subtype */
	2,			/* EXTERNAL bJackType */
	2,			/* bJackID */
	0,			/* iJack */

	//B.4.4 MIDI OUT Jack Descriptor
	9,			/* length of descriptor in bytes */
	36,			/* descriptor type */
	3,			/* MIDI_OUT_JACK descriptor */
	1,			/* EMBEDDED bJackType */
	3,			/* bJackID */
	1,			/* No of input pins */
	2,			/* BaSourceID */
	1,			/* BaSourcePin */
	0,			/* iJack */

	9,			/* bLength of descriptor in bytes */
	36,			/* bDescriptorType */
	3,			/* MIDI_OUT_JACK bDescriptorSubtype */
	2,			/* EXTERNAL bJackType */
	4,			/* bJackID */
	1,			/* bNrInputPins */
	1,			/* baSourceID (0) */
	1,			/* baSourcePin (0) */
	0,			/* iJack */


	// B.5 Bulk OUT Endpoint Descriptors

	//B.5.1 Standard Bulk OUT Endpoint Descriptor
	9,			/* bLenght */
	USBDESCR_ENDPOINT,	/* bDescriptorType = endpoint */
	0x1,			/* bEndpointAddress OUT endpoint number 1 */
	3,			/* bmAttributes: 2:Bulk, 3:Interrupt endpoint */
	8, 0,			/* wMaxPacketSize */
	10,			/* bIntervall in ms */
	0,			/* bRefresh */
	0,			/* bSyncAddress */

	// B.5.2 Class-specific MS Bulk OUT Endpoint Descriptor
	5,			/* bLength of descriptor in bytes */
	37,			/* bDescriptorType */
	1,			/* bDescriptorSubtype */
	1,			/* bNumEmbMIDIJack  */
	1,			/* baAssocJackID (0) */


	//B.6 Bulk IN Endpoint Descriptors

	//B.6.1 Standard Bulk IN Endpoint Descriptor
	9,			/* bLenght */
	USBDESCR_ENDPOINT,	/* bDescriptorType = endpoint */
	0x81,			/* bEndpointAddress IN endpoint number 1 */
	3,			/* bmAttributes: 2: Bulk, 3: Interrupt endpoint */
	8, 0,			/* wMaxPacketSize */
	10,			/* bIntervall in ms */
	0,			/* bRefresh */
	0,			/* bSyncAddress */

	// B.6.2 Class-specific MS Bulk IN Endpoint Descriptor
	5,			/* bLength of descriptor in bytes */
	37,			/* bDescriptorType */
	1,			/* bDescriptorSubtype */
	1,			/* bNumEmbMIDIJack (0) */
	3,			/* baAssocJackID (0) */
};


uchar usbFunctionDescriptor(usbRequest_t * rq)
{
	if (rq->wValue.bytes[1] == USBDESCR_DEVICE) {
		usbMsgPtr = (uchar *) deviceDescrMIDI;
		return sizeof(deviceDescrMIDI);
	}
	else {		/* must be config descriptor */
		usbMsgPtr = (uchar *) configDescrMIDI;
		return sizeof(configDescrMIDI);
	}
}


/* ------------------------------------------------------------------------- */
/* ------------------------ interface to USB driver ------------------------ */
/* ------------------------------------------------------------------------- */

uchar usbFunctionSetup(uchar data[8]) {
	return 0xff;
}

uchar usbFunctionRead(uchar * data, uchar len) {	
	data[0] = 0;
	data[1] = 0;
	data[2] = 0;
	data[3] = 0;
	data[4] = 0;
	data[5] = 0;
	data[6] = 0;

	return 7;
}

uchar usbFunctionWrite(uchar * data, uchar len) {
	return 1;
}


// Oscillator Calibration
// Taken directly from EasyLogger: 
// https://www.obdev.at/products/vusb/easylogger.html

static void calibrateOscillator(void) {
	uchar	step = 128;
	uchar	trialValue = 0, optimumValue;
	int		x, optimumDev, targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);

	/* do a binary search: */
	do{
		OSCCAL = trialValue + step;
		x = usbMeasureFrameLength();	/* proportional to current real frequency */
		if(x < targetValue)				/* frequency still too low */
			trialValue += step;
		step >>= 1;
	}while(step > 0);
	/* We have a precision of +/- 1 for optimum OSCCAL here */
	/* now do a neighborhood search for optimum value */
	optimumValue = trialValue;
	optimumDev = x; /* this is certainly far away from optimum */
	for(OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++){
		x = usbMeasureFrameLength() - targetValue;
		if(x < 0)
			x = -x;
		if(x < optimumDev){
			optimumDev = x;
			optimumValue = OSCCAL;
		}
	}
	OSCCAL = optimumValue;
}

void usbEventResetReady(void) {
	/* Disable interrupts during oscillator calibration since
	 * usbMeasureFrameLength() counts CPU cycles.
	 */
	cli();
	calibrateOscillator();
	sei();
	eeprom_write_byte(0, OSCCAL);   /* store the calibrated value in EEPROM */
}






/*
  SysEx -> EEPROM stuff
  USB-MIDI messages arrive at the function usbFunctionWriteOut() and may be 4 or 8 bytes.
  The code index number (low nibble of the first byte) determintes if the sysex ends with
  that message or if there is more data expected.
*/

#define eeFloatAddr 4

uchar mode = 0; // Currently only used to enable calibration mode

struct {
  float up;
  float down;
  float left;
  float right;
  int16_t voltage[4];
  int16_t limit[4];
} bendAmount;

/*
//  These are example numbers that you might expect in the struct.
//  This function was used for debugging the eeprom stuff. 

void writeData(void){

    bendAmount.up = -34.13333333333333;
    bendAmount.down = -22.755555555555553;
    bendAmount.left = -9.416091954022988;
    bendAmount.right = 19.504761904761903;

    bendAmount.voltage[0] = 478;
    bendAmount.voltage[1] = 577;
    bendAmount.voltage[2] = 484;
    bendAmount.voltage[3] = 542;

    bendAmount.limit[0]=-8192;
    bendAmount.limit[1]= 5461;
    bendAmount.limit[2]=-2731;
    bendAmount.limit[3]=-5461;

    eeprom_write_block(&bendAmount, eeFloatAddr, sizeof(bendAmount)); 
}
*/

void usbMidiMessageIn(uchar * data){

  static uchar i = 0;

  if (i==0) { //Sysex not started
    if ( data[0]==0x04 //code index = sysex start/continue 
      && data[1]==0xF0 // sysex begin
      && data[2]==0x12) //magic number...
      {
        if ( data[3]==0x34) i=eeFloatAddr;
        else if ( data[3]==0x35 ) {if (0==mode) mode=2; else mode=0;}
      }
  } else { //sysex ongoing
    if (data[0]==0x04 || data[0]==0x05 || data[0]==0x06 || data[0]==0x07) { //all sysex code index numbers

      if (data[1] & 1) data[2] |=0x80;
      eeprom_write_byte(i++, data[2]);
      if (data[1] & 2) data[3] |=0x80;
      eeprom_write_byte(i++, data[3]);

      if (data[0]!=0x04) { //end of sysex
        i=0; 
        eeprom_read_block(&bendAmount, eeFloatAddr, sizeof(bendAmount)); 
      }
    }
  }
}

void usbFunctionWriteOut(uchar * data, uchar len) {

  usbMidiMessageIn(data);
  if (len==8) usbMidiMessageIn(data+4);

}






//////// Main ////////////

int main(void) {
	uchar i;
	uchar calibrationValue;
	uchar	midiMsg[8];
	int16_t	val1=0;
	int16_t	val2=0;
	int16_t	lastVal1=0;
	int16_t	lastVal2=0;
  int16_t lastbend=8192;

  eeprom_read_block(&bendAmount, eeFloatAddr, sizeof(bendAmount)); 
	
	calibrationValue = eeprom_read_byte(0); /* calibration value from last time */
	if(calibrationValue != 0xff){
		OSCCAL = calibrationValue;
	}
	usbDeviceDisconnect();
	for(i=0;i<20;i++){  /* 300 ms disconnect */
		_delay_ms(15);
	}
	usbDeviceConnect();
	
	wdt_enable(WDTO_1S);

	usbInit();
	sei();
	

	
	for(;;){	/* main event loop */
		
		wdt_reset();
		usbPoll();
    if(usbInterruptIsReady()){


      ADMUX = (1<<MUX1 | 1<<MUX0); //PB3
      ADCSRA = (1<<ADEN|1<<ADSC|1<<ADIF|1<<ADPS2|1<<ADPS1);
      while (!(ADCSRA & (1<<ADIF)));
      val1 = ADCW;

      ADMUX = (1<<MUX1); //PB4
      ADCSRA = (1<<ADEN|1<<ADSC|1<<ADIF|1<<ADPS2|1<<ADPS1);
      while (!(ADCSRA & (1<<ADIF)));
      val2 = ADCW;

      if (abs(val1-lastVal1)>1 || abs(val2-lastVal2)>1 ){
        i=0;
        lastVal1=val1;
        lastVal2=val2;

        if (mode==0) { // 4-way pitch bend

          int16_t bend1=0;
          int16_t bend2=0;
          int16_t bend=0;


          if (val1 <=bendAmount.voltage[0] ) {
            bend1 = (bendAmount.voltage[0]-val1)  *bendAmount.up; // -3 semitones

            if (bendAmount.limit[0]<0)
              {if (bend1 < bendAmount.limit[0]) bend1=bendAmount.limit[0];}
            else
              {if (bend1 > bendAmount.limit[0]) bend1=bendAmount.limit[0];}

          } else if (val1>=bendAmount.voltage[1]) {
            bend1 = (bendAmount.voltage[1]-val1) *bendAmount.down; //2 semitones up

            if (bendAmount.limit[1]<0)
              {if (bend1 < bendAmount.limit[1]) bend1=bendAmount.limit[1];}
            else
              {if (bend1 > bendAmount.limit[1]) bend1=bendAmount.limit[1];}

          } else bend1=0;

          if (val2 <= bendAmount.voltage[2]) {
            bend2 = (bendAmount.voltage[2]-val2) *bendAmount.left; //semitone down

            if (bendAmount.limit[2]<0)
              {if (bend2 < bendAmount.limit[2]) bend2=bendAmount.limit[2];}
            else
              {if (bend2 > bendAmount.limit[2]) bend2=bendAmount.limit[2];}

          }else if (val2>=bendAmount.voltage[3]) {
              bend2 = (bendAmount.voltage[3]-val2) *bendAmount.right; // two down

            if (bendAmount.limit[3]<0)
              {if (bend2 < bendAmount.limit[3]) bend2=bendAmount.limit[3];}
            else
              {if (bend2 > bendAmount.limit[3]) bend2=bendAmount.limit[3];}

          } else bend2=0;

          bend = 8192 + (bend1 + bend2);
          if (bend>16384) bend=16384;
          else if (bend <0) bend=0; 



          if (lastbend!=bend) {

            midiMsg[i++]= 0x0E; // Cable Number, Code Index Number
            midiMsg[i++]= 0xE0;
            midiMsg[i++]= (bend)&0x7F;
            midiMsg[i++]= (bend>>7)&0x7F;



            usbSetInterrupt(midiMsg, i);
            lastbend=bend;
          }

        } else if (mode==2) { //calibration mode


            midiMsg[i++]= 0x0E; 
            midiMsg[i++]= 0xE1;
            midiMsg[i++]= (val1)&0x7F;
            midiMsg[i++]= (val1>>7)&0x7F;


            midiMsg[i++]= 0x0E; 
            midiMsg[i++]= 0xE2;
            midiMsg[i++]= (val2)&0x7F;
            midiMsg[i++]= (val2>>7)&0x7F;

            usbSetInterrupt(midiMsg, i);
            lastVal1 = 1025; // force send new data

        }
      }


    }

	}
	return 0;
}
