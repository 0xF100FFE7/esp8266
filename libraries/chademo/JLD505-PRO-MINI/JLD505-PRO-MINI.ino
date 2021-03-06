#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <mcp_can.h>
#include <INA226.h>
#include <EEPROMAnything.h>
#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
//#include <DS2480B.h>
#include <DallasTemperature.h>
#include <FrequencyTimer2.h>

#include "shared.h"
int kangoo_charge_state = 8; //END state (not running)
void kangoo_charge_start();
void kangoo_charge_stop();
void kangoo_charge_cycle();

/*
Notes on what needs to be done:
- Timing analysis showed that the USB, CANBUS, and BT routines take up entirely too much time. They can delay processing by
	 almost 100ms!
*/

//#define DEBUG_TIMING	//if this is defined you'll get time related debugging messages

SoftwareSerial BTSerial(A2, A3); // RX | TX
AltSoftSerial altSerial; //pins 8 and 9

//DS2480B ds(altSerial);
//DallasTemperature sensors(&ds);

//These have been moved to eeprom. After initial compile the values will be read from EEPROM.
//These thus set the default value to write to eeprom upon first start up
#define MAX_CHARGE_V	400
#define MAX_CHARGE_A	40
#define TARGET_CHARGE_V	395
#define MIN_CHARGE_A	10
#define INITIAL_SOC 50

//set the proper digital pins for these
#define IN0		4
#define IN1		7
#define OUT0	5
#define OUT1	6

#define EMULATE_CHARGER_PIN 2
//#define CHADEMO_PLUG_IN A2

INA226 ina;
const unsigned long Interval = 10;
unsigned long Time = 0; 
unsigned long PreviousMillis = 0;
unsigned long CurrentMillis = 0;
float Voltage = 0;
float Current = 0;
float Power = 0;
int Count = 0;
byte Command = 0; // "z" will reset the AmpHours and KiloWattHours counters
volatile bool can_tick = false;
volatile uint8_t bStartConversion = 0;
volatile uint8_t bGetTemperature = 0;
volatile uint8_t timerIntCounter = 0;
volatile uint8_t timerFastCounter  = 0;
volatile uint8_t sensorReadPosition = 255;
uint8_t tempSensorCount = 0;
int32_t canMsgID = 0;
unsigned char canMsg[8];
unsigned char Flag_Recv = 0;
volatile uint8_t debugTick = 0;
bool chademo_plug_in = false;
unsigned long chademo_plug_in_insertion_time = 0;

bool send_initial_settings_over_serial = true;

typedef struct
{
	uint8_t valid; //a token to store EEPROM version and validity. If it matches expected value then EEPROM is not reset to defaults
	float ampHours;
	float kiloWattHours;
	float packSizeKWH;
	float voltageCalibration;
	float currentCalibration;  
	float voltage_calibration_offset;
	float current_calibration_offset;
	uint16_t maxChargeVoltage;
	uint16_t targetChargeVoltage;
	uint8_t maxChargeAmperage;
	uint8_t minChargeAmperage;
	uint8_t charge_time;
	bool ignore_current_mismatch;
	bool ignore_voltage_mismatch;
	bool emulate_charger;
	bool limit_charge_current_by_bms;

	uint8_t SOC;
} EESettings;
EESettings settings;
#define EEPROM_VALID	0xDE

//Bunch o' chademo related stuff. 
uint8_t bStartedCharge = 0; //we have started a charge since the plug was inserted. Prevents attempts to restart charging if it stopped previously
uint8_t bChademoMode = 0; //accessed but not modified in ISR so it should be OK non-volatile
uint8_t bChademoSendRequests = 0; //should we be sending periodic status updates?
volatile uint8_t bChademoRequest = 0;  //is it time to send one of those updates?
//target values are what we send with periodic frames and can be changed.
uint8_t askingAmps = 0; //how many amps to ask for. Trends toward targetAmperage
uint8_t bListenEVSEStatus = 0; //should we pay attention to stop requests and such yet?
uint8_t bDoMismatchChecks = 0; //should we be checking for voltage and current mismatches?

uint32_t mismatchStart;
uint32_t stateMilli;
uint32_t insertionTime = 0;
enum CHADEMOSTATE 
{
	STARTUP,
	SEND_INITIAL_PARAMS,
	WAIT_FOR_EVSE_PARAMS,
	SET_CHARGE_BEGIN,
	WAIT_FOR_BEGIN_CONFIRMATION,
	CLOSE_CONTACTORS,
	RUNNING,
	CEASE_CURRENT,
	WAIT_FOR_ZERO_CURRENT,
	OPEN_CONTACTOR,
	FAULTED,
	STOPPED,
	LIMBO
};
CHADEMOSTATE chademoState = STOPPED;
CHADEMOSTATE stateHolder = STOPPED;

typedef struct 
{
	uint8_t supportWeldCheck;
	uint16_t availVoltage;
	uint8_t availCurrent;
	uint16_t thresholdVoltage; //evse calculates this. It is the voltage at which it'll abort charging to save the battery pack in case we asked for something stupid
} EVSE_PARAMS;
EVSE_PARAMS evse_params;

typedef struct 
{
	uint16_t presentVoltage;
	uint8_t presentCurrent;
	uint8_t status;
	uint16_t remainingChargeSeconds;
} EVSE_STATUS;
EVSE_STATUS evse_status;

