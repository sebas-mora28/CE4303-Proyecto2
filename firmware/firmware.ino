#include <inttypes.h>
#include <stdlib.h>
#include "soc/soc_caps.h"
#include "esp32-hal.h"
#include "esp32-hal-ledc.h"
#include "driver/ledc.h"
#include "esp32-hal-periman.h"
#ifdef SOC_LEDC_SUPPORT_HS_MODE
#define LEDC_CHANNELS           (SOC_LEDC_CHANNEL_NUM<<1)
#else
#define LEDC_CHANNELS           (SOC_LEDC_CHANNEL_NUM)
#endif

//Use XTAL clock if possible to avoid timer frequency error when setting APB clock < 80 Mhz
//Need to be fixed in ESP-IDF
#ifdef SOC_LEDC_SUPPORT_XTAL_CLOCK
#define LEDC_DEFAULT_CLK        LEDC_USE_XTAL_CLK
#else
#define LEDC_DEFAULT_CLK        LEDC_AUTO_CLK
#endif

#define LEDC_MAX_BIT_WIDTH      SOC_LEDC_TIMER_BIT_WIDTH
uint32_t WriteTone(uint8_t pin, uint32_t freq)
{
    ledc_channel_handle_t *bus = (ledc_channel_handle_t*)perimanGetPinBus(pin, ESP32_BUS_TYPE_LEDC);
    if(bus != NULL){

        if(!freq){
            ledcWrite(pin, 0);
            return 1;
        }

        uint8_t group=(bus->channel/8), timer=((bus->channel/2)%4);
        ledc_timer_config_t ledc_timer = {
            .speed_mode       = (ledc_mode_t)group,
            .duty_resolution  = (ledc_timer_bit_t)12,
            .timer_num        = (ledc_timer_t)timer,
            .freq_hz          = freq, 
            .clk_cfg          = LEDC_DEFAULT_CLK
        };

        if(ledc_timer_config(&ledc_timer) != ESP_OK)
        {
            log_e("ledcWriteTone configuration failed!");
            return 0;
        }
        bus->channel_resolution = 12;

        uint32_t res_freq = ledc_get_freq((ledc_mode_t)group,(ledc_timer_t)timer);
        ledcWrite(pin, 0xFFF >> 1);
        return 1;
    }
    return 0;
}


#define STX '@'
#define EOT ';'

typedef uint8_t pin;

typedef struct {
    pin neg_enable;
    pin step;
}channel_t;

channel_t steppers[] = {
    {
        .neg_enable = D10, // GPIO9
        .step = D9, // GPIO10
    },
    {
        .neg_enable  = D8, // GPIO20
        .step =  D7, //GPIO8
    },
    {
        .neg_enable  = D5, //GPIO6
        .step =  D6, // GPIO21
    }
};

typedef enum{
    READ_STX,
    READ_CHN,
    READ_VAL,
    READ_EOT
}State;

char get_byte(){
    while(Serial.available()==0){
        continue;
    }
    return Serial.read();
}

char err_msg[] = "@ERROR;";
State st = READ_STX;
char cbyte;
char fstr[5] = "0000"; // freqency string
int fstr_idx = 0;
int fstr_idx_max = 4;
int channel = 0;

void apply_command(){

    /* 
    digitalWrite(steppers[0].neg_enable, LOW); // disable
    digitalWrite(steppers[1].neg_enable, LOW); // disable
    digitalWrite(steppers[2].neg_enable, LOW); // disable
    uint32_t notas[] = {
    // DO        RE        MI   FA        SOL       LA        SI
      16,   17,  18,  19,  20,  21,  23,  25,  26,  28,  29,  31, 
      33,   35,  37,  39,  41,  44,  46,  49,  52,  55,  58,  62, 
      65,   69,  73,  78,  82,  87,  93,  98, 104, 110, 117, 124,
      131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 245, 
      262, 277, 294, 311, 329, 349, 370, 392, 415, 440, 466, 493, 
      523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988,
      1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976, 
      2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951,
      4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902, 
    };
    uint32_t martillo[] = {
       392, 392, 440, 493, 493, 493, 440, 440, 440, 493, 392, 392, 392, 440, 493, 493, 493, 440, 440, 392, 392
    };

    for(auto nota: martillo){
      WriteTone(steppers[0].step, nota/2);
      WriteTone(steppers[1].step, nota/4);
      WriteTone(steppers[2].step, nota);
      delay(400);
      WriteTone(steppers[0].step, 0);
      WriteTone(steppers[1].step, 0);
      WriteTone(steppers[2].step, 0);
      delay(1);

    }
    digitalWrite(steppers[0].neg_enable, HIGH); // disable
    digitalWrite(steppers[1].neg_enable, HIGH); // disable
    digitalWrite(steppers[2].neg_enable, HIGH); // disable

    ///fin prueba */

    if (channel >= ((sizeof(steppers)/sizeof(channel_t)))){
        Serial.printf(err_msg);
        return;
    }
    uint32_t freq = atoll(fstr); 
    digitalWrite(steppers[channel].neg_enable, HIGH); // disable
    if(WriteTone(steppers[channel].step, freq)){
        if(freq != 0){
          digitalWrite(steppers[channel].neg_enable, LOW); // disable
        }
        Serial.printf("@%d%04"PRIu32";", channel, freq);
    }else{
        Serial.printf(err_msg);
    }
}


void setup() {
  Serial.begin(921600);
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  for (auto stepper: steppers){
    pinMode(stepper.step, OUTPUT);
    pinMode(stepper.neg_enable, OUTPUT);
    //analogWrite(stepper.step, 0);
    digitalWrite(stepper.neg_enable, HIGH);
  }
  analogWrite(D0, 0);
  analogWrite(steppers[0].step, 0);
  //analogWrite(D1, 0);
  analogWrite(steppers[1].step, 0);
  analogWrite(D1, 0);
  analogWrite(steppers[2].step, 0);

}

void loop() {
    cbyte = get_byte();
    switch(st){
        case READ_STX:
            if (cbyte != STX){
               break;
            }else{
                st = READ_CHN;
            }
            break;
        case READ_CHN:
            if(cbyte != STX){
                channel = cbyte - '0';
                st = READ_VAL;
                fstr_idx = 0;
            }
            break;
        case READ_VAL: 
            if(cbyte == STX){
                st = READ_CHN;
            }
            else if(cbyte == EOT){
                st = READ_STX;
                for(int i = fstr_idx; i < fstr_idx_max; i++){
                    fstr[i] = '\0';
                }
                apply_command();
                st = READ_STX;
            }else{
                fstr[fstr_idx] = cbyte;
                fstr_idx++;
                // evitar sobreescribir el caracter nulo
                if (fstr_idx == fstr_idx_max){
                    fstr[fstr_idx_max] = 0;
                    st = READ_EOT;
                }
            }
            break;
        case READ_EOT:
            if(cbyte == STX){
                st = READ_CHN;
            }
            else if(cbyte !=EOT){
                Serial.printf(err_msg);    
                st = READ_STX;
            }else{
                apply_command();
                st = READ_STX;
            }
            break;
        default:
            Serial.println("Bitflip galático mieo");
            st = READ_STX;
            break;
    }
}
