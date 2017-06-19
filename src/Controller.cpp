#include <Particle.h>
#include <OneWire.h>
#include "Controller.h"

Controller::Controller(String name, int sensorPin, int heaterPin, int coolerPin, bool celcius) : _sensor(sensorPin) {
     _sensorPin = sensorPin;
     _heaterPin = heaterPin;
     _coolerPin = coolerPin;

     _name = name;
     _currentTemp = ERR_SENSOR;
     _targetTemp = TARGET_TEMP_DEFAULT;
     _isAlarm = false;
     _celcius = celcius;
     _gapCount = 0;
     _adjustCount = 0;

     // initialize temp sensor pin
     if(_sensorPin != NULL_PIN) {
         pinMode(_sensorPin, INPUT);
     }

     // initilize heater relay control pin
     if(_heaterPin != NULL_PIN) {
        pinMode(_heaterPin, OUTPUT);
     }

     // initialize cooler relay control pin
     if(_coolerPin != NULL_PIN) {
        pinMode(_coolerPin, OUTPUT);
     }
}

// get the controller display name
String Controller::getName() {
    return _name;
}

// turns the controller on/off
void Controller::setOnState(bool on) {
    setState(on ? Idle : Off);
}

// returns true if the controller is on; false otherwise
bool Controller::getOnState() {
    return _state != Off;
}

// gets the current temperature
float Controller::getCurrentTemp() {
    return _currentTemp;
}

// gets the current temperature as a formatted string
String Controller::getCurrentTempFormatted() {
    return formatTemp(_currentTemp);
}

// this is a temporary method until my parts arrive
void Controller::setCurrentTemp(float temp) {
    _currentTemp = temp;
}

// returns true if the controller is in an alarm state
bool Controller::getIsAlarm() {
  return _isAlarm;
}

// sets the target temperature
int Controller::setTargetTemp(float temp) {
    if(!isValidTemp(temp)) {
        return -1;
    }
    _targetTemp = temp;
    return 0;
}

// gets the target temperature
float Controller::getTargetTemp() {
    return _targetTemp;
}

// gets the target temperature as a formatted string
String Controller::getTargetTempFormatted() {
    return formatTemp(_targetTemp);
}

// gets the current state of the controller
ControllerState Controller::getState() {
    return _state;
}

// gets the current state of the controller as a formatted string
String Controller::getStateFormatted() {
    return formatState(_state);
}

// gets the next state
ControllerState Controller::getNextState() {
    return _nextState;
}

// gets the next state as a formatted string
String Controller::getNextStateFormatted() {
  String nextStateFormatted = formatState(_nextState);
  if(_gapCount == 0) {
    return nextStateFormatted;
  }

  int nextStateInSecs = (GAP_COUNT_MAX - _gapCount) * (UPDATE_FREQUENCY / 1000);
  String s = String::format(" IN %dS", nextStateInSecs);
  return nextStateFormatted + s;
}

// gets the count of the number of updates where the temp is out of tolerance
int Controller::getGapCount() {
    return _gapCount;
}

// call this method in a loop
void Controller::update() {
    //_currentTemp = readSensor(); commented out until my hardware arrives

    if((_state != Off) && isValidTemp(_currentTemp)) {

        // calculate temp range +- tolerance
        float minTemp = _targetTemp - TARGET_TEMP_TOLERANCE;
        float maxTemp = _targetTemp + TARGET_TEMP_TOLERANCE;

        // is the current temp out of tolerance?
        if(_currentTemp < minTemp || _currentTemp > maxTemp) {

            // if in idle state (not heating or cooling)
            if(_state == Idle) {

                // next state is cool
                if(_currentTemp > maxTemp) {
                    _nextState = Cool;
                }

                // next state is heat
                if(_currentTemp < minTemp) {
                    _nextState = Heat;
                }

                _gapCount++; // increase the gap counter

                // adjust to correct gap
                if(_gapCount >= GAP_COUNT_MAX) {

                  // set the next state
                  setState(_nextState);

                    // reset gap counter
                    _gapCount = 0;
                }
            }
            else {
              // keep track how long we've been heating or cooling
              _adjustCount++;
              _nextState = Idle;
            }
        }
        else { // the current temperature is in tolerance
            setState(Idle); // within range, set to idle
            _nextState = Idle;
            _gapCount = 0;
            _adjustCount = 0;
        }
    }
}

// private methods

// convert to F
float Controller::convertToF(float tempC) {
  return (tempC * 1.8) + 32.0;
}

// convert to C
float Controller::convertToC(float tempF) {
  return (tempF - 32) * 0.55;
}

// reads the ds18b20 temp sensor
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

    // if in F mode, convert to F
    float temp = _celcius ? tempC : convertToF(tempC);

    if(!isValidTemp(temp)) {
        return ERR_READING;
    }

    return temp;
}

// formats the specified temperature
String Controller::formatTemp(float temp) {
    if(temp == ERR_SENSOR) {
        return "NO SENSOR";
    }
    if(temp == ERR_CRC) {
        return "INVALID CRC";
    }
    if(temp == ERR_DEVICE) {
        return "DEVICE ERROR";
    }
    if(temp == ERR_READING) {
        return "BAD READING";
    }
    return String::format("%0.1f%1s", temp, _celcius ? "C" : "F");
}

// returns true if the temp is valid
bool Controller::isValidTemp(float temp) {
  float minTemp = _celcius ? SENSOR_MINC : convertToF(SENSOR_MINC);
  float maxTemp = _celcius ? SENSOR_MAXC : convertToF(SENSOR_MAXC);
  return ((temp > minTemp) && (temp < maxTemp));
}

// sets the state of the controller
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

// formats the specified state
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

// convenience method for controlling with a particle function
// you can pass in "ON" or "IDLE" to turn on or "OFF" to turn OFF
// you can set the target temp by passing in a temperature
int Controller::control(String command) {
  // turn on
  if(command.toUpperCase() == "IDLE" || command.toUpperCase() == "ON") {
      setState(Idle);
      return 0;
  }

  // turn off
  if(command.toUpperCase() == "OFF") {
      setState(Off);
      return 0;
  }

  // invalid command
  if(!isFloat(command)) {
    return -1;
  }

  // set target temp
  float newSetTemp = command.toFloat();
  if(!isValidTemp(newSetTemp)) {
    return -1;
  }

  // set the temp
  setTargetTemp(newSetTemp);
  return 0;
}

// returns true if the specified string is a float
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