typedef struct 
{
	int got_charging_info;
	int temp;
	int max_input_power;
	float amperage;
	float voltage;
	float soc;
} BMS_STATUS;
BMS_STATUS bms_status;

typedef struct 
{
	uint16_t targetVoltage; //what voltage we want the EVSE to put out
	uint8_t targetCurrent; //what current we'd like the EVSE to provide
	uint8_t remainingKWH; //report # of KWh in the battery pack (charge level)
	uint8_t battOverVolt : 1; //we signal that battery or a cell is too high of a voltage
	uint8_t battUnderVolt : 1; //we signal that battery is too low
	uint8_t currDeviation : 1; //we signal that measured current is not the same as EVSE is reporting
	uint8_t battOverTemp : 1; //we signal that battery is too hot
	uint8_t voltDeviation : 1; //we signal that we measure a different voltage than EVSE reports
	uint8_t chargingEnabled : 1; //ask EVSE to enable charging
	uint8_t notParked : 1; //advise EVSE that we're not in park.
	uint8_t chargingFault : 1; //signal EVSE that we found a fault
	uint8_t contactorOpen : 1; //tell EVSE whether we've closed the charging contactor 
	uint8_t stopRequest : 1; //request that the charger cease operation before we really get going
} CARSIDE_STATUS;
CARSIDE_STATUS carStatus;

//The IDs for chademo comm - both carside and EVSE side so we know what to listen for
//as well.
#define CARSIDE_BATT		0x100
#define CARSIDE_CHARGETIME	0x101
#define CARSIDE_CONTROL		0x102

#define EVSE_PARAMS			0x108
#define EVSE_STATUS			0x109

#define CARSIDE_FAULT_OVERV		1 //over voltage
#define CARSIDE_FAULT_UNDERV	2 //Under voltage
#define CARSIDE_FAULT_CURR		4 //current mismatch
#define CARSIDE_FAULT_OVERT		8 //over temperature
#define CARSIDE_FAULT_VOLTM		16 //voltage mismatch

#define CARSIDE_STATUS_CHARGE	1 //charging enabled
#define CARSIDE_STATUS_NOTPARK	2 //shifter not in safe state
#define CARSIDE_STATUS_MALFUN	4 //vehicle did something dumb
#define CARSIDE_STATUS_CONTOP	8 //main contactor open
#define CARSIDE_STATUS_CHSTOP	16 //charger stop before even charging

#define EVSE_STATUS_CHARGE		1 //charger is active
#define EVSE_STATUS_ERR			2 //something went wrong
#define EVSE_STATUS_CONNLOCK	4 //connector is currently locked
#define EVSE_STATUS_INCOMPAT	8 //parameters between vehicle and charger not compatible
#define EVSE_STATUS_BATTERR		16 //something wrong with battery?!
#define EVSE_STATUS_STOPPED		32 //charger is stopped

void MCP2515_ISR()
{
	Flag_Recv = 1;
}

void timer2Int()
{
	can_tick = true;
	timerFastCounter++;
	if (timerFastCounter == 10)
	{
		debugTick = 1;
		if (bChademoMode  && bChademoSendRequests) bChademoRequest = 1;
		timerFastCounter = 0;
		timerIntCounter++;
		if (timerIntCounter < 12)
		{
			bGetTemperature = 1;
			sensorReadPosition++;
		}
		if (timerIntCounter == 12)
		{
			bStartConversion = 1;
			sensorReadPosition = 255;
		}
		if (timerIntCounter == 22)
		{
			timerIntCounter = 0;
		}
	}
}
	
//will wait delayTime milliseconds and then transition to new state. Sets state to LIMBO in the meantime
void chademoDelayedState(int newstate, uint16_t delayTime)
{
	chademoState = LIMBO;
	stateHolder = (CHADEMOSTATE)newstate;
	stateMilli = millis() + delayTime;
}

void parse(struct cmd &cmd) {
	switch (cmd.type) {
	case CMD_RESET:
		settings.SOC = 0;
		settings.ampHours = 0.0;
		settings.kiloWattHours = 0.0;
		break;  
	
	case CMD_SET_TARGET_VOLT:
		settings.targetChargeVoltage = cmd.buf.toInt();
	
	case CMD_CAL_AMPS:
		settings.current_calibration_offset = cmd.buf.toFloat();
		break;
	
	case CMD_CAL_VOLT:
		settings.current_calibration_offset = cmd.buf.toFloat();
		break;
	
	case CMD_SET_CAPACITY:
		settings.packSizeKWH = cmd.buf.toInt();
		break;
	
	case CMD_SET_MAX_VOLT:
		settings.maxChargeVoltage = cmd.buf.toFloat();
		break;
	case CMD_SET_MAX_ASKING_AMPS:
		settings.maxChargeAmperage = cmd.buf.toInt();
		break;
		
	case CMD_IGNORE_CURRENT_MISMATCH:
		settings.ignore_current_mismatch = cmd.buf.toInt();
		break;
	
	case CMD_IGNORE_VOLTAGE_MISMATCH:
		settings.ignore_voltage_mismatch = cmd.buf.toInt();
		break;
		
	case CMD_END_CHARGE: //emergency stop signal
		chademoState = CEASE_CURRENT;
		break;
		
	case CMD_EMULATE_CHARGER:
		if (settings.emulate_charger = cmd.buf.toInt())
			kangoo_charge_start();
		else
			kangoo_charge_stop();
		break;
	
	case CMD_LIMIT_CHARGE_CURRENT_BY_BMS:
		settings.limit_charge_current_by_bms = cmd.buf.toInt();
		break;
		
	case CMD_SET_CHARGE_TIME:
		settings.charge_time = cmd.buf.toInt();
		break;
		
	default:
		break;
	}
}

