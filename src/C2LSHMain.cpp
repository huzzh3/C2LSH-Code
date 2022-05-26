/*
 * C2LSHMain.cpp
 *
 *  Created on: 2011-9-26
 *      Author: Junhao Gan
 */

#ifndef IGNORE_LSH

#include "headers.h"

/*
 * Prints the usage of the C2LSHMain.
 */
void usage() {
	printf("C2LSH coded by Junhao Gan, Sun Yat-sen University\n");
	printf("Option\n");
	printf("-n  {integer}\t the number of points\n");
	printf("-qn {integer}\t the number of queries\n");
	printf("-d  {integer}\t the dimensionality of points\n");
	printf("-ds {string}\t the file path of dataset\n");
	printf("-qs {string}\t the file path of query set\n");
	printf("-pf {string}\t the folder path of projection files\n");
	printf("-gt {string}\t the file path of ground truth\n");
	printf("-k  {integer}\t the k of k-NN\n");
	printf("-b  {integer}\t the page size\n");
	printf("-nid{integer}\t the number of id in each counting in RAM mode\n");
	printf("-rf {string}\t the folder path of results\n");
	printf("-c  {double}\t the approximate ratio, [c = 3] in default\n");
	printf("-useCt {1 or 0}\t If [useCt = 1] then use <Ct> as the count threshold.\n\t\t Otherwise, use <l>. [useCt = 1] in default.\n");
	printf("\n");

	printf("1. Construct hash tables.\n");
	printf("   Parameter list: -n -d -ds -pf [-c]\n");
	printf("\n");

	printf("2. Generate the ground truth file.\n");
	printf("   Parameter list: -n -d -ds -qn -qs -gt\n");
	printf("\n");

	printf("3. Process queries by external I/O.\n");
	printf("   Parameter list: -n -d -ds -qn -qs -pf -gt -b [-k] [-c] [-useCt] [-rf]\n");
	printf("\n");

	printf("4. Process queries in RAM\n");
	printf("   Parameter list: -n -d -ds -qn -qs -pf -gt [-nid] [-k] [-c] [-useCt] [-rf]\n");
	printf("\n");
}

/*
 * Get the index of next unblank char from a string.
 */
int GetNextChar(char *str) {
	int rtn = 0;

	// Jump over all blanks
	while (str[rtn] == ' ') {
		rtn++;
	}

	return rtn;
}

/*
 * Get next word from a string.
 */
void GetNextWord(char *str, char *word) {
	// Jump over all blanks
	while (*str == ' ') {
		str++;
	}

	while (*str != ' ' && *str != '\0') {
		*word = *str;
		str++;
		word++;
	}

	*word = '\0';
}

/*
 * Process queries in RAM.
 * When k == -1, it processes {1, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100}-NN queries with each query point.
 * When k != -1, it processes only k-NN query with each query point.
 */
