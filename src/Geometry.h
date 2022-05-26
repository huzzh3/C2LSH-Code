/*
 * DataProcessor.h
 *
 *  Created on: 2011-9-26
 *      Author: Junhao Gan
 */

#ifndef GEOMETRY_INCLUDED
#define GEOMETRY_INCLUDED

// A simple point in d-dimensional space. A point is defined by a
// vector of coordinates.
typedef struct _PointT {
	int index; // the index of this point in the dataset list of points
	double *coordinates;
//	double sqrLength; // the square of the length of the vector
} PointT, *PPointT;

// Calculate the Euclidean distance between two points.
double distance(int dimension, PPointT p1, PPointT p2);

#endif