void setup()
{ 
//first thing configure the I/O pins and set them to a sane state
	pinMode(IN0, INPUT);
	pinMode(IN1, INPUT);
	pinMode(OUT0, OUTPUT);
	pinMode(OUT1, OUTPUT);
	digitalWrite(OUT0, LOW);
	digitalWrite(OUT1, LOW);
	pinMode(A1, OUTPUT); //KEY - Must be HIGH
	pinMode(A0, INPUT); //STATE
	digitalWrite(A1, HIGH);
	pinMode(3, INPUT_PULLUP); //enable weak pull up on MCP2515 int pin connected to INT1 on MCU
	pinMode(EMULATE_CHARGER_PIN, OUTPUT);
	//pinMode(CHADEMO_PLUG_IN, INPUT_PULLUP); //Input or output?


	Serial.begin(115200);
	BTSerial.begin(115200);
	altSerial.begin(9600);
	
//xxx	sensors.begin();
//xxx	sensors.setWaitForConversion(false); //we're handling the time delay ourselves so no need to wait when asking for temperatures
	
	CAN.begin(CAN_500KBPS);
	attachInterrupt(1, MCP2515_ISR, FALLING);     // start interrupt

	ina.begin();//69
	ina.configure(INA226_AVERAGES_16, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);//  ina.configure(INA226_AVERAGES_16, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);
	ina.calibrate(0.0000833333, 600);

	EEPROM_readAnything(0, settings);
	if (settings.valid != EEPROM_VALID) //not proper version so reset to defaults
	{
		settings.valid = EEPROM_VALID;
		settings.ampHours = 0;
		settings.kiloWattHours = 0.0;
		settings.currentCalibration = 200.0/0.075; //800A 75mv shunt
		settings.voltageCalibration = (100000.0*830000.0/930000.0+1000000.0)/(100275.0*830000.0/930000.0); // (Voltage Divider with (100k in parallel with 830k) and 1M )
		settings.current_calibration_offset = 0.0;
		settings.voltage_calibration_offset = 0.0;
		settings.packSizeKWH = 28.0; //just a random guess. Maybe it should default to zero though?
		settings.maxChargeAmperage = MAX_CHARGE_A;
		settings.maxChargeVoltage = MAX_CHARGE_V;
		settings.targetChargeVoltage = TARGET_CHARGE_V;
		settings.minChargeAmperage = MIN_CHARGE_A;
		settings.charge_time = 90;
		settings.ignore_current_mismatch = true;
		settings.ignore_voltage_mismatch = true;
		settings.emulate_charger = true;
		settings.limit_charge_current_by_bms = true;
		settings.SOC = INITIAL_SOC;
		EEPROM_writeAnything(0, settings);
	}

	settings.ampHours = settings.kiloWattHours = 0.0;

	//attachInterrupt(0, Save, FALLING);
	FrequencyTimer2::setPeriod(20000); //interrupt every 20ms
	FrequencyTimer2::setOnOverflow(timer2Int);
	
	Serial.print(F("Found "));
//xxx	tempSensorCount = sensors.getDeviceCount(); 
	Serial.print(tempSensorCount);
	Serial.println(F(" temperature sensors."));

	carStatus.targetCurrent = settings.maxChargeAmperage;
	carStatus.targetVoltage = settings.targetChargeVoltage;
	carStatus.contactorOpen = 1;
	
	/*if (settings.emulate_charger)
		kangoo_charge_start();*/
}

