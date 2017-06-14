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
const float SENSOR_MINF = -67.0;
const float SENSOR_MAXF = 257.0;

// min/max fermenter target temperature
const float TARGET_TEMP_MIN = 28.0;
const float TARGET_TEMP_MAX = 80.0;
const float TARGET_TEMP_DEFAULT = 65.0;

// temperature tolerance +-
const float TARGET_TEMP_TOLERANCE = 1;

// maximum number of gaps between current and target before changing state
const float GAP_COUNT_MAX = 12;

enum ControllerState { Off, Idle, Heat, Cool };

class Controller {
    public:
        Controller(String name, int sensorPin, int heaterPin, int coolerPin);
        String getName();
        void setOnState(bool on);
        bool getOnState();
        ControllerState getState();
        String getStateFormatted();
        void setCurrentTemp(float tempF); // temporary
        float getCurrentTemp();
        String getCurrentTempFormatted();
        int setTargetTemp(float tempF);
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
        ControllerState _state;
        int _gapCount;
        boolean isFloat(String tString);
        float readSensor();
        bool isGoodReading(float tempF);
        void setState(ControllerState state);
        String formatState(ControllerState state);
        String formatTemp(float tempF);
};
