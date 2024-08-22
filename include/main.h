#pragma once 

////////////////////////// INCLUDES //////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h> // for int16_tS
#include <time.h> // for measuring time that the forward executiuon takes

////////////////////////// TYPE DEF //////////////////////////////////
typedef uint16_t neuron_idx_t;
typedef uint16_t neuron_mp_t;
typedef uint32_t neuron_ts_t;
typedef uint8_t weight_t;
typedef uint32_t my_time_t;
typedef float decimal_temporary_t;
typedef uint8_t input_t;

////////////////////// GLOBAL CONSTANTS //////////////////////////////
const neuron_mp_t RP = 10;   // resting potential
const weight_t WTH = 7; // threshold for weights to count as connection
const neuron_mp_t ATH = 255;  // threshold for membrane potential to spike
const my_time_t DT = 1;   // time step
const int MAX_COMPUTATIONS = 30;
const my_time_t MAX_TIME = 3000; // maximum time for simulation
const decimal_temporary_t TAU = 2;  // time constant