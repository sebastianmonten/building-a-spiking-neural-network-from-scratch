#pragma once 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h> // for measuring time that the forward executiuon takes

////////////////////// GLOBAL CONSTANTS //////////////////////////////
const double RP = 0.1;   // resting potential
const double WTH = 0.01; // threshold for weights to count as connection
const double ATH = 1.0;  // threshold for membrane potential to spike
const double DT = 0.1;   // time step
const int MAX_COMPUTATIONS = 10000;
const double MAX_TIME = 7.0; // maximum time for simulation
const double TAU = 2.0;  // time constant
const double TIME_TOLERANCE = 0.0001; // tolerance for time comparisons




double*** weights;