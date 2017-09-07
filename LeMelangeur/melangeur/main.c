/*********************************************************************
 *               switch example for Versa1.0
 *	Switch capture on connectors K1, K2, K3 and K4. 
 *********************************************************************/

#define BOARD 8X2A

#include <fruit.h>
#include <switch.h>
#include <analog.h>
#include <dcmotor.h>
#include <ramp.h>

#define SECONDS_INACTIVE 8

t_delay mainDelay;
t_ramp speedRamp;

int motorDelay = 0;
DCMOTOR_DECLARE(C);
int pot;
unsigned char userDetected;

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
//----------- Analog setup ----------------
	analogInit();		// init analog module
	analogSelect(0,K3);	// assign connector K3 to analog channel 0
    dcmotorInit(C);

	rampInit(&speedRamp);
	rampSetPos(&speedRamp,0);
	
	speedRamp.maxSpeed = 5000;
	speedRamp.maxAccel = speedRamp.maxDecel = 15000;
	
	//EEreadMain();
}

void userDetectService()
{
	static int oldPot;
	static int stableCount = 0;
	
	if(abs(oldPot - pot) > 1024) {
		stableCount = 0;
		oldPot = pot;
		userDetected = 1;
		return;
	} else {
		if(stableCount > (200 * SECONDS_INACTIVE)) userDetected = 0;
		else  stableCount++;
		return;
	}
}

void loop() {
	int tmp;
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
//	switchService();	// switch management routine
//	fraiseService();	// listen to Fraise events
	analogService();	// switch management routine
//	fraiseService();	// listen to Fraise events

	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay
		analogSend();		// send analog channels that changed
		
		pot = analogGet(0);
		userDetectService();
		
		if(userDetected) {
			tmp = pot / 256;
			if(tmp < 5) tmp = 0;
			//else tmp = (tmp + 50) * 6;
			else tmp = (tmp * 1) + 350;
			rampGoto(&speedRamp, tmp);
		} else rampGoto(&speedRamp, 0);
		
		rampCompute(&speedRamp);
		DCMOTOR(C).Vars.PWMConsign = rampGetPos(&speedRamp);
		DCMOTOR(C).Setting.Mode = 0;
		
		DCMOTOR_COMPUTE(C,ASYM);
		
		printf("C M %d\n", DCMOTOR(C).Vars.PWMConsign);
	}
}

// Receiving

void fraiseReceiveChar() // receive textfraiseReceive
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
	else if((c=='S') && (fraiseGetChar()=='A') && (fraiseGetChar()=='V') && (fraiseGetChar()=='E'))
	    EEwriteMain();
}

void fraiseReceive() // receive raw
{
 	unsigned int i;
 	unsigned char c = fraiseGetChar();
 	
	switch(c) {
	    case 120 : DCMOTOR_INPUT(C) ; break;
	    case 121 : rampInput(&speedRamp) ; break;
	}
}

void EEdeclareMain()
{
    rampDeclareEE(&speedRamp);
}
