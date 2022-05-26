/*
 * CollisionCountLSH.h
 *
 *  Created on:  2011-03-12
 *  Modified on: 2012-06-07
 *      Author: Junhao Gan
 */

#ifndef COLLISIONCOUNTLSH_H_
#define COLLISIONCOUNTLSH_H_

#include <fstream>
#include <vector>
#include <algorithm>
// #include <ext/hash_map>
#include <unordered_map>
using namespace __gnu_cxx;
using namespace std;

#include "C2LSH_Initializor.h"

// The result item struct
typedef struct _ResultItem {
	// Point ID
	int pointID;

	// The real distance from query q
	double distanceFromQuery;

	_ResultItem() {
	}

	bool operator <(const _ResultItem& temp) const {
		if (distanceFromQuery < temp.distanceFromQuery)
			return true;
		else if (distanceFromQuery == temp.distanceFromQuery) {
			return pointID < temp.pointID;
		} else
			return false;
	}

	bool operator >(const _ResultItem& temp) const {
		if (distanceFromQuery > temp.distanceFromQuery)
			return true;
		else if (distanceFromQuery == temp.distanceFromQuery) {
			return pointID > temp.pointID;
		} else
			return false;
	}

} ResultItem, *PResultItemT;

// C2LSH algorithm structure
typedef struct _C2Structure {

	// Pointer to initializor
	PC2LSH_InitializorT initializor;

	// The radius value in last round.
	double lastRadius;

	// The current radius value.
	double currentRadius;

	// The current <c> * <R> value
	double currentCR;

	// <k> hash values corresponding to <k> hash tables of query q.
	int* queryHashValueList;

	// Candidate ID list
	vector<int> resultIDList;

	// Result item ID list
	vector<ResultItem> resultList;

	_C2Structure(PC2LSH_InitializorT _initializor) {
		initializor = _initializor;
	}

} C2Structure, *PC2StructureT;

// Initialize
void Initialize(PC2StructureT collisionCountStruct);

// Project query point q into <k> hash tables.
int* ProjectQueryPoint(PC2StructureT collisionCountStruct, PPointT q);

// Update Radius.
void UpdateRadius(PC2StructureT collisionCountStruct);

// Return the index of the smallest value no less than the target value.
// If the value in the list, then return this value's index.
int UpperBound(vector<OffsetItem> &list, int s, int t, int targetValue);

// Return the index of the largest value no more than the target value.
// If the value in the list, then return this value's index.
int LowerBound(vector<OffsetItem> &list, int s, int t, int targetValue);

///*
// * Collision Count的函数(数组统计版本)(hash_map或map)
// * 返回在cr距离内的对象个数
// */
//int CollisionCount_Array(PC2StructureT collisionCountStruct, PPointT q, int nNNs = 10);


#endif /* COLLISIONCOUNTLSH_H_ */
