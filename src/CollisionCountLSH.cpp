/*
 * CollisionCountLSH.cpp
 *
 *  Created on: 2011-3-12
 *      Author: Junhao Gan
 */

#include "headers.h"



// Initialize
void Initialize(PC2StructureT collisionCountStruct) {

	//前一个的半径大小
	collisionCountStruct->lastRadius = 1 / (collisionCountStruct->initializor->c);

	//当前的半径大小
	collisionCountStruct->currentRadius = 1;

	//当前的cr大小
	collisionCountStruct->currentCR = collisionCountStruct->initializor->c;

}

// Project query point into <k> hash tables.
int* ProjectQueryPoint(PC2StructureT collisionCountStruct, PPointT q) {

	PC2LSH_InitializorT initializor = collisionCountStruct->initializor;

	FAILIF(NULL==(collisionCountStruct->queryHashValueList=(int*)MALLOC(initializor->k * SizeIntT)));

	double value = 0;

	int* queryValueList;

	FAILIF(NULL==(queryValueList=(int*)MALLOC(initializor->k * SizeIntT)));

	for (int i = 0; i < initializor->k; i++) {
		value = 0;

		for (int m = 0; m < initializor->dimension; m++) {
			value += (initializor->hashFunctionList[i].a[m] * q->coordinates[m]);
		}

		value += initializor->hashFunctionList[i].b;
		value /= initializor->w;

		collisionCountStruct->queryHashValueList[i] = (int) (value);
	}

	return queryValueList;
}

// Update radius.
void UpdateRadius(PC2StructureT collisionCountStruct) {

	collisionCountStruct->lastRadius = collisionCountStruct->currentRadius;

	collisionCountStruct->currentRadius = collisionCountStruct->currentCR;

	collisionCountStruct->currentCR = collisionCountStruct->currentRadius * collisionCountStruct->initializor->c;
}

// Return the index of the smallest value no less than the target value.
// If the value in the list, then return this value's index.
int UpperBound(vector<OffsetItem> &list, int s, int t, int targetValue) {
	int mid = 0;

	while (s < t) {
		mid = (s + t + 1) / 2;
		if (list[mid].bucketID == targetValue) {
			return mid;
		}

		if (list[mid].bucketID <= targetValue) {
			s = mid;
		} else
			t = mid - 1;
	}

	return s;
}

// Return the index of the largest value no more than the target value.
// If the value in the list, then return this value's index.
int LowerBound(vector<OffsetItem> &list, int s, int t, int targetValue) {
	int mid = 0;

	while (s < t) {
		mid = (s + t) / 2;
		if (list[mid].bucketID == targetValue) {
			return mid;
		}

		if (list[mid].bucketID >= targetValue) {
			t = mid;
		} else
			s = mid + 1;
	}

	return s;
}

