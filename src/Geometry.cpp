/*
 * DataProcessor.h
 *
 *  Created on: 2011-9-26
 *      Author: Junhao Gan
 */

#include "headers.h"

// Return the Euclidean distance from point <p1> to <p2>.
double distance(int dimension, PPointT p1, PPointT p2) {
	double result = 0;

	for (int i = 0; i < dimension; i++) {
		result += SQR(p1->coordinates[i] - p2->coordinates[i]);
	}

	return SQRT(result);
}
