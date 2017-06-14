#include <Particle.h>
#include <OneWire.h>
#include "Controller.h"

Controller::Controller(String name, int sensorPin, int heaterPin, int coolerPin) : _sensor(sensorPin) {
     _sensorPin = sensorPin;
     _heaterPin = heaterPin;
     _coolerPin = coolerPin;

     _name = name;
     _currentTemp = ERR_SENSOR;
     _targetTemp = TARGET_TEMP_DEFAULT;
     _gapCount = 0;

     // initialize sensor pin
     if(_sensorPin != NULL_PIN) {
         pinMode(_sensorPin, INPUT);
     }

     // initilize relay control pins
     if(_heaterPin != NULL_PIN) {
        pinMode(_heaterPin, OUTPUT);
     }

     if(_coolerPin != NULL_PIN) {
        pinMode(_coolerPin, OUTPUT);
     }
}

String Controller::getName() {
    return _name;
}

float Controller::readSensor() {
    byte data[12];
    byte addr[8];

    if (!_sensor.search(addr)) {
        _sensor.reset_search(); //no more sensors on chain, reset search
        return ERR_SENSOR;
    }

    if (OneWire::crc8(addr, 7) != addr[7]) {
        return ERR_CRC;
    }

    if (addr[0] != 0x10 && addr[0] != 0x28) {
        return ERR_DEVICE;
    }

    _sensor.reset();
    _sensor.select(addr);
    _sensor.write(0x44,1); // start conversion, with parasite power on at the end

    byte present = _sensor.reset();
    _sensor.select(addr);
    _sensor.write(0xBE); // read scratchpad

    for (int i = 0; i < 9; i++) // we need 9 bytes
    {
        data[i] = _sensor.read();
    }

    _sensor.reset_search();

    byte MSB = data[1];
    byte LSB = data[0];

    int tempTemp = ((MSB << 8) | LSB); // using two's compliment

    if (MSB&0x80) {
	    tempTemp = tempTemp | 0xFFFF0000;
  	} else {
  	    tempTemp = tempTemp & 0x0000FFFF;
  	}

	  long longTemp = tempTemp;		// should sign extend

    float tempRead = longTemp;		// convert to floating

    float tempC = tempRead/16.0;

    float tempF = tempC * 1.8 + 32.0; // convert to F

    if(tempF < SENSOR_MINF || tempF > SENSOR_MAXF) {
        return ERR_READING;
    }

    return tempF;
}

float Controller::getCurrentTemp() {
    return _currentTemp;
}

void Controller::setCurrentTemp(float tempF) {
    _currentTemp = tempF;
}

String Controller::getCurrentTempFormatted() {
    return formatTemp(_currentTemp);
}

int Controller::setTargetTemp(float tempF) {
    if(tempF < TARGET_TEMP_MIN || tempF > TARGET_TEMP_MAX) {
        return -1;
    }
    _targetTemp = tempF;
    return 0;
}

float Controller::getTargetTemp() {
    return _targetTemp;
}

String Controller::getTargetTempFormatted() {
    return formatTemp(_targetTemp);
}

String Controller::formatTemp(float tempF) {
    if(tempF == ERR_SENSOR) {
        return "No sensor";
    }
    if(tempF == ERR_CRC) {
        return "CRC not valid";
    }
    if(tempF == ERR_DEVICE) {
        return "Device not recognized";
    }
    if(tempF == ERR_READING) {
        return "Bad reading";
    }
    return String::format("%0.1f°F", tempF);
}

bool Controller::isGoodReading(float tempF) {
    return ((tempF > SENSOR_MINF) && (tempF < SENSOR_MAXF));
}

void Controller::update() {
    _currentTemp = readSensor();

    if((_state != Off) && isGoodReading(_currentTemp)) {

        // calculate temp range +- tolerance
        float minTemp = _targetTemp - TARGET_TEMP_TOLERANCE;
        float maxTemp = _targetTemp + TARGET_TEMP_TOLERANCE;

        // is the current temp out of tolerance?
        if(_currentTemp < minTemp || _currentTemp > maxTemp) {

            // if in idle state (not heating or cooling)
            if(_state == Idle) {
                _gapCount++; // increase the gap counter

                // take action to correct gap
                if(_gapCount >= GAP_COUNT_MAX) {

                    if(_currentTemp > maxTemp) {
                        setState(Cool);
                    }

                    if(_currentTemp < minTemp) {
                        setState(Heat);
                    }

                    // reset gap counter
                    _gapCount = 0;
                }
            }
        }
        else {
            setState(Idle); // within rang, set to idle
            _gapCount = 0;
        }
    }
}

void Controller::setState(ControllerState state) {
    _state = state;

    switch(state) {
        case Off:
        case Idle:
            digitalWrite(_coolerPin, LOW);
            digitalWrite(_heaterPin, LOW);
            break;
        case Heat:
            digitalWrite(_coolerPin, LOW);
            digitalWrite(_heaterPin, HIGH);
            break;
        case Cool:
            digitalWrite(_coolerPin, HIGH);
            digitalWrite(_heaterPin, LOW);
            break;
    }
}

ControllerState Controller::getState() {
    return _state;
}

String Controller::getStateFormatted() {
    return formatState(_state);
}

void Controller::setOnState(bool on) {
    setState(on ? Idle : Off);
}

bool Controller::getOnState() {
    return _state != Off;
}

String Controller::formatState(ControllerState state) {
    switch(state) {
        case Off:
            return "Off";
            break;
        case Idle:
            return "Idle";
            break;
        case Heat:
            return "Heating";
            break;
        case Cool:
            return "Cooling";
            break;
        default:
            return "";
            break;
    }
}

int Controller::getGapCount() {
    return _gapCount;
}

// convenience method for controlling via particle
int Controller::control(String command) {
    if(command.toUpperCase() == "IDLE" || command.toUpperCase() == "ON") {
        setState(Idle);
        return 0;
    }

    if(command.toUpperCase() == "OFF") {
        setState(Off);
        return 0;
    }

    if(isFloat(command)) {
        float newSetTemp = command.toFloat();
        return setTargetTemp(newSetTemp);
    }

    return -1;
}

boolean Controller::isFloat(String tString) {
    String tBuf;
    boolean decPt = false;

    if(tString.charAt(0) == '+' || tString.charAt(0) == '-') {
        tBuf = &tString[1];
    }
    else {
        tBuf = tString;
    }

    for(int x=0;x<tBuf.length();x++)
    {
        if(tBuf.charAt(x) == '.') {
            if(decPt) {
                return false;
            }
            else {
                decPt = true;
            }
        }
        else if(tBuf.charAt(x) < '0' || tBuf.charAt(x) > '9') {
            return false;
        }
    }

    return true;
}
