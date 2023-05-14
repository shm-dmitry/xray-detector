#ifndef CONFIG_H_
#define CONFIG_H_

// simulate encoder wia 3-buttons
#define USERINPUT_ENCODER_SIMUL false

// display type
#define DISPLAY_TYOE_SIMUL_ADAFRUIT false
#define DISPLAY_TYPE_SIMIL_CUSTOM false
#define DISPLAY_TYPE_ST7335_ADAFRUIT false
#define DISPLAY_TYPE_ST7335_CUSTOM true

// Enable hign voltage generator
#define UV_HIGH_VOLTAGE_OUTPUT_ENABLED true
#define UV_LINEAR_APPROX_FREQ_ENABLED  true

// init fake accumulated data buffer on startup
#define EEPROM_CONTROL_INIT_FAKE_BUFFER false

#define CHARGER_CONTROL_ALLOW_WAKE_UP false

#endif /* CONFIG_H_ */
