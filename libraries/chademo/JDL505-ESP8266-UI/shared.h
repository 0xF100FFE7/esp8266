//#include <Arduino.h>

enum command_type {
	CMD_START = 4,
	
	CMD_BEGIN_CHARGE, CMD_END_CHARGE,
	CMD_EVSE_STATUS,
	CMD_RESET, CMD_SET_TARGET_VOLT, CMD_CAL_AMPS, CMD_CAL_VOLT, CMD_SET_CAPACITY, CMD_SET_MAX_VOLT, CMD_SET_MAX_ASKING_AMPS, CMD_IGNORE_VOLTAGE_MISMATCH, CMD_IGNORE_CURRENT_MISMATCH,
	CMD_VOLTAGE, CMD_CURRENT, CMD_AMPHOURS, CMD_POWER, CMD_KWH, CMD_SOC, CMD_UPDATE_STATUS, CMD_TIME_REMAINING, CMD_EMULATE_CHARGER,
	
	CMD_CAN_MODE, CMD_TEMP,
};

struct cmd {
	enum state {CS_RECORDING, CS_STOPPED, CS_TYPE} state = CS_STOPPED;
	char type;
	String buf;
	
	bool send(char type)
	{
		Serial.write(1);
		Serial.write(type);
		Serial.write(4);
	}
	
	template <typename T>
	bool send(char type, T message)
	{
		Serial.write(1);
		Serial.write(type);
		Serial.print(message);
		Serial.write(4);
	}
	
	bool get()
	{
		if (Serial.available() > 0) {
			char byte = Serial.read();
			switch (state) {
			case CS_RECORDING:
				if (byte == 4) {
					state = CS_STOPPED;
					return true;
				} else {
					buf += byte;
				}
				break;
				
			case CS_STOPPED:
				if (byte == 1) {
					state = CS_TYPE;
					buf = "";
				}
				break;
					
			case CS_TYPE:
				type = byte;
				state = CS_RECORDING;
				break;
			}
		}
		
		return false;
	}
};
