/*
 * Copyright (c) 2004-2005 Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * MIT grants permission to use, copy, modify, and distribute this software and
 * its documentation for NON-COMMERCIAL purposes and without fee, provided that
 * this copyright notice appears in all copies.
 *
 * MIT provides this software "as is," without representations or warranties of
 * any kind, either expressed or implied, including but not limited to the
 * implied warranties of merchantability, fitness for a particular purpose, and
 * noninfringement.  MIT shall not be liable for any damages arising from any
 * use of this software.
 *
 * Author: Alexandr Andoni (andoni@mit.edu), Piotr Indyk (indyk@mit.edu)
 */
#ifndef GLOBALVARS_INCLUDED
#define GLOBALVARS_INCLUDED
#ifndef GLOBALVARS_CPP
#define DECLARE_EXTERN extern
#define EXTERN_INIT(x)
#else
#define DECLARE_EXTERN
#define EXTERN_INIT(x) x
#endif
DECLARE_EXTERN TimeVarT timeRNNQuery;
DECLARE_EXTERN BooleanT timingOn EXTERN_INIT(= 1);
DECLARE_EXTERN TimeVarT timevSpeed EXTERN_INIT(= 0);
DECLARE_EXTERN int nOfDistComps EXTERN_INIT(= 0);
DECLARE_EXTERN MemVarT totalAllocatedMemory EXTERN_INIT(= 0);

//Junho's code
DECLARE_EXTERN int nOfIO EXTERN_INIT(= 0);
DECLARE_EXTERN int nOfVisistingBuckets EXTERN_INIT(= 0);
DECLARE_EXTERN TimeVarT timeCollisionCount;
DECLARE_EXTERN TimeVarT timeSearchBucket;
DECLARE_EXTERN TimeVarT candidateTime;

// The data set containing all the points.
DECLARE_EXTERN PPointT *dataSetPoints EXTERN_INIT(= NULL);

// Number of points in the data set.
DECLARE_EXTERN int nPoints EXTERN_INIT(= 0);

// The dimension of the points.
DECLARE_EXTERN int pointsDimension EXTERN_INIT(= 0);

// The max coordinate value.
DECLARE_EXTERN double maxCoordinate EXTERN_INIT(= -2147483647);

// The min coordinate value.
DECLARE_EXTERN double minCoordinate EXTERN_INIT(= 2147483647);

// The max length of the vectors (data points).
DECLARE_EXTERN double maxSqrLength EXTERN_INIT(= 0);

// The min length of the vectors (data points).
DECLARE_EXTERN double minSqrLength EXTERN_INIT(= 2147483647);

// Just a buffer
DECLARE_EXTERN char sBuffer[100];

// Page size
DECLARE_EXTERN int PAGESIZE EXTERN_INIT(= 4096);

// The specified IO times.
DECLARE_EXTERN int MAXIO EXTERN_INIT(= 0);

DECLARE_EXTERN  int SizeIntT EXTERN_INIT(= sizeof(int));
DECLARE_EXTERN  int SizeRealT EXTERN_INIT(= sizeof(double));
DECLARE_EXTERN  int SizeBool EXTERN_INIT(= sizeof(bool));
#endif
