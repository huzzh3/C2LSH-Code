/*
 * C2LSH_RAM.cpp
 *
 *  Created on: 2012-6-21
 *      Author: Junhao Gan
 */

#include "headers.h"

//更新夹子
void UpdateClips_RAM(PC2LSH_InitializorT initializor, int basicIntervalID, double radius, int dimensionIndex) {
	//计算第i维的偏移表的元素个数
	int offsetSize = initializor->offsetTableList[dimensionIndex].size();

	int endIntervalID = basicIntervalID + (int) radius - 1;

	int temp = UpperBound(initializor->offsetTableList[dimensionIndex], 0, offsetSize - 1, basicIntervalID);
	initializor->fileInfoList[dimensionIndex].left_end = initializor->offsetTableList[dimensionIndex][temp].offset;

	temp = LowerBound(initializor->offsetTableList[dimensionIndex], 0, offsetSize - 1, endIntervalID);
	if (temp == offsetSize - 1) {
//		initializor->fileInfoList[dimensionIndex].right_end = initializor->nPoints
//				- initializor->offsetTableList[dimensionIndex][temp].offset;
		initializor->fileInfoList[dimensionIndex].right_end = initializor->nPoints;
	} else {
		initializor->fileInfoList[dimensionIndex].right_end
				= initializor->offsetTableList[dimensionIndex][temp + 1].offset;
	}

}

//Find the appropriate radius so that the bucket with this radius value fits in a page.
void FindRadius_RAM(PC2StructureT collisionCountStruct) {

	PC2LSH_InitializorT initializor = collisionCountStruct->initializor;

	int basicIntervalID;

	while (true) {
		basicIntervalID = ((int) (collisionCountStruct->queryHashValueList[0] / collisionCountStruct->currentRadius))
				* collisionCountStruct->currentRadius;

		UpdateClips_RAM(initializor, basicIntervalID, collisionCountStruct->currentRadius, 0);

		if(initializor->fileInfoList[0].right_end - initializor->fileInfoList[0].left_end < 0){
			printf("wrong\n");
		}

		if (initializor->fileInfoList[0].right_end - initializor->fileInfoList[0].left_end < (PAGESIZE / SizeIntT)) {
			//Update the radius.
			UpdateRadius(collisionCountStruct);
		} else {
			break;
		}
	}

}

// Read a bucket from the ith dimension hash table.
int CountBucket(PC2StructureT collisionCountStruct, int dimensionID, int pointsMetThreshold, int* countList,
		int* leftPointer, int* rightPointer, int* flag) {

	PC2LSH_InitializorT initializor = collisionCountStruct->initializor;

	int countThreshold = initializor->countThreshold;
	int k = initializor->k;

	// The left and right clips in this round.
	int left_end = initializor->fileInfoList[dimensionID].left_end;
	int right_end = initializor->fileInfoList[dimensionID].right_end;

	// The left and right offsets in the last round.
	int last_left = initializor->fileInfoList[dimensionID].left_cur;
	int last_right = initializor->fileInfoList[dimensionID].right_cur;

	int length_left = last_left - leftPointer[dimensionID] - left_end;
	int length_right = right_end - rightPointer[dimensionID] - last_right;

	int cnt = PAGESIZE / SizeIntT;
	int pageSize = cnt;

	// The start position of the current part which is needed to read.
	int readPos = left_end;

	if ((length_left <= 0 && length_right > 0) || (length_left > 0 && length_right > 0 && leftPointer[dimensionID]
			>= rightPointer[dimensionID])) {
		// Read the right part.
		pageSize = length_right > cnt ? cnt : length_right;

		// Record the start position which is needed to read.
		readPos = last_right + rightPointer[dimensionID];

		// Update the position of the right pointer.
		rightPointer[dimensionID] += pageSize;

	} else if ((length_left > 0 && length_right <= 0) || (length_left > 0 && length_right > 0
			&& leftPointer[dimensionID] < rightPointer[dimensionID])) {
		// Read the left part.
		pageSize = length_left > cnt ? cnt : length_left;

		// Record the start position which is needed to read.
		readPos = last_left - leftPointer[dimensionID] - pageSize;

		// Update the position of the left pointer.
		leftPointer[dimensionID] += pageSize;

	} else if (length_left <= 0 && length_right <= 0) {
		// Both two sides are finished reading.

		// Update the clips in the last round.
		initializor->fileInfoList[dimensionID].left_cur = left_end;
		initializor->fileInfoList[dimensionID].right_cur = right_end;

		// Mark that the interval is finished reading in this round.
		flag[dimensionID] = 1;
		return ((int) collisionCountStruct->resultIDList.size());
	}

	int readEnd = readPos + pageSize;
	int id = 0;

	for (int i = readPos; i < readEnd; i++) {
		id = initializor->hashTables[dimensionID][i];

		// countList[id] > k means that object has been recorded as a candidate.
		if (countList[id] <= k) {

			countList[id]++;

			if (countList[id] >= countThreshold) {

				collisionCountStruct->resultIDList.push_back(id);

				countList[id] = k + 1;

				// Added on 2011-10-24 by Junho
				if ((int) collisionCountStruct->resultIDList.size() >= pointsMetThreshold) {
					return collisionCountStruct->resultIDList.size();
				}
			}
		}
	}
	return ((int) collisionCountStruct->resultIDList.size());
}