void loop()
{
	static struct cmd cmd;
	if (cmd.get())
		parse(cmd);

	//Basic debouncer implementation
	/*static bool prev_plug_in_state = false;
	if (!digitalRead(CHADEMO_PLUG_IN)) {
		if (chademo_plug_in_insertion_time == 0)
			chademo_plug_in_insertion_time = millis();
			
		if (millis() - chademo_plug_in_insertion_time >= 500)
			chademo_plug_in = true;	
	} else {
		chademo_plug_in_insertion_time = 0;
		chademo_plug_in = false;
	}
	
	//If we plugged out chademo cable, only then call end of charge
	if (chademo_plug_in != prev_plug_in_state && chademo_plug_in == false) {
		cmd.send(CMD_KWH, settings.kiloWattHours);
		cmd.send(CMD_END_CHARGE);
		kangoo_charge_stop();
	}
	
	prev_plug_in_state = chademo_plug_in;
	*/
	uint8_t pos;
	CurrentMillis = millis();
	uint8_t len;

#ifdef DEBUG_TIMING
	if (debugTick == 1)
	{
		debugTick = 0;
		Serial.println(millis());
	}
#endif 
 
	if(CurrentMillis - PreviousMillis >= Interval)
	{
		Time = CurrentMillis - PreviousMillis;
		PreviousMillis = CurrentMillis;   

		if (!bDoMismatchChecks && chademoState == RUNNING)
		{
			if (CurrentMillis > mismatchStart) bDoMismatchChecks = 1;
		}

		if (chademoState == LIMBO && CurrentMillis > stateMilli)
		{
			chademoState = stateHolder;
		}
		
		Count++;
		
		if (bms_status.got_charging_info > 0) {
			Voltage = bms_status.voltage;
			Current = -bms_status.amperage;
			settings.SOC = bms_status.soc;
			bms_status.got_charging_info--;
		} else {
			kangoo_charge_state = 8; //Immediate stop
			//Voltage = ina.readBusVoltage() * (settings.voltageCalibration + settings.voltage_calibration_offset);
			//Current = ina.readShuntVoltage() * (settings.currentCalibration + settings.current_calibration_offset);
			settings.SOC = (-settings.kiloWattHours / settings.packSizeKWH) * 100;
		}
		
		Power = Voltage * Current / 1000.0;
		settings.ampHours += Current * (float)Time / 1000.0 / 3600.0;
		settings.kiloWattHours += Power * (float)Time / 1000.0 / 3600.0;
		
		if (chademoState == RUNNING && bDoMismatchChecks)
		{
			static int voltage_mismatch_count = 0;
			static int current_mismatch_count = 1;
			
			if ((abs(abs(Voltage) - evse_status.presentVoltage) > 7 && !carStatus.voltDeviation) && !settings.ignore_voltage_mismatch)
			{
				if (voltage_mismatch_count++ > 5) {
					Serial.println(F("Voltage mismatch! Aborting!"));
					carStatus.voltDeviation = 1;
					voltage_mismatch_count = 0;
					chademoState = CEASE_CURRENT;
				}
			} else if (voltage_mismatch_count > 0) {
				voltage_mismatch_count--;
			}

			if ((abs(abs(Current) - evse_status.presentCurrent) > 7 && !carStatus.currDeviation) && !settings.ignore_current_mismatch)
			{
				if (current_mismatch_count++ > 5) {
					Serial.println(F("Current mismatch! Aborting!"));
					carStatus.currDeviation = 1;
					current_mismatch_count = 0;
					chademoState = CEASE_CURRENT;
				}
			} else if (current_mismatch_count > 0) {
				current_mismatch_count--;
			}

			if (Voltage > settings.maxChargeVoltage)
			{
				Serial.println(F("Over voltage fault!"));
				carStatus.battOverVolt = 1;
				chademoState = CEASE_CURRENT;
			}
			//Constant Current/Constant Voltage Taper checks.  If minimum current is set to zero, we terminate once target voltage is reached.
			//If not zero, we will adjust current up or down as needed to maintain voltage until current decreases to the minimum entered
								
			if(Count==20)  //To allow batteries time to react, we only do this once in 50 counts
			{
				if (Voltage > settings.targetChargeVoltage-1) //All initializations complete and we're running.We've reached charging target
				{
					if (settings.minChargeAmperage = 0 || carStatus.targetCurrent < settings.minChargeAmperage) chademoState = CEASE_CURRENT;  //Terminate charging
						else carStatus.targetCurrent--;  //Taper. Actual decrease occurs in sendChademoStatus                                   
				} 
				else //Only adjust upward if we have previous adjusted downward and do not exceed max amps
				{
					if (carStatus.targetCurrent < settings.maxChargeAmperage) //settings.maxChargeAmperage
						carStatus.targetCurrent++;
					else if (carStatus.targetCurrent > settings.maxChargeAmperage)
						carStatus.targetCurrent--;
				}
			}
 		}

							 
		//if (!bChademoMode) 
		//{
			if (Count >= 50)
			{
				Count = 0;
				USB();												
				if (!bChademoMode) //save some processor time by not doing these in chademo mode
				{
					if (send_initial_settings_over_serial)
					{
						struct cmd cmd;
						cmd.send(CMD_CAL_AMPS, settings.current_calibration_offset);
						cmd.send(CMD_CAL_VOLT, settings.voltage_calibration_offset);
						cmd.send(CMD_SET_TARGET_VOLT, settings.targetChargeVoltage);
						cmd.send(CMD_SET_CAPACITY, settings.packSizeKWH);
						cmd.send(CMD_SET_MAX_VOLT, settings.maxChargeVoltage);
						cmd.send(CMD_SET_MAX_ASKING_AMPS, settings.maxChargeAmperage);
						cmd.send(CMD_SET_CHARGE_TIME, settings.charge_time);
						cmd.send(CMD_IGNORE_CURRENT_MISMATCH, settings.ignore_current_mismatch);
						cmd.send(CMD_IGNORE_VOLTAGE_MISMATCH, settings.ignore_voltage_mismatch);
						cmd.send(CMD_EMULATE_CHARGER, settings.emulate_charger);
						cmd.send(CMD_LIMIT_CHARGE_CURRENT_BY_BMS, settings.limit_charge_current_by_bms);
						send_initial_settings_over_serial = false;
					}
					//CANBUS();
					BT();
				}
				else 
				{
					Serial.print(F("Chademo Mode: "));
					Serial.println(chademoState);
				}
				Save();
			}
		//}		
	}

	if (Flag_Recv || CAN.checkReceive() == CAN_MSGAVAIL) {
		Flag_Recv = 0;		
		CAN.readMsgBuf(&len, canMsg);            // read data,  len: data length, buf: data buf
		canMsgID = CAN.getCanId();
		switch (canMsgID) {
		
		case EVSE_PARAMS:
			if (chademoState == WAIT_FOR_EVSE_PARAMS) chademoDelayedState(SET_CHARGE_BEGIN, 100);
			evse_params.supportWeldCheck = canMsg[0];
			evse_params.availVoltage = canMsg[1] + canMsg[2] * 256;
			evse_params.availCurrent = canMsg[3];			
			evse_params.thresholdVoltage = canMsg[4] + canMsg[5] * 256;

			//if charger cannot provide our requested voltage then GTFO
			if (evse_params.availVoltage < carStatus.targetVoltage)
			{
				Serial.println(F("EVSE can't provide needed voltage. Aborting."));
				Serial.println(evse_params.availVoltage);
				chademoState = CEASE_CURRENT;
			}

			//if we want more current then it can provide then revise our request to match max output
			if (evse_params.availCurrent < carStatus.targetCurrent) carStatus.targetCurrent = min(evse_params.availCurrent, settings.maxChargeAmperage);
			break;
			
		case EVSE_STATUS:
			evse_status.presentVoltage = canMsg[1] + 256 * canMsg[2];
			evse_status.presentCurrent  = canMsg[3];
			evse_status.status = canMsg[5];				
			if (canMsg[6] < 0xFF)
			{
				evse_status.remainingChargeSeconds = canMsg[6] * 10;
			}
			else 
			{
				evse_status.remainingChargeSeconds = canMsg[7] * 60;
			}

			//on fault try to turn off current immediately and cease operation
			if ((evse_status.status & 0x1A) != 0) //if bits 1, 3, or 4 are set then we have a problem.
			{
				Serial.println(F("EVSE reports fault. Aborting."));
				if (chademoState == RUNNING) chademoState = CEASE_CURRENT;
			}
			
			if (chademoState == RUNNING)
			{
				if (bListenEVSEStatus)
				{
					if ((evse_status.status & EVSE_STATUS_STOPPED) != 0)
					{
						Serial.println(F("EVSE requests we stop charging."));
						chademoState = CEASE_CURRENT;
					}

					//if there is no remaining time then gracefully shut down
					if (evse_status.remainingChargeSeconds == 0)
					{
						Serial.println(F("EVSE reports time elapsed. Finishing."));
						chademoState = CEASE_CURRENT;
					}
				}
				else
				{
					//if charger is not reporting being stopped and is reporting remaining time then enable the checks.
					if ((evse_status.status & EVSE_STATUS_STOPPED) == 0 && evse_status.remainingChargeSeconds > 0) bListenEVSEStatus = 1;
				}
			}
			break;
			
		case 0x155:
			//Once car awaken from sleep and charger emulator is enabled
			if (settings.emulate_charger && !bms_status.got_charging_info)
				kangoo_charge_start();
			
			bms_status.got_charging_info = 50;
			bms_status.amperage = (((canMsg[1] & 0x0F) << 8 | canMsg[2]) - 0x7D0) * 0.26;
			bms_status.voltage = (canMsg[6] << 8 | canMsg[7]) / 2;
			bms_status.soc = (float)canMsg[4] / 1.6 + ((float)canMsg[5] / 0xFF);
			break;
				
		case 0x424:
			bms_status.max_input_power = (canMsg[2] / 2);
			int max_input_amp = bms_status.max_input_power / Voltage;
			if (settings.limit_charge_current_by_bms && Current > max_input_amp)
				carStatus.targetCurrent = max_input_amp;
			bms_status.temp = canMsg[4] - 40;
			break;
			
		default:
			break;
		}
	}
	
	if (bStartConversion == 1)
	{
		bStartConversion = 0;
//xxx		sensors.requestTemperatures();
	}
	if (bGetTemperature)
	{
		bGetTemperature = 0;
		pos = sensorReadPosition;
		if (pos < tempSensorCount)
		{		  
			Serial.print(pos);
			Serial.print(": ");
			//sensors.isConnected(pos);
			// Serial.println(sensors.getCelsius(pos));
//			Serial.println(sensors.getTempCByIndex(pos));  xxx
		}
	}

	if (!digitalRead(IN1)) //IN1 goes low if we have been plugged into the chademo port
	{
		if (insertionTime == 0)
		{
			insertionTime = millis();
		}
		else if (millis() > insertionTime + 500)
		{
			if (bChademoMode == 0)
			{
				bChademoMode = 1;
				if (chademoState == STOPPED && !bStartedCharge) {
					chademoState = STARTUP;
					Serial.println(F("Starting Chademo process."));
					carStatus.battOverTemp = 0;
					carStatus.battOverVolt = 0;
					carStatus.battUnderVolt = 0;
					carStatus.chargingFault = 0;
					carStatus.chargingEnabled = 0;
					carStatus.contactorOpen = 1;
					carStatus.currDeviation = 0;
					carStatus.notParked = 0;
					carStatus.stopRequest = 0;
					carStatus.voltDeviation = 0;
				}
			}
		}
	}
	else
	{
		insertionTime = 0;
		if (bChademoMode == 1)
		{
			Serial.println(F("Stopping chademo process."));
			bChademoMode = 0;
			bStartedCharge = 0;
			chademoState = STOPPED;
			//maybe it would be a good idea to try to see if EVSE is still transmitting to us and providing current
			//as it is not a good idea to open the contactors under load. But, IN1 shouldn't trigger 
			//until the EVSE is ready. Also, the EVSE should have us locked so the only way the plug should come out under
			//load is if the idiot driver took off in the car. Bad move moron.
			digitalWrite(OUT0, LOW);
			digitalWrite(OUT1, LOW);
		}
	}

	if (bChademoMode)
	{
		struct cmd cmd;
		if (bChademoSendRequests && bChademoRequest)
		{
			bChademoRequest = 0;
			sendChademoStatus();
			sendChademoBattSpecs();
			sendChademoChargingTime();
			//Serial.println("Tx");
		}

		switch (chademoState)
		{
		case STARTUP: //really useful state huh?
			chademoDelayedState(SEND_INITIAL_PARAMS, 100);
			break;
		case SEND_INITIAL_PARAMS:
			//we could do calculations to see how long the charge should take based on SOC and 
			//also set a more realistic starting amperage. Options for the future.
			//One problem with that is that we don't yet know the EVSE parameters so we can't know
			//the max allowable amperage just yet.
			bChademoSendRequests = 1; //causes chademo frames to be sent out every 100ms
			chademoDelayedState(WAIT_FOR_EVSE_PARAMS, 100);
			Serial.println(F("Sent parameters to EVSE. Waiting."));
			break;
		case WAIT_FOR_EVSE_PARAMS:
			//for now do nothing while we wait. Might want to try to resend start up messages periodically if no reply
			break;
		case SET_CHARGE_BEGIN:
			Serial.println(F("Setting begin charge request."));
			digitalWrite(OUT1, HIGH); //signal that we're ready to charge
			carStatus.chargingEnabled = 1; //should this be enabled here???
			chademoDelayedState(WAIT_FOR_BEGIN_CONFIRMATION, 150);
			break;
		case WAIT_FOR_BEGIN_CONFIRMATION:
			if (digitalRead(IN0)) //inverse logic from how IN1 works. Be careful!
			{
				chademoDelayedState(CLOSE_CONTACTORS, 100);
			}
			break;
		case CLOSE_CONTACTORS:
			cmd.send(CMD_BEGIN_CHARGE);
			Serial.println(F("Closing contactor"));
			digitalWrite(OUT0, HIGH);
			chademoDelayedState(RUNNING, 150);
			carStatus.contactorOpen = 0; //its closed now
			carStatus.chargingEnabled = 1; //please sir, I'd like some charge
			bStartedCharge = 1;
			mismatchStart = millis() + 10000; //start mismatch checks 10 seconds after we start the charge
			break;
		case RUNNING:
			//do processing here by taking our measured voltage, amperage, and SOC to see if we should be commanding something
			//different to the EVSE. Also monitor temperatures to make sure we're not incinerating the pack.
			break;
		case CEASE_CURRENT:
			Serial.println(F("Setting current request to zero."));
			carStatus.targetCurrent = 0;
			chademoState = WAIT_FOR_ZERO_CURRENT;
			break;
		case WAIT_FOR_ZERO_CURRENT:
			if (evse_status.presentCurrent == 0)
			{
				chademoDelayedState(OPEN_CONTACTOR, 50);
			}
			break;
		case OPEN_CONTACTOR:
			Serial.println(F("Opening contactor"));
			digitalWrite(OUT0, LOW);
			carStatus.contactorOpen = 1;
			carStatus.chargingEnabled = 0;
			sendChademoStatus(); //we probably need to force this right now
			chademoDelayedState(STOPPED, 100);
			break;
		case FAULTED:
			//cmd.send(CMD_END_CHARGE);
			Serial.println(F("Detected fault!"));
			chademoState = CEASE_CURRENT;
			//digitalWrite(OUT0, LOW);
			//digitalWrite(OUT1, LOW);
			break;
		case STOPPED:
			//cmd.send(CMD_END_CHARGE);
			digitalWrite(OUT0, LOW);
			digitalWrite(OUT1, LOW);
			bChademoSendRequests = 0; //don't need to keep sending anymore.
			break;
		}
	}
	
	kangoo_charge_cycle();
}

