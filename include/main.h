#pragma once 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h> // for int16_t
#include <time.h> // for measuring time that the forward executiuon takes

////////////////////// GLOBAL CONSTANTS //////////////////////////////
const float  RP = 0.1;   // resting potential
const int16_t WTH = 100; // threshold for weights to count as connection
const float ATH = 1.0;  // threshold for membrane potential to spike
const float DT = 0.1;   // time step
const int MAX_COMPUTATIONS = 10000;
const float MAX_TIME = 7.0; // maximum time for simulation
const float TAU = 2.0;  // time constant
const float TIME_TOLERANCE = 0.001; // tolerance for time comparisons




int16_t*** weights;