int CollisionCount_RAM(PC2StructureT collisionCountStruct, PPointT q, int nNNs) {

	PC2LSH_InitializorT initializor = collisionCountStruct->initializor;

	int k = initializor->k;

	//Collision counter list
	int* countList;

	//Flag array (Use to mark the reading state of each hash table.)
	int* flag;

	//Record the current positions of left pointers.
	int* leftPointer;

	//Record the current positions of right pointers.
	int* rightPointer;

	FAILIF(NULL == (flag = (int*) MALLOC(k * SizeIntT)));

	FAILIF(NULL == (leftPointer = (int*) MALLOC(k * SizeIntT)));

	FAILIF(NULL == (rightPointer = (int*) MALLOC(k * SizeIntT)));

	// (\beta * n + k in paper)
	int pointsMetThreshold = initializor->pointsMetThreshold + nNNs;

	// Initialize the C2Structure.
	Initialize(collisionCountStruct);

	// Project the query point into k hash tables.
	ProjectQueryPoint(collisionCountStruct, q);

	// The collision counter array.
	FAILIF(NULL == (countList = (int*) MALLOC(initializor->nPoints * SizeIntT)));
	// Initialize all counters.
	memset(countList, 0, initializor->nPoints * SizeIntT);

	// This array is to record the ID of the left most buckets which should be searched in this round.
	int* basicIntervalIDList;
	int* lastBasicIntervalIDList;

	FAILIF(NULL == (basicIntervalIDList = (int*) MALLOC(k * SizeIntT)));
	FAILIF(NULL == (lastBasicIntervalIDList = (int*) MALLOC(k * SizeIntT)));
	memset(lastBasicIntervalIDList, 0, k * SizeIntT);

	// Find an appropriate radius so that the bucket size fits in a page.
	FindRadius_RAM(collisionCountStruct);

	// Initialize the ID of the left most buckets.
	for (int i = 0; i < k; i++) {
		basicIntervalIDList[i] = ((int) (collisionCountStruct->queryHashValueList[i]
				/ collisionCountStruct->currentRadius)) * collisionCountStruct->currentRadius;

		// Update the clips.(It means that update the IDs of the left most and right most buckets.)
		UpdateClips_RAM(initializor, basicIntervalIDList[i], collisionCountStruct->currentRadius, i);

		initializor->fileInfoList[i].left_cur = initializor->fileInfoList[i].right_cur
				= initializor->fileInfoList[i].right_end;
	}

	timeCollisionCount = 0;

	// Record the of dimensions which is finished reading.
	int trueCount = 0;

	// Added on 2012-02-28 by Junho
	// To record the correct candidate count
	int cnt = 0;

	// If there is no answer which should be returned, then continue the loop.
	while (true) {

		// Added on 2012-02-28 by Junho
		// Clean the candidate count.
		cnt = 0;
		double cR = initializor->c * collisionCountStruct->currentRadius;
		for (int i = 0; i < collisionCountStruct->resultList.size(); i++) {
			if (collisionCountStruct->resultList[i].distanceFromQuery <= cR) {
				cnt++;
			}
		}
		// If the candidate count is larger than nNNs, then returned them.
		if (cnt >= nNNs) {

			FREE(flag);
			FREE(countList);
			FREE(leftPointer);
			FREE(rightPointer);

			// Return the number of the objects found.
			return collisionCountStruct->resultIDList.size();
		}

		// Initialize the arrays.
		memset(flag, 0, k * SizeIntT);
		memset(leftPointer, 0, k * SizeIntT);
		memset(rightPointer, 0, k * SizeIntT);

		trueCount = 0;

		while (true) {

			// Retrieval k dimensions.
			for (int i = 0; i < k; i++) {

				// Judge whether this dimension is finished reading in this round.
				if (flag[i] == 0) {

					//					TIMEV_START(timeSearchBucket);
					CountBucket(collisionCountStruct, i, pointsMetThreshold, countList, leftPointer, rightPointer, flag);
					//					TIMEV_END(timeSearchBucket);

					// Added on 2012-02-28 by Junho
					for (int l = collisionCountStruct->resultList.size(); l < collisionCountStruct->resultIDList.size(); l++) {
						ResultItem item;
						item.pointID = collisionCountStruct->resultIDList[l];
						item.distanceFromQuery = distance(pointsDimension, dataSetPoints[item.pointID], q);
						collisionCountStruct->resultList.push_back(item);

						if (item.distanceFromQuery <= cR) {
							cnt++;
						}
					}

					if (collisionCountStruct->resultIDList.size() >= pointsMetThreshold) {
						FREE(flag);
						FREE(countList);
						FREE(leftPointer);
						FREE(rightPointer);

						//printf("%d %d\n", i + 1, trueCount);
						return collisionCountStruct->resultIDList.size();
					}
				} else if (flag[i] == 1) {
					trueCount++;
					flag[i] = 2;
					if (trueCount == k) {
						break;
					}
				}
			}

			// Jump out of the loop of while.
			if (trueCount == k)
				break;
		}// the second while ends

		UpdateRadius(collisionCountStruct);

		double radius = collisionCountStruct->currentRadius;

		// Swap the current interval ID list with that in the last round.
		int* temp = lastBasicIntervalIDList;
		lastBasicIntervalIDList = basicIntervalIDList;
		basicIntervalIDList = temp;

		// Update the current interval ID list.
		for (int i = 0; i < k; i++) {
			//			TIMEV_START(timeSearchBucket);
			basicIntervalIDList[i] = ((int) (collisionCountStruct->queryHashValueList[i] / radius)) * radius;
			//			TIMEV_END(timeSearchBucket);

			UpdateClips_RAM(initializor, basicIntervalIDList[i], collisionCountStruct->currentRadius, i);
		}

	}// the first while ends
}