void Save()
{
	EEPROM_writeAnything(0, settings);
}  

void USB()
{
	static struct cmd cmd;
	cmd.send(CMD_CAN_MODE, bms_status.got_charging_info ? true : false);
	cmd.send(CMD_VOLTAGE, Voltage);
	cmd.send(CMD_CURRENT, Current);
	cmd.send(CMD_SOC, settings.SOC);
	cmd.send(CMD_POWER, Power);
	cmd.send(CMD_AMPHOURS, settings.ampHours);
	cmd.send(CMD_KWH, settings.kiloWattHours);
	cmd.send(CMD_TEMP, bms_status.temp);
	cmd.send(CMD_TIME_REMAINING, bms_status.got_charging_info ? evse_status.remainingChargeSeconds : 0);
	cmd.send(CMD_MAX_INPUT_POWER, bms_status.max_input_power);
	cmd.send(CMD_UPDATE_STATUS);
	//cmd.send(90, "\n");
	
	String evs_stat =
	String("Evse current: ") + evse_status.presentCurrent + "\n" +
	"Evse voltage: " + evse_status.presentVoltage + "\n";
	//"Evse time remaining: " + (evse_status.remainingChargeSeconds / 60 / 60) + ":" + ((evse_status.remainingChargeSeconds / 60) % 60) + ":" + (evse_status.remainingChargeSeconds % 60);
	
	cmd.send(CMD_EVSE_STATUS, evs_stat);
	/*Serial.print (Voltage, 3);   
	Serial.print ("V ");
	Serial.print (Current, 2);    
	Serial.print ("A ");
	Serial.print (settings.ampHours, 1);    
	Serial.print ("Ah ");
	Serial.print (Power, 1);        
	Serial.print ("kW ");
	Serial.print (settings.kiloWattHours, 1);    
	Serial.print ("kWh ");
	Serial.print ("SOC ");
	Serial.println (settings.SOC);  
	if (Serial.available() > 0) {
		switch(Serial.read()) {
		case 'z':
			Serial.println("Reset Ah & Wh");
			settings.SOC = 0;
			settings.ampHours = 0.0;
			settings.kiloWattHours = 0.0;
			Serial.println("Done!!!"); 
			break;
		
		case '+':
			settings.SOC += 1;
			//settings.voltageCalibration +=0.004;
			//Serial.println (settings.voltageCalibration, 5);   
			break;
				
		case '-':
			settings.SOC -= 1;
			//settings.voltageCalibration -=0.004;
			//Serial.println (settings.voltageCalibration, 5);   
			break;

		case 'r':
			insertionTime = 0;
			if (bChademoMode == 1) {
				Serial.println("Repeating chademo process");
				bChademoMode = 0;
				bStartedCharge = 0;
				chademoState = STOPPED;
				digitalWrite(OUT0, LOW);
				digitalWrite(OUT1, LOW);
				bChademoSendRequests = 0;
			} 
			break;
			
		case 'e':
			Serial.println("Enabling mismatch checks");
			ignore_mismatches = false;
			break;

		case 'd':
			Serial.println("Disabling mismatch checks");
			ignore_mismatches = true;
			break;

		default:
			Serial.println("Unknown command");
			break;
		}
			
	 while(Serial.available()>0) Serial.read();}*/
}

