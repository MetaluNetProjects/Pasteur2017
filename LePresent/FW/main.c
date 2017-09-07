/*********************************************************************
 *            firmware for "le Present"
 * 4 PWM controlled power LEDs + 1 PWM one-way motor    
 *********************************************************************/

#define BOARD Versa1

#include <fruit.h>
#include <analog.h>
#include <ramp.h>

t_delay mainDelay;
t_delay randDelay;
t_ramp motorRamp, LEDramps[4];
unsigned char i;

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 10000); 	// init the mainDelay to 10 ms
//-----------------
	analogWrite(LED1,0);
	analogWrite(LED2,0);
	analogWrite(LED3,0);
	analogWrite(LED4,0);
	pinModeAnalogOut(LED1);
	pinModeAnalogOut(LED2);
	pinModeAnalogOut(LED3);
	pinModeAnalogOut(LED4);

	analogWrite(MOTORPWM,0);
	pinModeDigitalOut(MA1);
	pinModeDigitalOut(MA2);
	pinModeDigitalOut(MAEN);
	pinModeDigitalOut(MAEN2);
	pinModeAnalogOut(MOTORPWM);
	digitalSet(MAEN);
	digitalSet(MAEN2);
	digitalClear(MA1);
	digitalSet(MA2);
	
	analogInit();
	analogSelect(0, MOTA_CURRENT);
	analogSelect(1, POT1);
	analogSelect(2, POT2);
	
	rampInit(&motorRamp);
	for(i = 0; i < 4; i++) rampInit(&LEDramps[i]);

	EEreadMain();
}
// ---------------------------------
int squ(int i)
{
	return ((long)i * i) / 1024;
}
// ---------- Main loop ------------
void loop() {
	static int count = 499;
	static long delTime = 10000;
	static int motSpeedOffset = 1024;
	// min:3V max:12V ; 12VDC power supply
#define DCVOLTAGE  12
#define motMinSpeed  ((3 * 1024) / DCVOLTAGE)
#define motMaxSpeed  ((12 * 1024) / DCVOLTAGE)
	
	fraiseService();	// listen to Fraise events
	analogService();
	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, delTime); 	// re-init mainDelay
		analogSend();		// send analog channels that changed
		count++;
		if(count > 500) // reset every 5 secs
		{
			count = 0;
			rampGoto(&motorRamp, motMinSpeed + (squ(rand()%1024) * (long)motSpeedOffset)/1024);
			for(i = 0; i < 4; i++) rampGoto(&LEDramps[i], squ(rand()%1024));
		}

		rampCompute(&motorRamp);
		analogWrite(MOTORPWM,rampGetPos(&motorRamp));
		
		for(i = 0; i < 4; i++) rampCompute(&LEDramps[i]);
		analogWrite(LED1,rampGetPos(&LEDramps[0]));
		analogWrite(LED2,rampGetPos(&LEDramps[1]));
		analogWrite(LED3,rampGetPos(&LEDramps[2]));
		analogWrite(LED4,rampGetPos(&LEDramps[3]));			 

		printf("CR %d %d %d %d %d\n", (int)rampGetPos(&motorRamp), 
			(int)rampGetPos(&LEDramps[0]), (int)rampGetPos(&LEDramps[1]), 
			(int)rampGetPos(&LEDramps[2]), (int)rampGetPos(&LEDramps[3])
		);
		
		delTime = (8192 - analogGet(1)) * (200000/8192) + 10000; // (10ms -> 210ms)
		motSpeedOffset = ((long)analogGet(2) * (motMaxSpeed - motMinSpeed)) / 8192;
	}
}

// Receiving

void fraiseReceive() // receive raw bytes
{
	unsigned char c=fraiseGetChar();
	unsigned int i;
	/*if(c==50) softpwmReceive(); // if first byte is 50, then call softpwm receive function.
	else*/ if(c==51) {
		i = fraiseGetInt();
		analogWrite(LED1,i);
	}
	else if(c==52) {
		i = fraiseGetInt();
		analogWrite(LED2,i);
	}
	else if(c==53) {
		i = fraiseGetInt();
		analogWrite(LED3,i);
	}
	else if(c==54) {
		i = fraiseGetInt();
		analogWrite(LED4,i);
	}
	else if(c==60) {
		i = fraiseGetInt();
		analogWrite(MOTORPWM,i);
	}
	else if(c==100) rampInput(&motorRamp);
	else if(c==101) rampInput(&LEDramps[0]);
	else if(c==102) rampInput(&LEDramps[1]);
	else if(c==103) rampInput(&LEDramps[2]);
	else if(c==104) rampInput(&LEDramps[3]);
}

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
	}
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
	else if((c=='S') && (fraiseGetChar()=='A') && (fraiseGetChar()=='V') && (fraiseGetChar()=='E'))
	    EEwriteMain();
}

void EEdeclareMain()
{
	rampDeclareEE(&motorRamp);
	rampDeclareEE(&LEDramps[0]);
	rampDeclareEE(&LEDramps[1]);
	rampDeclareEE(&LEDramps[2]);
	rampDeclareEE(&LEDramps[3]);
}	
