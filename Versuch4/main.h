#define POT BIT5
#define LDR BIT4
#define ANALOG_IN (POT | LDR)

#define LEDR BIT0
#define LEDG BIT1
#define LEDB BIT2
#define LEDS (LEDR | LEDG | LEDB)

#define P2_OUT 0x7F
#define P2_IN 0x80

#define ADC_VALUES 4

#define COLORS 5

// Color names which are printed out to the console
static const char* COLOR_NAMES[COLORS + 1] = {
  "none",
  "white",
  "black",
  "red",
  "green",
  "blue"
};

// Color components averaged from 3 measurements per color
static const uint16_t COLOR_COMPONENTS[COLORS][3] = {
  { 200, 240, 240 }, // White
  { 80, 100, 115 }, // Black
  { 180, 140, 130 }, // Red
  { 90, 160, 125 }, // Green
  { 90, 130, 180 } // Blue
};

// Maximum squared distance between color and measured color
static const uint16_t COLOR_DIFF_THRESHOLD = 10;

// Number of measurement points to switch to new color
static const uint16_t COLOR_DETECT_THRESHOLD = 3;

static const uint16_t ADC_CHANNEL[ADC_VALUES] = {
  INCH_4, INCH_4, INCH_4, INCH_5
};

static const uint8_t ADC_LEDS[ADC_VALUES] = {
  LEDR, LEDG, LEDB, 0
};

void setup(void);
void loop(void);

__inline void process_analog_value(uint8_t index, uint16_t value);
__inline void adc_convert(uint8_t index);

void identify_color(void);
void report_new_color(uint8_t index);

__inline void set_shift_register_leds(uint8_t state);
__inline void shift_register_clock(void);

#define ABS(x) (x > 0 ? x : -x)