void ProcessQueries_RAM(PC2LSH_InitializorT initializor, char* ds, int qn, char *qs, char* pf, char* gt, char* rf, int k = -1, double c = 3, bool useCt = true) {
	// Initialize the random seed.
	srand((unsigned) time(NULL));

	// Read dataset from file.
	readDataSetFromFile(ds);

	// Initialize the algorithm structure.
	Initialize(initializor, dataSetPoints, nPoints, pointsDimension, maxCoordinate, pf, c, useCt);

	ReadHashTables(initializor);
	//PrepareForCounting(initializor);

	printf("Prepare finished.\n\n");

	// Read the specified query set from file.
	PPointT* queryPointList = NULL;
	FAILIF(NULL == (queryPointList = (PPointT*)MALLOC(qn * sizeof(PPointT))));
	readDataSetFromFile(queryPointList, qn, qs);

	// Read ground truth file.
	double** groundTruthList = NULL;
	FAILIF(NULL == (groundTruthList = (double**)MALLOC(qn * sizeof(double*))));
	readGroundTruth(groundTruthList, qn, gt);

	int workLoad = qn;
	int target_nNNs = 0;
	double overallRatio = 0;
	TimeVarT meanQueryTime = 0;

	// Added by Junho on 2012-02-26. To run tests.
	int kNNs[11] = { 1, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
	int testRound = 0;
	int maxRound = 11;

	double allTime[11];
	double allRatio[11];
	memset(allTime, 0, 11 * SizeRealT);
	memset(allRatio, 0, 11 * SizeRealT);

	if (k != -1) {
		kNNs[0] = k;
		maxRound = 1;
	}

	while (true) {
		if (testRound == maxRound)
			break;
		target_nNNs = kNNs[testRound++];

		//Record all the single rank ratio.
		double rankRatioArray[100];
		memset(rankRatioArray, 0, 100 * sizeof(double));

		//Initialize all the recording parameter in each test round.
		overallRatio = 0;
		meanQueryTime = 0;

		//Queries start.
		for (int i = 0; i < workLoad; i++) {

			C2Structure c2structure(initializor);

			//Global variant of query time.
			timeRNNQuery = 0;

			PPointT query = queryPointList[i];

			//			timeSearchBucket = 0;

			//Time starts.
			TIMEV_START(timeRNNQuery);

			//Process a query.
			CollisionCount_RAM(&c2structure, query, target_nNNs);

			//Sort the result list.
			sort(c2structure.resultList.begin(), c2structure.resultList.end(), less<ResultItem> ());

			//printf("%d\n", s);

			//Time ends.
			TIMEV_END(timeRNNQuery);

			meanQueryTime += timeRNNQuery;

			// 输出具体每个query的情况
			//			printf("Query point %d: found %d NNs at distance %0.6lf with projectScale %0.6lf and r %0.6lf. First %d NNs are:\n", i, nNNs,
			//					(double) (collisionCountStruct.currentRadius), collisionCountStruct.initializor->projectScale, collisionCountStruct.currentRadius
			//							/ collisionCountStruct.initializor->projectScale, MIN(nNNs, MAX_REPORTED_POINTS));
			//
			//			for (IntT j = 0; j < MIN(nNNs, MAX_REPORTED_POINTS); j++) {
			//				printf("%09d\tDistance:%0.6lf\tGround Truth: %09d\tDistance:%0.6lf\n", collisionCountStruct.resultList[j].pointID,
			//						collisionCountStruct.resultList[j].distanceFromQuery, collisionCountStruct.groundTruth[j].pointID, collisionCountStruct.groundTruth[j].distanceFromQuery);
			//			}

			double ratio = 0;

			int zeroCount = 0;

			//Calculate the Average Overall Approximate Ratio.
			for (int j = 0; j < target_nNNs; j++) {
				if (groundTruthList[i][j] == 0) {
					zeroCount++;
					continue;
				}
				double temp = c2structure.resultList[j].distanceFromQuery == groundTruthList[i][j] ? 1 : c2structure.resultList[j].distanceFromQuery / groundTruthList[i][j];
				ratio += temp;
				rankRatioArray[j] += temp;
			}

			overallRatio += (ratio / (target_nNNs - zeroCount));

		}

		workLoad = MIN(workLoad, qn);

		//Output the Average Running Time.
		if (workLoad > 0) {
			meanQueryTime = meanQueryTime / workLoad;
			overallRatio = overallRatio / workLoad;
			timeSearchBucket /= workLoad;
			candidateTime /= workLoad;
			allTime[testRound - 1] = meanQueryTime;
			allRatio[testRound - 1] = overallRatio;
			printf("%d-NN\n", kNNs[testRound - 1]);
			printf("Average query time:\t%0.6lf\n", (double) meanQueryTime);
			//			printf("Mean bucket searching time:\t%0.6lf\n", (double) timeSearchBucket);
			//			printf("Mean candidate searching time:\t%0.6lf\n", (double) candidateTime);
			printf("Average overall ratio:\t%0.6lf\n", (double) overallRatio);
			printf("\n");

			// Output query results to file under the path of <-rf>
			char filename[110];
			char buffer[10];
			sprintf(buffer, "%d-NN", kNNs[testRound - 1]);
			strcpy(filename, rf);
			strcat(filename, buffer);
			FILE *file;
			file = fopen(filename, "w");
			if (file == NULL) {
				mkdir(rf, 0777);
				file = fopen(filename, "w");
				FAILIF(NULL == file);
			}

			fprintf(file, "%d-NN\n", kNNs[testRound - 1]);
			fprintf(file, "Average query time:\t%0.6lf\n", (double) meanQueryTime);
			fprintf(file, "Average overall ratio:\t%0.6lf\n", (double) overallRatio);
			fprintf(file, "\n");

			// Output the rank i ratio
			for (int j = 0; j < target_nNNs; j++) {
				rankRatioArray[j] = rankRatioArray[j] / workLoad;
				fprintf(file, "Rank %d ratio:\t%0.6lf\n", j + 1, rankRatioArray[j]);
			}

			fclose(file);
		}
	}

	char output[1000] = "";
	sprintf(output, "%sResult_Statistics", rf);
	FILE* file = fopen(output, "w");
	FAILIF(file == NULL);
	fprintf(file, "k-NN\tQuery Time\tRatio\n");
	for (int i = 0; i < maxRound; i++) {
		fprintf(file, "%d\t%0.6lf\t%0.6lf\n", kNNs[i], allTime[i], allRatio[i]);
	}
	fclose(file);

	printf("Queries finished.\n");
}

/*
 * Process queries by IO.
 * When k == -1, it processes {1, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100}-NN queries with each query point.
 * When k != -1, it processes only k-NN query with each query point.
 */
void ProcessQueries_External(PC2LSH_InitializorT initializor, char* ds, int qn, char *qs, char* pf, char* gt, int b, char* rf, int k = -1, double c = 3, bool useCt = true) {
	// Initialize the random seed.
	srand((unsigned) time(NULL));

	// Read dataset from file.
	readDataSetFromFile(ds);

	// Initialize the algorithm structure.
	Initialize(initializor, dataSetPoints, nPoints, pointsDimension, maxCoordinate, pf, c, useCt);

	// Read the index file.
	ReadIndexInfo(initializor);

	// Read the specified query set from file.
	PPointT* queryPointList = NULL;
	FAILIF(NULL == (queryPointList = (PPointT*)MALLOC(qn * sizeof(PPointT))));
	readDataSetFromFile(queryPointList, qn, qs);

	// Read ground truth file.
	double** groundTruthList = NULL;
	FAILIF(NULL == (groundTruthList = (double**)MALLOC(qn * sizeof(double*))));
	readGroundTruth(groundTruthList, qn, gt);

	int workLoad = qn;
	int target_nNNs = 0;
	double overallRatio = 0;
	TimeVarT meanQueryTime = 0;

	int IOs = 0;

	// Added by Junho on 2012-02-26. To run tests.
	int kNNs[11] = { 1, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
	int testRound = 0;
	int maxRound = 11;

	int allIO[11];
	double allRatio[11];
	memset(allIO, 0, 11 * SizeIntT);
	memset(allRatio, 0, 11 * SizeRealT);

	if (k != -1) {
		kNNs[0] = k;
		maxRound = 1;
	}

	while (true) {
		if (testRound == maxRound)
			break;
		target_nNNs = kNNs[testRound++];

		//Record all the single rank ratio.
		double rankRatioArray[100];
		memset(rankRatioArray, 0, 100 * sizeof(double));

		//Initialize all the recording parameter in each test round.
		overallRatio = 0;
		IOs = 0;
		meanQueryTime = 0;

		//Queries start.
		for (int i = 0; i < workLoad; i++) {

			C2Structure c2structure(initializor);

			//Global variant of query time.
			timeRNNQuery = 0;

			PPointT query = queryPointList[i];

			//A global variant of the number of total distance calculating.
			nOfDistComps = 0;

			//A global variant of IO number.
			nOfIO = 0;
			//			timeSearchBucket = 0;

			//Time starts.
			TIMEV_START(timeRNNQuery);

			//Process a query.
			int nNNs = CollisionCount_External(&c2structure, query, target_nNNs);

			//			printf("%d\n",nNNs);

			//Sort the result list.
			sort(c2structure.resultList.begin(), c2structure.resultList.end(), less<ResultItem> ());

			//Time ends.
			TIMEV_END(timeRNNQuery);

			nOfIO += nNNs;

			meanQueryTime += timeRNNQuery;
			IOs += nOfIO;

			// 输出具体每个query的情况
			//			printf("Query point %d: found %d NNs at distance %0.6lf with projectScale %0.6lf and r %0.6lf. First %d NNs are:\n", i, nNNs,
			//					(double) (collisionCountStruct.currentRadius), collisionCountStruct.initializor->projectScale, collisionCountStruct.currentRadius
			//							/ collisionCountStruct.initializor->projectScale, MIN(nNNs, MAX_REPORTED_POINTS));
			//
			//			for (IntT j = 0; j < MIN(nNNs, MAX_REPORTED_POINTS); j++) {
			//				printf("%09d\tDistance:%0.6lf\tGround Truth: %09d\tDistance:%0.6lf\n", collisionCountStruct.resultList[j].pointID,
			//						collisionCountStruct.resultList[j].distanceFromQuery, collisionCountStruct.groundTruth[j].pointID, collisionCountStruct.groundTruth[j].distanceFromQuery);
			//			}

			double ratio = 0;

			int zeroCount = 0;

			//Calculate the Average Overall Approximate Ratio.
			for (int j = 0; j < target_nNNs; j++) {
				if (groundTruthList[i][j] == 0) {
					zeroCount++;
					continue;
				}
				double temp = c2structure.resultList[j].distanceFromQuery == groundTruthList[i][j] ? 1 : c2structure.resultList[j].distanceFromQuery / groundTruthList[i][j];
				ratio += temp;
				rankRatioArray[j] += temp;
				//printf("qid %d\tratio %lf\n", j + 1, temp);
			}

			overallRatio += (ratio / (target_nNNs - zeroCount));

		}

		workLoad = MIN(workLoad, qn);

		//Output the Average Running Time.
		if (workLoad > 0) {
			meanQueryTime = meanQueryTime / workLoad;
			overallRatio = overallRatio / workLoad;
			IOs = CEIL(IOs / workLoad);
			allIO[testRound - 1] = IOs;
			allRatio[testRound - 1] = overallRatio;
			printf("%d-NN\n", kNNs[testRound - 1]);
			printf("Average query time:\t%0.6lf\n", (double) meanQueryTime);
			printf("Average overall ratio:\t%0.6lf\n", (double) overallRatio);
			printf("Average overall IO:\t%d\n", IOs);
			//			timeSearchBucket /= workLoad;
			//			printf("Mean bucket searching time:\t%0.6lf\n", (double) timeSearchBucket);
			printf("\n");

			// Output query results to file under the path of <-rf>
			char filename[110];
			char buffer[10];
			sprintf(buffer, "%d-NN", kNNs[testRound - 1]);
			strcpy(filename, rf);
			strcat(filename, buffer);
			FILE* file;
			file = fopen(filename, "w");
			if (file == NULL) {
				mkdir(rf, 0777);
				file = fopen(filename, "w");
				FAILIF(NULL == file);
			}
			fprintf(file, "%d-NN\n", kNNs[testRound - 1]);
			fprintf(file, "Average query time:\t%0.6lf\n", (double) meanQueryTime);
			fprintf(file, "Average overall ratio:\t%0.6lf\n", (double) overallRatio);
			fprintf(file, "Average overall IO:\t%d\n", IOs);
			fprintf(file, "\n");

			// Output the rank i ratio
			for (int j = 0; j < target_nNNs; j++) {
				rankRatioArray[j] = rankRatioArray[j] / workLoad;
				fprintf(file, "Rank %d ratio:\t%0.6lf\n", j + 1, rankRatioArray[j]);
			}

			fclose(file);
		}
	}

	char output[1000] = "";
	sprintf(output, "%sResult_Statistics", rf);
	FILE* file = fopen(output, "w");
	FAILIF(file == NULL);
	fprintf(file, "k-NN\tIO\tRatio\n");
	for (int i = 0; i < maxRound; i++) {
		fprintf(file, "%d\t%d\t%lf\n", kNNs[i], allIO[i], allRatio[i]);
	}
	fclose(file);
	printf("Queries finished.\n");
}

// Added on 2012-06-14 by Junho
// The main entry of C2LSH
int main(int nargs, char **args) {
	// Print the usage
	// usage();

	// These two are global variables
	nPoints = -1; // the number of points
	pointsDimension = -1; // the dimensionality of points

	// -------------------- Some algorithm parameters --------------------
	int qn = -1; // the number of queries
	int k = -1; // the k of k-NN
	int b = -1; // page size
	int nid = -1; // the number of id in each counting in RAM mode
	char ds[200] = ""; // the file path of dataset
	char qs[200] = ""; // the file path of query set
	char pf[200] = ""; // the folder path of
	char gt[200] = ""; // the file path of ground truth
	char rf[200] = ""; // the folder path of results
	double c = 3; // the approximate ratio
	bool useCt = true; // use Ct or l as count threshold

	// -------------------- Some local variables --------------------

	int cnt = 1;
	bool failed = false;
	char *arg;
	int i;
	char para[10];

	// -------------------- Parse the paramters --------------------

	while (cnt < nargs && !failed) {
		arg = args[cnt++];
		if (cnt == nargs) {
			failed = true;
			break;
		}

		i = GetNextChar(arg);
		if (arg[i] != '-') {
			failed = true;
			break;
		}

		GetNextWord(arg + i + 1, para);
		printf("%s\t", para);

		arg = args[cnt++];

		if (strcmp(para, "n") == 0) {
			nPoints = atoi(arg);
			if (nPoints <= 0) {
				failed = true;
				break;
			}
			printf("%d\n", nPoints);
		} else if (strcmp(para, "d") == 0) {
			pointsDimension = atoi(arg);
			if (pointsDimension <= 0) {
				failed = true;
				break;
			}
			printf("%d\n", pointsDimension);
		} else if (strcmp(para, "qn") == 0) {
			qn = atoi(arg);
			if (qn <= 0) {
				failed = true;
				break;
			}
			printf("%d\n", qn);
		} else if (strcmp(para, "k") == 0) {
			k = atoi(arg);
			if (k <= 0) {
				failed = true;
				break;
			}
			printf("%d\n", k);
		} else if (strcmp(para, "b") == 0) {
			b = atoi(arg);
			if (b <= 0) {
				failed = true;
				break;
			}
			PAGESIZE = b;
			printf("%d\n", b);
		} else if (strcmp(para, "nid") == 0) {
			nid = atoi(arg);
			if (nid <= 0) {
				failed = true;
				break;
			}
			PAGESIZE = nid * SizeIntT;
			printf("%d\n", nid);
		} else if (strcmp(para, "c") == 0) {
			c = atof(arg);
			if (c != 3 && c != 2) {
				failed = true;
				printf("Currently, we only support c = 2 or c = 3.\n");
				break;
			}
			printf("%lf\n", c);
		} else if (strcmp(para, "useCt") == 0) {
			int temp = atoi(arg);
			if (temp != 1 && temp != 0) {
				failed = true;
				printf("useCt could only equal to 1 or 0.\n");
				break;
			} else {
				if (temp == 0) {
					useCt = false;
				}
				printf("%d\n", useCt);
			}
		} else if (strcmp(para, "ds") == 0) {
			GetNextWord(arg, ds);
			printf("%s\n", ds);
		} else if (strcmp(para, "qs") == 0) {
			GetNextWord(arg, qs);
			printf("%s\n", qs);
		} else if (strcmp(para, "pf") == 0) {
			GetNextWord(arg, pf);
			printf("%s\n", pf);
		} else if (strcmp(para, "gt") == 0) {
			GetNextWord(arg, gt);
			printf("%s\n", gt);
		} else if (strcmp(para, "rf") == 0) {
			GetNextWord(arg, rf);
			printf("%s\n", rf);
		} else {
			failed = true;
			printf("Unknown option -%s!\n\n", para);
		}

	}

	if (failed) {
		usage();
		return 0;
	}

	C2LSH_Initializor initializor;

	cnt = (nargs - 1) / 2;
	if (cnt == 4 || (cnt == 5 && (c == 3 || c == 2))) {
		// Construct hash tables.
		// Parameter list: -n -d -ds -pf [-c]

		if (nPoints != -1 && pointsDimension != -1 && ds[0] != '\0' && pf[0] != '\0') {

			// Initialize the random seed.
			srand((unsigned) time(NULL));

			// Read dataset from file.
			readDataSetFromFile(ds);

			// Initialize the algorithm structure.
			Initialize(&initializor, dataSetPoints, nPoints, pointsDimension, maxCoordinate, pf, c);

			// Output hash tables to file.
			WriteProjectedPoints(&initializor);

			printf("\nConstruction finish!\n");

		} else {
			failed = true;
		}
	} else if (cnt == 6) {
		// Generate ground truth.
		// Parameter list: -n -d -ds -qn -qs -gt

		if (nPoints != -1 && pointsDimension != -1 && ds[0] != '\0' && qn != -1 && qs[0] != '\0' && gt[0] != '\0') {

			// Read dataset from file.
			readDataSetFromFile(ds);

			// Write ground truth to file.
			writeGroundTruth(qs, gt, qn);

		} else {
			failed = true;
		}
	}

	else if (cnt >= 7 && b == -1) {
		// Process queries
		// RAM
		// Parameter list: -n -d -ds -qn -qs -pf -gt [-cnt] [-k] [-c] [-useCt] [-rf]

		if (rf[0] == '\0') {
			strcpy(rf, ds);
			int len = strlen(rf);
			while (rf[len] != '/' && rf[len] != '\\') {
				len--;
			}
			rf[len + 1] = '\0';
			if (rf[len] == '/') {
				strcat(rf, "RAM/");
			} else
				strcat(rf, "RAM\\");
			printf("rf\t%s\n", rf);
		}

		if (nPoints != -1 && pointsDimension != -1 && ds[0] != '\0' && qn != -1 && qs[0] != '\0' && pf[0] != '\0' && gt[0] != '\0' && rf[0] != '\0') {

			ProcessQueries_RAM(&initializor, ds, qn, qs, pf, gt, rf, k, c, useCt);
		}

	} else if (cnt >= 8 && b != -1) {
		// Process queries
		// I/O
		// Parameter list: -n -d -ds -qn -qs -pf -gt -b [-k] [-c] [-useCt] [-rf]

		if (rf[0] == '\0') {
			strcpy(rf, ds);
			int len = strlen(rf);
			while (rf[len] != '/' && rf[len] != '\\') {
				len--;
			}
			rf[len + 1] = '\0';
			if (rf[len] == '/') {
				strcat(rf, "IO/");
			} else
				strcat(rf, "IO\\");
			printf("rf\t%s\n", rf);
		}

		if (nPoints != -1 && pointsDimension != -1 && ds[0] != '\0' && qn != -1 && qs[0] != '\0' && pf[0] != '\0' && gt[0] != '\0' && b != -1 && rf[0] != '\0') {

			ProcessQueries_External(&initializor, ds, qn, qs, pf, gt, b, rf, k, c, useCt);
		} else {
			failed = true;
		}
	} else {
		failed = true;
	}

	if (failed) {
		printf("\n===========================================\n");
		printf("Please enter correct parameter list!");
		printf("\n===========================================\n");
		usage();
		return 0;
	}

	return 0;
}
#endif
