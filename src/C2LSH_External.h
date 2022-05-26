/*
 * CollisionCountLSH_Radius.h
 *
 *  Created on:  2011-10-05
 *  Modified on: 2012-06-07
 *      Author: Junhao Gan
 */

#ifndef COLLISIONCOUNTLSH_RADIUS_H_
#define COLLISIONCOUNTLSH_RADIUS_H_

#include "headers.h"

// The version of with new C1 (Condition 1) which calculates the real distance <= cR.
int CollisionCount_External(PC2StructureT collisionCountStruct, PPointT q, int nNNs);

//// Update the left and right pointers in this round.
//void UpdateClips(PC2LSH_InitializorT initializor, int basicIntervalID, double radius, int dimensionIndex);

#endif /* COLLISIONCOUNTLSH_RADIUS_H_ */