void BT()
{
	
	BTSerial.print (Voltage, 2);   
	BTSerial.print ("V ");
	BTSerial.print (Current, 2);    
	BTSerial.print ("A ");
	BTSerial.print (settings.ampHours, 1);    
	BTSerial.print ("Ah ");
	BTSerial.print (Power, 1);        
	BTSerial.print ("kW ");
	BTSerial.print (settings.kiloWattHours, 1);    
	BTSerial.println ("kWh");
	
	/*
	BTSerial.write(02);
	BTSerial.write(highByte((int)(Voltage*10)));  
	BTSerial.write(lowByte((int)(Voltage*10)));
	BTSerial.write(highByte((int)(Current*10)));    
	BTSerial.write(lowByte((int)(Current*10)));
	BTSerial.write(highByte((int)(AmpHours*10)));    
	BTSerial.write(lowByte((int)(AmpHours*10)));
	BTSerial.write(highByte((int)(Power*10)));        
	BTSerial.write(lowByte((int)(Power*10)));
	BTSerial.write(highByte((int)(KiloWattHours*10)));    
	BTSerial.write(lowByte((int)(KiloWattHours*10)));
	BTSerial.write(03);*/
	
	if (BTSerial.available() > 0)
	{
		Command = BTSerial.read();
		if (Command == 'z')
		{
			settings.ampHours = 0.0;
			settings.kiloWattHours = 0.0;
		}
		while(BTSerial.available()>0) BTSerial.read();
	}
}

