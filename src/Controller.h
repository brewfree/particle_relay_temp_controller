#ifndef Controller_h
#define Controller_h

#include <Particle.h>
#include <OneWire.h>

// pins
const int NULL_PIN = -1;

// sensor errors
const float ERR_SENSOR = -1000;
const float ERR_CRC = -1001;
const float ERR_DEVICE = -1002;
const float ERR_READING = -1003;

// stated range on the ds18b20 datasheet
const float SENSOR_MINC = -55.0;
const float SENSOR_MAXC = 125;

// min/max fermenter target temperature
const float TARGET_TEMP_MIN = 28.0;
const float TARGET_TEMP_MAX = 80.0;
const float TARGET_TEMP_DEFAULT = 65.0;

// temperature tolerance +-
const float TARGET_TEMP_TOLERANCE = 1;
const float TARGET_TEMP_ALARM_TOLERANCE = 10;

// maximum number of gaps between current and target before changing state
const int GAP_COUNT_MAX = 12;

// maximumn number of updates controller can be heating or cooling
const int ADJUST_COUNT_MAX = 24;

// how often to update in MS
const int UPDATE_FREQUENCY = 5000;

enum ControllerState { Off, Idle, Heat, Cool };

class Controller {
    public:
        Controller(String name, int sensorPin, int heaterPin, int coolerPin, bool celcius);
        String getName();
        void setOnState(bool on);
        bool getOnState();
        ControllerState getState();
        String getStateFormatted();
        ControllerState getNextState();
        String getNextStateFormatted();
        void setCurrentTemp(float temp); // temporary
        float getCurrentTemp();
        String getCurrentTempFormatted();
        bool getIsAlarm();
        int setTargetTemp(float temp);
        float getTargetTemp();
        String getTargetTempFormatted();
        int getGapCount();
        void update();
        int control(String command);
    private:
        String _name;
        int _sensorPin;
        int _heaterPin;
        int _coolerPin;
        OneWire _sensor;
        float _currentTemp;
        float _targetTemp;
        bool _isAlarm;
        bool _celcius;
        ControllerState _state;
        ControllerState _nextState;
        int _gapCount;
        int _adjustCount;
        boolean isFloat(String tString);
        float readSensor();
        bool isValidTemp(float temp);
        void setState(ControllerState state);
        String formatState(ControllerState state);
        String formatTemp(float temp);
        float convertToF(float tempC);
        float convertToC(float tempF);
};

#endif
