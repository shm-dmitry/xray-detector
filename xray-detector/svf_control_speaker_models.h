#ifndef SVF_CONTROL_SPEAKER_MODELS_H_
#define SVF_CONTROL_SPEAKER_MODELS_H_

const uint16_t SVF_CONTROL_SPEAKER_1[] PROGMEM = {1, 500, 200};
const uint16_t SVF_CONTROL_SPEAKER_2[] PROGMEM = {5, 300, 500, 100, 500, 300, 500, 100, 500, 300, 500};
const uint16_t SVF_CONTROL_SPEAKER_3[] PROGMEM = {11, 600, 500, 300, 500, 600, 500, 300, 500, 600, 500, 300, 500, 600, 500, 300, 500, 600, 500, 300, 500, 600, 500};
const uint16_t SVF_CONTROL_SPEAKER_4[] PROGMEM = {3, 200, 10000, 500, 10000, 200, 10000};

const uint16_t* const SVF_CONTROL_MODELS_SPEAKER[] = {
  SVF_CONTROL_SPEAKER_1,
  SVF_CONTROL_SPEAKER_2,
  SVF_CONTROL_SPEAKER_3,
  SVF_CONTROL_SPEAKER_4
};

#endif /* NVF_CONTROL_SPEAKER_MODELS_ */