void kangoo_charge_start()
{
	kangoo_charge_state = 0;
}

void kangoo_charge_stop()
{
	kangoo_charge_state = 5;
}

void kangoo_charge_cycle()
{
	if (can_tick)
		can_tick = false;
	else
		return;
		
	static unsigned char msg1[4] = {0xAA, 0x20, 0x00, 0x00};
	static unsigned char msg2[7] = {0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x80};
	static unsigned char msg3[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x3F, 0xFF};
	static unsigned char msg4[8] = {0x00, 0x00, 0x28, 0x00, 0x90, 0x00, 0x00, 0x00};

	static unsigned long counter = 0;
	static unsigned long counter_prev = 0;
	enum {BEGIN, WAIT_TO_RELAY_ON, WAIT_TO_TURNERS_ON, WAIT_TO_START, NORMAL_CYCLE, INTERRUPT, WAIT_TO_RELAY_OFF, WAIT_TO_END, END};// state;
	int &state = kangoo_charge_state;
	
	if (counter - counter_prev >= 10) //100ms
	{
		counter_prev = counter;
	
		switch (state) {
		case BEGIN:
			digitalWrite(EMULATE_CHARGER_PIN, HIGH);
			counter = counter_prev = 0;
			msg2[0] = 0x00;
			msg2[3] = 0x00;
			msg2[6] = 0x80;
			msg4[2] = 0x28;
			msg4[4] = 0x90;
			
			state = WAIT_TO_RELAY_ON;
			break;
		
		case WAIT_TO_RELAY_ON:
			if (counter >= 64) {
				msg4[2] = 0x2C;
				state = WAIT_TO_TURNERS_ON;
			}
			break;

		case WAIT_TO_TURNERS_ON:
			if (counter >= 164) {
				msg4[4] = 0x28; //0x29 - (1 bit = cooler enabled)
				state = WAIT_TO_START;
			}
			break;

		case WAIT_TO_START:
			if (counter >= 400) { //4 sec
				msg2[0] = 0x20;
				msg2[3] = 0x10; //enables charger counter???
				state = NORMAL_CYCLE;
			}
			break;

		case NORMAL_CYCLE:
			//interrupt here if we need to end charging
			break;
			
		case INTERRUPT:
			counter = counter_prev = 0;
			msg2[0] = 0x00;
			msg2[6] = 0x00;
			msg4[2] = 0x28;
			msg4[4] = 0x90;
			
			state = WAIT_TO_RELAY_OFF;
			break;
			
		case WAIT_TO_RELAY_OFF:
			if (counter >= 30) { //3 cycles
				msg4[4] = 0xB0;
				state = WAIT_TO_END;
			}
			break;
		
		case WAIT_TO_END:
			if (counter >= 94) {
				state = END;
			}
			break;
			
		case END:
			digitalWrite(EMULATE_CHARGER_PIN, LOW);
			break;
		}
		
		if (state != END) {
			CAN.sendMsgBuf(0x428, 0, 7, msg2);
			CAN.sendMsgBuf(0x429, 0, 8, msg3);
			CAN.sendMsgBuf(0x4F7, 0, 8, msg4);
		}
	} else if (state != END) {
		CAN.sendMsgBuf(0x1C7, 0, 4, msg1);
	}
	
	counter++;
}

