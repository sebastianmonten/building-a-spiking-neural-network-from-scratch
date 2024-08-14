#pragma once 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h> // for int16_tS
#include <time.h> // for measuring time that the forward executiuon takes

//////////////////////     TYPE DEF     //////////////////////////////
typedef uint16_t neuron_mp_t;
typedef uint32_t neuron_ts_t;
typedef uint16_t weight_t;

////////////////////// GLOBAL CONSTANTS //////////////////////////////
const uint16_t RP = 10;   // resting potential
const uint16_t WTH = 7; // threshold for weights to count as connection
const neuron_mp_t ATH = 255;  // threshold for membrane potential to spike
const uint16_t DT = 1;   // time step
const int MAX_COMPUTATIONS = 10000;
const neuron_ts_t MAX_TIME = 7.0; // maximum time for simulation
const float TAU = 3;  // time constant
const float TIME_TOLERANCE = 0.001; // tolerance for time comparisons



const uint16_t LAYER_SIZES[] = {2, 3, 2};
const uint16_t NUM_LAYERS = sizeof(LAYER_SIZES) / sizeof(LAYER_SIZES[0]);

neuron_mp_t* NEURONS_MP;
neuron_ts_t* NEURONS_TS;