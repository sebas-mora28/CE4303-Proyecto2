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
            return 0;
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
        return res_freq;
    }
    return 0;
}


#define STX '@'
#define EOT ';'

typedef uint8_t pin;

typedef struct {
    pin step;
    pin dir;
}channel_t;

channel_t steppers[] = {
    {
        .step = D10, // GPIO10 
        .dir = D9 // GPIO9
    },
    {
        .step =  D8, //GPIO8 
        .dir =  D7 //GPIO20
    },
    {
        .step =  D5, // GPIO7 
        .dir = D6 // GPIO21
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
    if (channel >= ((sizeof(steppers)/sizeof(channel_t)))){
        Serial.printf(err_msg);
        return;
    }
    uint32_t freq = atoll(fstr); 
    if(WriteTone(steppers[channel].step, freq)){
        Serial.printf("@%d%04"PRIu32";", channel, freq);
    }else{
        Serial.printf(err_msg);
    }
}


void setup() {
  Serial.begin(921600);
  for (auto stepper: steppers){
    pinMode(stepper.step, OUTPUT);
    pinMode(stepper.dir, OUTPUT);
    analogWrite(stepper.step, 0);
  }
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
