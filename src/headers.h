/*
 * headers.h
 *
 *  Created on:  2011-03-12
 *  Modified on: 2012-06-07
 *      Author: Junhao Gan
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <fstream>
#include <algorithm>
using namespace __gnu_cxx;
using namespace std;

#include "BasicDefinitions.h"
#include "Geometry.h"
#include "Random.h"
#include "GlobalVars.h"
#include "DataProcessor.h"
#include "C2LSH_Initializor.h"
#include "CollisionCountLSH.h"
#include "C2LSH_External.h"
#include "C2LSH_RAM.h"

///** On OS X malloc definitions reside in stdlib.h */
//#ifdef DEBUG_MEM
//#ifndef __APPLE__
//#include <malloc.h>
//#endif
//#endif
//
#ifdef DEBUG_TIMINGS
#include <sys/time.h>
#endif