/*void CANBUS()
{
	canMsgID = 0x404;
	canMsg[0] = highByte((int)(Voltage * 10)); // Voltage High Byte
	canMsg[1] = lowByte((int)(Voltage * 10)); // Voltage Low Byte
	canMsg[2] = highByte((int)(Current * 10)); // Current High Byte
	canMsg[3] = lowByte((int)(Current * 10)); // Current Low Byte
	canMsg[4] = highByte((int)(settings.ampHours * 10)); // AmpHours High Byte
	canMsg[5] = lowByte((int)(settings.ampHours * 10)); // AmpHours Low Byte
	canMsg[6] = 0x00; // Not Used
	canMsg[7] = 0x00; // Not Used
	CAN.sendMsgBuf(canMsgID, 0, 6, canMsg);
	
	
	canMsgID = 0x505;
	canMsg[0] = highByte((int)(Power * 10)); // Power High Byte
	canMsg[1] = lowByte((int)(Power * 10)); // Power Low Byte
	canMsg[2] = highByte((int)(settings.kiloWattHours * 10)); // KiloWattHours High Byte
	canMsg[3] = lowByte((int)(settings.kiloWattHours * 10)); // KiloWattHours Low Byte
	canMsg[4] = 0x00; // Not Used
	canMsg[5] = 0x00; // Not Used
	canMsg[6] = 0x00; // Not Used
	canMsg[7] = 0x00; // Not Used
	CAN.sendMsgBuf(canMsgID, 0, 4, canMsg);
 }*/

void sendChademoBattSpecs()
{
	
	canMsgID = CARSIDE_BATT;
	canMsg[0] = 0x00; // Not Used
	canMsg[1] = 0x00; // Not Used
	canMsg[2] = 0x00; // Not Used
	canMsg[3] = 0x00; // Not Used
	canMsg[4] = lowByte(settings.maxChargeVoltage);
	canMsg[5] = highByte(settings.maxChargeVoltage); 
	canMsg[6] = 100; //settings.SOC;
	canMsg[7] = 0; //not used
	CAN.sendMsgBuf(canMsgID, 0, 8, canMsg);
}

void sendChademoChargingTime()
{
	
	canMsgID = CARSIDE_CHARGETIME;
	canMsg[0] = 0x00; // Not Used
	canMsg[1] = 0xFF; //not using 10 second increment mode
	canMsg[2] = settings.charge_time; //ask for how long of a charge? It will be forceably stopped if we hit this time
	canMsg[3] = 60; //how long we think the charge will actually take
	canMsg[4] = 0; //not used
	canMsg[5] = 0; //not used
	canMsg[6] = 0; //not used
	canMsg[7] = 0; //not used
	CAN.sendMsgBuf(canMsgID, 0, 8, canMsg);
}

void sendChademoStatus()
{
	uint8_t faults = 0;
	uint8_t status = 0;

	if (carStatus.battOverTemp) faults |= CARSIDE_FAULT_OVERT;
	if (carStatus.battOverVolt) faults |= CARSIDE_FAULT_OVERV;
	if (carStatus.battUnderVolt) faults |= CARSIDE_FAULT_UNDERV;
	if (carStatus.currDeviation) faults |= CARSIDE_FAULT_CURR;
	if (carStatus.voltDeviation) faults |= CARSIDE_FAULT_VOLTM;

	if (carStatus.chargingEnabled) status |= CARSIDE_STATUS_CHARGE;
	if (carStatus.notParked) status |= CARSIDE_STATUS_NOTPARK;
	if (carStatus.chargingFault) status |= CARSIDE_STATUS_MALFUN;
	if (carStatus.contactorOpen) status |= CARSIDE_STATUS_CONTOP;
	if (carStatus.stopRequest) status |= CARSIDE_STATUS_CHSTOP;

	canMsgID = CARSIDE_CONTROL;
	canMsg[0] = 2; //tell EVSE we are talking 1.0 protocol
	canMsg[1] = lowByte(carStatus.targetVoltage);
	canMsg[2] = highByte(carStatus.targetVoltage);
	canMsg[3] = askingAmps;
	canMsg[4] = faults;
	canMsg[5] = status;
	canMsg[6] = settings.SOC;
	canMsg[7] = 0; //not used
	CAN.sendMsgBuf(canMsgID, 0, 8, canMsg);
	if (chademoState == RUNNING &&  askingAmps < carStatus.targetCurrent) askingAmps++;
	//not a typo. We're allowed to change requested amps by +/- 20A per second. We send the above frame every 100ms so a single
	//increment means we can ramp up 10A per second. But, we want to ramp down quickly if there is a problem so do two which
	//gives us -20A per second.
	if (chademoState != RUNNING && askingAmps > 0) askingAmps--;
	if (askingAmps > carStatus.targetCurrent) askingAmps--;
	if (askingAmps > carStatus.targetCurrent) askingAmps--;
}
