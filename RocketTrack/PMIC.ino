uint8_t batvolt=0x00;
bool livepmicdata=false;
bool PMIC_semaphore=false;

bool short_button_press=false;
bool long_button_press=false;

AXP20X_Class axp;

void PMIC_Interrupt(void)
{
    PMIC_semaphore=true;
}

int SetupPMIC(void)
{
	Serial.print("AXP192 Init");
	if(!axp.begin(Wire, AXP192_SLAVE_ADDRESS))	{	Serial.println(" PASS\r\n");	} 
	else                                        {	Serial.println(" FAIL\r\n");	}
	
	axp.setPowerOutPut(AXP192_LDO2,AXP202_ON);
	axp.setPowerOutPut(AXP192_LDO3,AXP202_ON);
	axp.setPowerOutPut(AXP192_DCDC2,AXP202_ON);
	axp.setPowerOutPut(AXP192_EXTEN,AXP202_ON);
	axp.setPowerOutPut(AXP192_DCDC1,AXP202_ON);
	axp.setDCDC1Voltage(3300);
	
	Serial.printf("\tDCDC1 2v5: %s\r\n",axp.isDCDC1Enable()?"ENABLE":"DISABLE");
	Serial.printf("\tDCDC2 Not Used: %s\r\n",axp.isDCDC2Enable()?"ENABLE":"DISABLE");
	Serial.printf("\tLDO2 LoRa: %s\r\n",axp.isLDO2Enable()?"ENABLE":"DISABLE");
	Serial.printf("\tLDO3 GPS: %s\r\n",axp.isLDO3Enable()?"ENABLE":"DISABLE");
	Serial.printf("\tDCDC3 3v3: %s\r\n",axp.isDCDC3Enable()?"ENABLE":"DISABLE");
	Serial.printf("\tExten: %s\r\n\n",axp.isExtenEnable()?"ENABLE":"DISABLE");

	if(axp.isChargeingEnable())	{	Serial.print("Charging is enabled\r\n\n");	}
	else						{	Serial.print("Charging is disabled\r\n\n");	}
	
	axp.adc1Enable(AXP202_BATT_CUR_ADC1,true);

    //! attachInterrupt to gpio 35
    pinMode(PMIC_IRQ,INPUT_PULLUP);
    attachInterrupt(PMIC_IRQ,PMIC_Interrupt,FALLING);
    axp.clearIRQ();

    //! enable all irq channel
    axp.enableIRQ(AXP202_ALL_IRQ, true);
	
	return(0);
}

void PollPMIC(void)
{
	static uint32_t updateat=0;
	
	if(PMIC_semaphore)
	{
        axp.readIRQ();
        Serial.println("axp20x irq enter!");

        if(axp.isPEKShortPressIRQ())	{	short_button_press=true;	}
        if(axp.isPEKLongtPressIRQ())	{	long_button_press=true;		}
		        
	    axp.clearIRQ();
        PMIC_semaphore=false;
	}

	if(millis()>updateat)
	{
		lastfix.voltage=axp.getBattVoltage();
		float batterycurrent=axp.getBattChargeCurrent();
		
		if(livepmicdata)
		{
			Serial.printf("Battery voltage = %.1f mV",lastfix.voltage);
			if(axp.isChargeing())	{	Serial.printf(", charging at %.1f mA ...\r\n",batterycurrent);	}
			else					{	Serial.print("\r\n");												}
		}
		
		updateat=millis()+1000L;
	}
}

int PMICCommandHandler(uint8_t *cmd,uint16_t cmdptr)
{
	// ignore a single key stroke
	if(cmdptr<=2)	return(0);

#if (DEBUG>0)
	Serial.println((char *)cmd);
#endif
	
	int retval=1;
	float batteryvoltage=axp.getBattVoltage();
	float batterychargecurrent=axp.getBattChargeCurrent();
	
	switch(cmd[1]|0x20)
	{
		case 'l':	livepmicdata=!livepmicdata;
					if(livepmicdata)	Serial.print("Live PMIC data on\r\n");
					else				Serial.print("Live PMIC data disabled\r\n");
					break;
					
		case 'c':	if(axp.isChargeing())	Serial.print("PMIC is charging\r\n");
					else					Serial.print("PMIC is not charging\r\n");
					break;
		
		case 'v':	Serial.printf("Battery voltage = %.1f mV\r\n",batteryvoltage);
					break;
		
		case 'i':	Serial.printf("Battery charge current = %.1f mA\r\n",batterychargecurrent);
					break;
		
		case '?':	Serial.print("PMIC Test Harness\r\n=================\r\n\n");
					Serial.print("l\t-\tLive PMIC data on/off\r\n");
					Serial.print("c\t-\tCheck charging status\r\n");
					Serial.print("v\t-\tCheck battery voltage\r\n");
					Serial.print("?\t-\tShow this menu\r\n");
					break;
		
		default:	// ignore
					break;
	}
	
	return(retval);
}

void ControlLED(axp_chgled_mode_t Mode)
{
//	Serial.print("ControlLED() entry\r\n");
	
	static axp_chgled_mode_t OldMode=AXP20X_LED_OFF;

	if(Mode!=OldMode)
	{
		axp.setChgLEDMode(Mode);
		OldMode=Mode;
	}
	
//	Serial.print("ControlLED() exit\r\n");
}

