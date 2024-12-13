#include "Cmd.h"
#include "Serial.h"
#include "Config.h"
#include "LED.h"
#include "Timer.h"

extern uint8_t g_running;
extern uint8_t g_amplitude;
extern uint8_t g_frequency;

Command commands[7] = {
	{"START", Start},
	{"STOP", Stop},
	{"AMPL+", AmplPlus},
	{"AMPL-", AmplMinus},
	{"FREQ+", FreqPlus},
	{"FREQ-", FreqMinus},
};

void Start(void){
    g_running = 1;
    Serial_SendString("START_OK\r\n");
}

void Stop(void){
    g_running = 0;
    Serial_SendString("STOP_OK\r\n");
}

void AmplPlus(void){
    if(g_running){
        g_amplitude += 10;
        if(g_amplitude > 200){
            g_amplitude = 200;
        }
        GenerateSineTable(g_amplitude);
        Serial_SendString("AMPL+_OK\r\n");
    }
}

void AmplMinus(void){
    if(g_running){
        g_amplitude -= 10;
        if(g_amplitude > 200){
            g_amplitude = 0;
        }
        GenerateSineTable(g_amplitude);
        Serial_SendString("AMPL-_OK\r\n");
    }
}

void FreqPlus(void){
    if(g_running){
        g_frequency += 1;
        if(g_frequency > 10){
            g_frequency = 10;
        }
        Timer_Init(2000/g_frequency,720);
        Serial_SendString("FREQ+_OK\r\n");
    }
}

void FreqMinus(void){
    if(g_running){
        g_frequency -= 1;
        if(g_frequency > 10){
            g_frequency = 1;
        }
        Timer_Init(2000/g_frequency,720);
        Serial_SendString("FREQ-_OK\r\n");
    }
}
