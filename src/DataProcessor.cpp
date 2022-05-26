/*
 * DataProcessor.cpp
 *
 *  Created on: 2011-9-26
 *      Author: Junhao Gan
 */

#include "headers.h"

// Read a point from file.
PPointT readPoint(FILE *fileHandle) {
	PPointT p;
	double sqrLength = 0;
	FAILIF(NULL == (p = (PPointT)MALLOC(sizeof(PointT))));
	FAILIF(NULL == (p->coordinates = (double*)MALLOC(pointsDimension * sizeof(double))));

	// fscanf(fileHandle, "%d", &(p->index));

	for (int d = 0; d < pointsDimension; d++) {
		// fscanf(fileHandle, "%lf", &(p->coordinates[d]));
		float temp;
		fread(&temp, sizeof(float), 1, fileHandle);

		p->coordinates[d] = (double)temp;
		sqrLength += SQR(p->coordinates[d]);

		//记录最大和最小的坐标值
		if (p->coordinates[d] > maxCoordinate) {
			maxCoordinate = p->coordinates[d];
		}
		if (p->coordinates[d] < minCoordinate) {
			minCoordinate = p->coordinates[d];
		}
	}

	// 这里估计是为了将换行符读掉
	// fscanf(fileHandle, "%[^\n]", sBuffer);

	//	p->sqrLength = sqrLength;
	if (sqrLength > maxSqrLength)
		maxSqrLength = sqrLength;
	else if (sqrLength < minSqrLength && sqrLength != 0)
		minSqrLength = sqrLength;
	return p;
}

// Reads in the data set points from <filename> in the array
// <dataSetPoints>. Each point get a unique number in the field
// <index> to be easily indentified.
void readDataSetFromFile(char *filename) {
	// FILE *f = fopen(filename, "rt");
	FILE *f = fopen(filename, "rb");
	FAILIF(f == NULL);

	// 申请n个点的空间
	FAILIF(NULL == (dataSetPoints = (PPointT*)MALLOC(nPoints * sizeof(PPointT))));
	// 从指定文件中读取,并且给出对应编号(索引)
	for (int i = 0; i < nPoints; i++) {
		dataSetPoints[i] = readPoint(f);
		dataSetPoints[i]->index = i;
	}
	printf("\nCoordinate's Range: [%lf, %lf]\n", minCoordinate, maxCoordinate);
}

// Read the data set points to a specified array with size = n.
void readDataSetFromFile(PPointT* pointList, int n, char *filename) {
	FILE *f = fopen(filename, "rt");
	FAILIF(f == NULL);

	//	// 申请n个点的空间
	//	FAILIF(NULL == (pointList = (PPointT*)MALLOC(n * sizeof(PPointT))));
	// 从指定文件中读取,并且给出对应编号(索引)
	for (int i = 0; i < n; i++) {
		pointList[i] = readPoint(f);
	}
}

// Output the ground truth file.
void writeGroundTruth(char* queryFilePath, char* groundTruthFilePath, int nQueries) {
	PPointT* queryPointList;

	FAILIF(NULL == (queryPointList = (PPointT*)MALLOC(nQueries * sizeof(PPointT))));

	FILE *queryFile = fopen(queryFilePath, "rb");
	FAILIF(queryFile == NULL);

	FILE *groundTruthFile = fopen(groundTruthFilePath, "w");
	FAILIF(groundTruthFile == NULL);

	int maxKNN = 100;
	fprintf(groundTruthFile, "%d %d\n", nQueries, maxKNN);

	// 一次过读取所有queryPoint
	for (int i = 0; i < nQueries; i++) {
		//			FAILIF(NULL == (queryPointList[i]->coordinates = (RealT*)MALLOC(pointsDimension * sizeof(RealT))));
		queryPointList[i] = readPoint(queryFile);
		queryPointList[i]->index = i;

		vector<double> groundTruthes;
		double dis;

		//计算所有真实值
		for (int j = 0; j < nPoints; j++) {
			dis = distance(pointsDimension, dataSetPoints[j], queryPointList[i]);
			groundTruthes.push_back(dis);
		}

		//ground truth排序
		partial_sort(groundTruthes.begin(), groundTruthes.begin() + 100, groundTruthes.end());

		fprintf(groundTruthFile, "%d", queryPointList[i]->index);
		for (int k = 0; k < maxKNN; k++) {
			fprintf(groundTruthFile, " %lf", groundTruthes[k]);
		}
		fprintf(groundTruthFile, "\n");
		printf("%d\n", i + 1);
	}
	fclose(queryFile);
	fclose(groundTruthFile);
}

// Read the ground truth file.
void readGroundTruth(double** groundTruthFileList, int nQueries, char* groundTruthFilePath) {

	int max_knn = 100;

	FILE *groundTruthFile = fopen(groundTruthFilePath, "rt");
	FAILIF(groundTruthFile == NULL);

	//	FAILIF(NULL == (groundTruthFileList = (double**)MALLOC(nQueries * sizeof(double*))));

	int a;
	fscanf(groundTruthFile, "%d %d\n", &a, &max_knn);

	for (int i = 0; i < nQueries; i++) {
		FAILIF(NULL == (groundTruthFileList[i] = (double*)MALLOC(max_knn * sizeof(double))));

		fscanf(groundTruthFile, "%d", &a);

		for (int j = 0; j < max_knn; j++) {

			double temp;
			fscanf(groundTruthFile, " %lf", &(temp));
			groundTruthFileList[i][j] = temp;

		}
		// 这里估计是为了将换行符读掉
		// fscanf(fileHandle, "%[^\n]", sBuffer);
	}

	fclose(groundTruthFile);
}