////数一个区间(数组版本)
//int CountAnInterval_Array(PC2StructureT collisionCountStruct, int dimensionID, int* countList, int intervalID) {
//
//	PC2LSH_InitializorT initializor = collisionCountStruct->initializor;
//
//	int k = initializor->k;
//
//	int countThreshold = initializor->countThreshold;
//
//	hash_map<int, int*>::iterator it;
//
//	//	TimeVarT searchBucketTime = 0;
//	//
//	//
//	//	TIMEV_START(searchBucketTime);
//
//	//在set中查找给定intervalID的区间
//	it = initializor->hashValueTable_Array[dimensionID].find(intervalID);
//
//	//如果找到对应的hash区间,否则就直接跳过
//	if (it != initializor->hashValueTable_Array[dimensionID].end()) {
//
//		int* idList = it->second;
//
//		int length = idList[0] + 1;
//
//		int id;
//
//		//把这个区间内的所有对象都数一次
//		for (int j = 1; j < length; j++) {
//
//			id = idList[j];
//
//			//如果该点出现的次数已经超过k次了,也就是证明了,这个点已经被记录为返回结果了
//			//如果出现次数<=k次,则可以继续操作
//			if (countList[id] <= k) {
//
//				countList[id]++;
//
//				//如果当前点出现的次数超过阈值,则计算它与查询对象q的真实距离
//				if (countList[id] >= countThreshold) {
//
//					//4-4添加,把resultIDList改为IntT型数组
//					collisionCountStruct->resultIDList.push_back(id);
//
//					//设置countItem出现的次数超过最多值k
//					//也就是用来标记是否已经不用再数这个数据点的碰撞次数了
//					countList[id] = k + 1;
//				}
//			}
//		}
//	}
//
//	//	//计时结束
//	//	TIMEV_END(searchBucketTime);
//	//
//	//	timeSearchBucket += searchBucketTime;
//
//	//返回当前结果列表的大小
//	return ((int) collisionCountStruct->resultIDList.size());
//}
//
////定义一个函数指针
//typedef int (*CountingFunction)(PC2StructureT collisionCountStruct, int dimensionID, int* countList, int intervalID);
//
///*Collision Count的函数(数组版本)(可选hash_map或map版本)
// * 返回在cr距离内的对象个数
// */
//int CollisionCount_ArrayVersion(PC2StructureT collisionCountStruct, PPointT q, int nNNs, CountingFunction countingFunction) {
//
//	PC2LSH_InitializorT initializor = collisionCountStruct->initializor;
//
//	int k = initializor->k;
//
//	//	当找够超过k + \beta * n个才返回
//	int pointsMetThreshold = initializor->pointsMetThreshold + nNNs;
//
//	//初始化
//	Initialize(collisionCountStruct);
//
//	//投影查询对象
//	ProjectQueryPoint(collisionCountStruct, q);
//
//	//记录每个点碰撞次数的数组
//	int* countList;
//
//	FAILIF(NULL==(countList=(int*)MALLOC(initializor->nPoints * SizeIntT)));
//	memset(countList, 0, initializor->nPoints * SizeIntT);
//
//	//记录区间基值ID的数组
//	int* basicIntervalIDList;
//	int* lastBasicIntervalIDList;
//
//	FAILIF(NULL==(basicIntervalIDList=(int*)MALLOC(k * SizeIntT)));
//	FAILIF(NULL==(lastBasicIntervalIDList=(int*)MALLOC(k * SizeIntT)));
//
//	//记录区间ID的相关变量
//	int basicIntervalID = 0;
//	//	IntT basicEndIntervalID = 0;
//	int lastBasicIntervalID = 0;
//	int lastEndIntervalID = 0;
//	int intervalID = 0;
//
//	double radius = collisionCountStruct->currentRadius;
//
//	//初始化当前区间基值ID数组
//	for (int i = 0; i < k; i++) {
//		basicIntervalIDList[i] = ((int) (collisionCountStruct->queryHashValueList[i] / radius)) * radius;
//
//		//		//初始化文件信息列表的文件句柄
//		//		InitializeFileInfoList(initializor, i);
//	}
//
//	timeCollisionCount = 0;
//
//	//	//计时开始
//	//	TIMEV_START(candidateTime);
//
//	//如果当前还没有找到结果,则继续循环
//	while (((int) collisionCountStruct->resultIDList.size()) < nNNs) {
//
//		//这趟循环需要遍历的区间的个数
//		//		IntT intervalCount = radius - collisionCountStruct->lastRadius;
//		//		IntT intervalCount = radius;
//
//		radius = collisionCountStruct->currentRadius;
//
//		//遍历宽度为当前半径r的区间
//		for (int r = 0; r < radius; r++) {
//
//			//遍历k个维度
//			for (int i = 0; i < initializor->k; i++) {
//
//				//计算第i维投影区间的基值,即Floor(x/r)*r
//				basicIntervalID = basicIntervalIDList[i];
//
//				//计算第i维前一个半径大小的投影区间基值
//				lastBasicIntervalID = lastBasicIntervalIDList[i];
//
//				//前一个半径大小的投影区间右端值
//				lastEndIntervalID = lastBasicIntervalID + collisionCountStruct->lastRadius - 1;
//
//				// 这段代码是直接从新的basicID开始加(还是这种方法比下面的方法效果好)
//				intervalID = basicIntervalID + r;
//
//				if (intervalID < lastBasicIntervalID || intervalID > lastEndIntervalID) {
//
//					//					int s = CountAnInterval_Array(collisionCountStruct, i, countList, intervalID);
//
//
//					if (CountAnInterval_Array(collisionCountStruct, i, countList, intervalID) >= pointsMetThreshold) {
//						FREE(countList);
//						//printf("%d\n", i + 1);
//
//						//						//计时结束
//						//						TIMEV_END(candidateTime);
//
//						return collisionCountStruct->resultIDList.size();
//					}
//
//				}
//
//				//				//这段代码是从原来的intervalID开始扩展开
//				//				intervalID = lastEndIntervalID + 1 + r;
//				//				if (intervalID < basicIntervalID + radius) {
//				//					if (CountAnInterval_Array(collisionCountStruct, i, countList, intervalID) >= pointsMetThreshold) {
//				//						FREE(countList);
//				//						//						for (int j = 0; j < (int) collisionCountStruct->resultList.size(); j++) {
//				//						//							collisionCountStruct->resultList[j].distanceFromQuery = distance(initializor->dimension, q,
//				//						//									initializor->dataSetPoints[collisionCountStruct->resultList[j].pointID]);
//				//						//						}
//				//
//				//						//						//根据与q的距离从小到大排序
//				//						//						sort(collisionCountStruct->resultList.begin(), collisionCountStruct->resultList.end(), less<
//				//						//								ResultItem> ());
//				//						return collisionCountStruct->resultIDList.size();
//				//					}
//				//				}
//				//
//				//				intervalID = lastBasicIntervalID - 1 - r;
//				//				if (intervalID >= basicIntervalID) {
//				//					if (CountAnInterval_Array(collisionCountStruct, i, countList, intervalID) >= pointsMetThreshold) {
//				//						FREE(countList);
//				//						//						for (int j = 0; j < (int) collisionCountStruct->resultList.size(); j++) {
//				//						//							collisionCountStruct->resultList[j].distanceFromQuery = distance(initializor->dimension, q,
//				//						//									initializor->dataSetPoints[collisionCountStruct->resultList[j].pointID]);
//				//						//						}
//				//
//				//						//						//根据与q的距离从小到大排序
//				//						//						sort(collisionCountStruct->resultList.begin(), collisionCountStruct->resultList.end(), less<
//				//						//								ResultItem> ());
//				//						return collisionCountStruct->resultIDList.size();
//				//					}
//				//				}
//
//			}
//		}
//
//		//更新半径大小
//		UpdateRadius(collisionCountStruct);
//
//		radius = collisionCountStruct->currentRadius;
//
//		//交换当前与上一个的数组
//		int* temp = lastBasicIntervalIDList;
//		lastBasicIntervalIDList = basicIntervalIDList;
//		basicIntervalIDList = temp;
//
//		//更新当前区间基值ID数组和文件信息表
//		for (int i = 0; i < k; i++) {
//			basicIntervalIDList[i] = ((int) (collisionCountStruct->queryHashValueList[i] / radius)) * radius;
//		}
//
//	}//while结束
//
//	FREE(countList);
//	//	for (int j = 0; j < (int) collisionCountStruct->resultList.size(); j++) {
//	//		collisionCountStruct->resultList[j].distanceFromQuery = distance(initializor->dimension, q, initializor->dataSetPoints[collisionCountStruct->resultList[j].pointID]);
//	//	}
//
//	//	//根据与q的距离从小到大排序
//	//	sort(collisionCountStruct->resultList.begin(), collisionCountStruct->resultList.end(), less<ResultItem> ());
//	//	//计时结束
//	//	TIMEV_END(candidateTime);
//
//	return collisionCountStruct->resultIDList.size();
//}
//
////采用数组统计
//int CollisionCount_Array(PC2StructureT collisionCountStruct, PPointT q, int nNNs) {
//	return CollisionCount_ArrayVersion(collisionCountStruct, q, nNNs, CountAnInterval_Array);
//}



