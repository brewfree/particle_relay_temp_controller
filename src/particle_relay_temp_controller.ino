#include <Particle.h>
#include "Controller.h"
#include "Display.h"

// struct to persist controllers state in EEPROM
struct ControllerSettings {
  uint8_t version;
  bool fermenter1On;
	float fermenter1TargetTemp;
	bool fermenter2On;
	float fermenter2TargetTemp;
};

// controllers
Controller ambientController("Ambient", A0, NULL_PIN, NULL_PIN, false); // only reads temps
Controller glycolController("Glycol", A1, NULL_PIN, NULL_PIN, false); // only reads temps
Controller fermenter1Controller("Brew Bucket", A2, D3, D4, false);
Controller fermenter2Controller("Conical", A3, D5, D6, false);
Display display;

// particle variables
String ambient = "";
String glycol = "";
String fermenter1Target = "";
String fermenter1 = "";
String fermenter1Status = "";
String fermenter2Target = "";
String fermenter2 = "";
String fermenter2Status = "";
String fermenter1GapCount = "";

int setFermenter1(String command) {
    int result = fermenter1Controller.control(command);
    saveSettings();
    return result;
}

int setFermenter2(String command) {
    int result = fermenter2Controller.control(command);
    saveSettings();
    return result;
}

int fakeTemp1(String command) {
    fermenter1Controller.setCurrentTemp(command.toFloat());
    return 0;
}

ControllerSettings loadSettings() {
    ControllerSettings settings;

    EEPROM.get(0, settings);

    // default settings
    if(settings.version != 0) {
        settings.version = 0;
        settings.fermenter1TargetTemp = TARGET_TEMP_DEFAULT;
        settings.fermenter2TargetTemp = TARGET_TEMP_DEFAULT;
        settings.fermenter1On = false;
        settings.fermenter2On = false;
    }

    return settings;
}

void saveSettings() {
    ControllerSettings settings;

    settings.version = 0;
    settings.fermenter1On = fermenter1Controller.getOnState();
    settings.fermenter1TargetTemp = fermenter1Controller.getTargetTemp();
    settings.fermenter2On = fermenter2Controller.getOnState();
    settings.fermenter2TargetTemp = fermenter2Controller.getTargetTemp();

    EEPROM.put(0, settings);
}

void update() {
    ambientController.update();
    ambient = ambientController.getCurrentTempFormatted();

    glycolController.update();
    glycol = glycolController.getCurrentTempFormatted();

    fermenter1Controller.update();
    fermenter1 = fermenter1Controller.getCurrentTempFormatted();
    fermenter1Target = fermenter1Controller.getTargetTempFormatted();
    fermenter1Status = fermenter1Controller.getStateFormatted();
    fermenter1GapCount = fermenter1Controller.getGapCount();

    fermenter2Controller.update();
    fermenter2 = fermenter2Controller.getCurrentTempFormatted();
    fermenter2Target = fermenter2Controller.getTargetTempFormatted();
    fermenter2Status = fermenter2Controller.getStateFormatted();

    display.update(fermenter1Controller);
}

void setup() {

    // setup variables
    Particle.variable("ambient", ambient);
    Particle.variable("glycol", glycol);
    Particle.variable("ferm1Status", fermenter1Status);
    Particle.variable("ferm1Target", fermenter1Target);
    Particle.variable("ferm1", fermenter1);
    Particle.variable("ferm2Status", fermenter2Status);
    Particle.variable("ferm1GapCtr", fermenter1GapCount);
    Particle.variable("ferm2Target", fermenter2Target);
    Particle.variable("ferm2", fermenter2);

    // setup functions
    Particle.function("setFerm1", setFermenter1);
    Particle.function("setFerm2", setFermenter2);
    Particle.function("fakeTemp1", fakeTemp1);

    // load settings from EEPROM
    ControllerSettings settings = loadSettings();

    // initialize the control settings
    ambientController.setOnState(true);
    glycolController.setOnState(true);
    fermenter1Controller.setOnState(settings.fermenter1On);
    fermenter1Controller.setTargetTemp(settings.fermenter1TargetTemp);
    fermenter2Controller.setOnState(settings.fermenter2On);
    fermenter2Controller.setTargetTemp(settings.fermenter2TargetTemp);
}

void loop() {
    update();
    delay(UPDATE_FREQUENCY);
}
