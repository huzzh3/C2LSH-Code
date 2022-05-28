/*
 * CollisionCountLSH_AlgorithmInitializor.cpp
 *
 *  Created on:  2011-03-28
 *  Modified on: 2012-06-08
 *      Author: Junhao Gan
 */

#include "headers.h"

// Calculate p(c)
double CalculatePc(double w, double c) {
	double rtn = 0;
	double temp = w / c;

	rtn = 1 - 2 * normal_cdf(-temp) - (2 / (sqrt(2 * PI) * temp)) * (1 - exp(-temp * temp / 2));

	return rtn;
}

// Calculate <k1> which means the parameter <m_1> in our paper.
int CalculateK1(double a, double ps, double p1) {
	double D = ((1 - a) * log((1 - a) / (1 - p1)) + a * log((a / p1))) / log(10);
	double k = 1 / D * log(1 / (1 - ps));
	//	printf("k1_old:\t%d\n", CEIL(k));
	k = 1 / (2 * (a - p1) * (a - p1)) * log(1 / (1 - ps));
	return CEIL(k);
}

// Calculate <k2> which means the parameter <m_2> in our paper.
int CalculateK2(double a, double b, int m, double p2) {
	//	printf("k2_old:\t%d\n", CEIL(2 * (1 - p2) / ((a - p2) * (a - p2)) * log(m / b)));
	double k = 1 / (2 * (a - p2) * (a - p2)) * log(m / b);
	return CEIL(k);
}

// Calculate <k> which means the parameter <m> in our paper.
void CalculateK(PC2LSH_InitializorT initializor) {
	double a = initializor->a;
	double ps = initializor->ps;
	double p1 = initializor->p1;

	double b = initializor->b;
	int m = initializor->m;
	double p2 = initializor->p2;

	initializor->k1 = CalculateK1(a, ps, p1);
	initializor->k2 = CalculateK2(a, b, m, p2);

	initializor->k = initializor->k1 > initializor->k2 ? initializor->k1 : initializor->k2;
}

// Initialize the hash functions.
void InitializeHashFunction(PC2LSH_InitializorT initializor, bool isExternal) {
	int k = initializor->k;
	int d = initializor->dimension;

	// bEnd = (c^Floor(log(c)(t*d))) * w^2
	double bEnd = (int) ((log(initializor->dimension) + log(initializor->maxCoordinate)) / log(initializor->c));
	bEnd = pow(initializor->c, bEnd) * initializor->w * initializor->w;

	FAILIF(NULL==(initializor->hashFunctionList=(PHashFunctionT)MALLOC(k * sizeof(HashFunction))));
	for (int i = 0; i < k; i++) {
		FAILIF(NULL==(initializor->hashFunctionList[i].a =(double*)MALLOC(d * sizeof(double))));
		// Each dimensional value of vector <a> is chosen from Gaussian Distribution.
		for (int j = 0; j < d; j++) {
			initializor->hashFunctionList[i].a[j] = genGaussianRandom();
		}

		// <b> is randomly chosen from Uniform Distribution U(0, (c^Floor(log(c)(t*d))) * w^2).
		initializor->hashFunctionList[i].b = genUniformRandom(0, bEnd);
	}

	if (isExternal) {
		// Output all the parameters(<a>, <b>) of each hash function to a file.
		WriteHashFunctionToFile(initializor);
	}
}

// Project all data points to <k> hash tables.
void ProjectAllPoints(PC2LSH_InitializorT initializor) {
	// <k> is the parameter <m> in our paper.
	int k = initializor->k;

	double value = 0;
	int intervalID = -1;

	// Projecting process starts.
	for (int i = 0; i < k; i++) {
		NewHashTable aHashTable;
		initializor->hashTableList.push_back(aHashTable);

		// Retrieve all points.
		for (int j = 0; j < initializor->nPoints; j++) {
			value = 0;
			intervalID = -1;

			// Compute the inner product value of <a> * dataPoint.
			for (int m = 0; m < initializor->dimension; m++) {
				value += (initializor->hashFunctionList[i].a[m] * initializor->dataSetPoints[j]->coordinates[m]);
			}
			//			// Scale the coordinate value, but in C2LSH scheme projectScale is always 1.
			//			value *= initializor->projectScale;
			value += initializor->hashFunctionList[i].b;

			value /= initializor->w;

			intervalID = (int) (value);

			// Modified on 2012-06-07 by Junho
			initializor->hashTableList[i][intervalID].push_back(j);
		}

		// Output this hash table to file
		WriteOneHashTable(initializor, i);
		// Clear this hash table from memory
		initializor->hashTableList[i].clear();
		free(initializor->hashFunctionList[i].a);
	}
}

// Write all projected points to file
void WriteProjectedPoints(PC2LSH_InitializorT initializor) {
	// Initialize hash functions
	InitializeHashFunction(initializor, true);

	// Project all points and write them to file
	ProjectAllPoints(initializor);
}

// Read hash functions and offset tables.
void ReadIndexInfo(PC2LSH_InitializorT initializor) {

	ReadHashFunctionFromFile(initializor);

	char filename[1000];
	char buffer[100];
	for (int i = 0; i < initializor->k; i++) {
		strcpy(filename, initializor->folderPath);
		sprintf(buffer, "/%d", i + 1);
		strcat(filename, buffer);

		initializor->fileInfoList[i].file.open(filename, ios_base::in | ios_base::binary);

		FAILIF(!((initializor->fileInfoList[i].file)));
	}

	// Read the offset tables.
	for (int i = 0; i < initializor->k; i++) {
		ReadOneOffsetTable(initializor, i);
	}
}

// Read the hash tables from file.
void ReadHashTables(PC2LSH_InitializorT initializor) {

	ReadIndexInfo(initializor);

	int k = initializor->k;
	int n = initializor->nPoints;

	initializor->hashTables = (int**) MALLOC(k * sizeof(int*));
	FAILIF(NULL == initializor->hashTables);

	for (int i = 0; i < k; i++) {
		initializor->hashTables[i] = (int*) MALLOC(n * SizeIntT);
		FAILIF(NULL == initializor->hashTables[i]);
		// Move the file pointer to the first position of hash table.
		initializor->fileInfoList[i].file.seekg(initializor->offsetTableList[i][0].offset, ios::beg);
		initializor->fileInfoList[i].file.read((char*) (initializor->hashTables[i]), n * SizeIntT);
		initializor->fileInfoList[i].file.close();
	}

	// Update all the offsets to be the index of each point id in the hash table.
	int temp;
	int basic;
	for (int i = 0; i < k; i++) {
		temp = initializor->offsetTableList[i].size();
		basic = initializor->offsetTableList[i][0].offset;
		for (int j = 0; j < temp; j++) {
			initializor ->offsetTableList[i][j].offset = (initializor ->offsetTableList[i][j].offset - basic)
					/ SizeIntT;
		}
	}
}

// Initialize the algorithm parameter structure
void Initialize(PC2LSH_InitializorT initializor, PPointT* dataSetPoints, int nPoints, int dimension,
		double maxCoordinate, char* folderPath, double c, bool useCt) {
	initializor->dataSetPoints = dataSetPoints;
	initializor->nPoints = nPoints;
	initializor->dimension = dimension;
	initializor->maxCoordinate = maxCoordinate;

	strcpy(initializor->folderPath, folderPath);

	// We fix <w> = 1 and <c> = 3
	initializor->w = 1;
	initializor->c = c;

	initializor->p1 = CalculatePc(initializor->w, 1);
	initializor->p2 = CalculatePc(initializor->w, initializor->c);

	initializor->m = 2;

	// We set <beta> to be 100/n
	initializor->b = ((double) 100) / nPoints;

	// <delta> = 1 - ps
	initializor->ps = 0.99;

	double beta = initializor->b;
	double temp = sqrt(log(2 / beta) / log(1 / (1 - initializor->ps)));

	// Parameter <alpha>
	initializor->a = (temp * initializor->p1 + initializor->p2) / (1 + temp);

	// Calculate parameter <m> in our paper
	CalculateK(initializor);

	double p3 = CalculatePc(initializor->w, initializor->c * initializor->c);
	double a = (temp * initializor->p2 + p3) / (1 + temp);
	//	printf("a:\t%lf\np3/p1:\t%lf\n", a, p3 / initializor->p1);
	a = p3 / initializor->p1;
	//	printf("alpha:\t%lf\n", initializor->a);
	//	printf("p3/p1:\t%lf\n", a);

	// Parameter <alpha> * <m>
	initializor->ak = CEIL(initializor->a * initializor->k);
	// Parameter <Ct>
	initializor->Ct = CEIL((double)initializor->ak * a);

	// Modified on 2012-02-26 by Junho
	if (useCt) {
		initializor->countThreshold = initializor->Ct;
	} else {
		initializor->countThreshold = initializor->ak;
	}

	// Parameter <beta> * <n>
	initializor->pointsMetThreshold = CEIL(initializor->b * initializor->nPoints);

	initializor->fileInfoList = new FileInfoItem[initializor->k];

	// Initialize the offset table list
	for (int i = 0; i < initializor->k; i++) {
		vector<OffsetItem> temp;
		initializor->offsetTableList.push_back(temp);
	}

	// Output all parameters
	PrintAlgorithm(initializor);

}

// Print algorithm structure
void PrintAlgorithm(PC2LSH_InitializorT initializor) {
	printf("==================================================\n");
	printf("nPoints:\t%d\n", initializor->nPoints);
	printf("dimension:\t%d\n", initializor->dimension);
	printf("maxCoordinat:\t%0.6lf\n", initializor->maxCoordinate);
	printf("w:\t%0.6lf\n", initializor->w);
	printf("c:\t%0.6lf\n", initializor->c);
	printf("p1:\t%0.6lf\n", initializor->p1);
	printf("p2:\t%0.6lf\n", initializor->p2);
	printf("alpha:\t%0.6lf\n", initializor->a);
	printf("beta:\t%0.9lf\n", initializor->b);
	printf("#false positive:\t%d\n", (int) (initializor->b * initializor->nPoints));
	printf("delta:\t%0.6lf\n", 1 - initializor->ps);
	//	printf("m1:\t%d\n", initializor->k1);
	//	printf("m2:\t%d\n", initializor->k2);
	printf("m:\t%d\n", initializor->k);
	printf("l:\t%d\n", initializor->ak);
	printf("Ct:\t%d\n", initializor->Ct);
	printf("Count Threshold:\t%d\n", initializor->countThreshold);
	printf("==================================================\n");
}

// Read hash functions from file
void ReadHashFunctionFromFile(PC2LSH_InitializorT initializor) {
	fstream file;
	char filename[1000];
	strcpy(filename, initializor->folderPath);
	strcat(filename, "/HashFunctions");

	file.open(filename, ios_base::in | ios::binary);

	int k = initializor->k;
	int d = initializor->dimension;

	FAILIF(NULL==(initializor->hashFunctionList=(PHashFunctionT)MALLOC(k * sizeof(HashFunction))));
	for (int i = 0; i < k; i++) {
		FAILIF(NULL==(initializor->hashFunctionList[i].a =(double*)MALLOC(d * SizeRealT)));

		// Read vector <a>
		for (int j = 0; j < d; j++) {
			file.read((char*) (&(initializor->hashFunctionList[i].a[j])), SizeRealT);
		}
		// Read real number <b>
		file.read((char*) (&(initializor->hashFunctionList[i].b)), SizeRealT);
	}

	file.close();
}

// Output all hash functions to a file.
void WriteHashFunctionToFile(PC2LSH_InitializorT initializor) {
	fstream file;
	char filename[1000];
	strcpy(filename, initializor->folderPath);
	strcat(filename, "/HashFunctions");

	file.open(filename, ios_base::out | ios::binary);

	if (file.good() == false) {
		FAILIF(mkdir(initializor->folderPath,0777)<0);
		file.open(filename, ios_base::out | ios::binary);
		FAILIF(!file);
	}

	int k = initializor->k;
	int d = initializor->dimension;

	for (int i = 0; i < k; i++) {
		for (int j = 0; j < d; j++) {
			file.write((char*) (&(initializor->hashFunctionList[i].a[j])), SizeRealT);
		}
		file.write((char*) (&(initializor->hashFunctionList[i].b)), SizeRealT);
	}

	file.close();
}

// 2011-10-01 add
// Output a hash table to file.
void WriteOneHashTable(PC2LSH_InitializorT initializor, int dimensionIndex) {
	fstream file;

	char buffer[100];

	char hashTableFolder[1000];

	char filename[1000];
	NewHashTable& hashTable = initializor->hashTableList[dimensionIndex];
	//	vector<NewHashTable>& hashTableList = initializor->hashTableList;

	strcpy(hashTableFolder, initializor->folderPath);
	sprintf(buffer, "/%d", dimensionIndex + 1);
	strcpy(filename, hashTableFolder);
	strcat(filename, buffer);

	file.open(filename, ios_base::out | ios::binary);

	if (file.good() == false) {
		FAILIF(mkdir(initializor->folderPath,0777)<0);
		file.open(filename, ios_base::out | ios::binary);
		FAILIF(!file);
	}

	// Use vector to sort hash_map by bucket keys. (Low efficiency!!!!!!!!!!)
	vector<int> bucketIDVector;
	NewHashTable::iterator it_temp = hashTable.begin();

	for (; it_temp != hashTable.end(); it_temp++) {
		bucketIDVector.push_back((int) (it_temp->first));
	}

	// Sorted by bucket ID
	sort(bucketIDVector.begin(), bucketIDVector.end());

	// #bucket in the hash table.
	int n = bucketIDVector.size();

	// Output <n> to file.
	file.write((char*) (&n), SizeIntT);

	// The offset of hash bucket content is
	// (SizeIntT (to store #bucket) + <n> * SizeOffsetItem (offsetTable size)).
	int offset = SizeIntT + n * SizeOffsetItem;

	// Output offset table to file.
	for (int j = 0; j < n; j++) {
		OffsetItem temp;
		temp.bucketID = bucketIDVector[j];
		temp.offset = offset;

		initializor->offsetTableList[dimensionIndex].push_back(temp);

		file.write((char*) (&temp), SizeOffsetItem);
		offset += ((int) hashTable[bucketIDVector[j]].size()) * SizeIntT;
	}
	//printf("\n\n");

	// Output the sorted bucket list to file
	for (int j = 0; j < n; j++) {
		vector<int>& valueList = hashTable[bucketIDVector[j]];
		for (int l = 0; l < (int) valueList.size(); l++) {
			file.write((char*) (&(valueList[l])), SizeIntT);
		}
	}
	file.close();

	bucketIDVector.clear();
}

// 2011-10-01 add
// Read the offset table from file.
void ReadOneOffsetTable(PC2LSH_InitializorT initializor, int dimensionIndex) {
	fstream* file;
	file = &(initializor->fileInfoList[dimensionIndex].file);

	int n = 0;
	file->read((char*) (&n), SizeIntT);

	for (int i = 0; i < n; i++) {
		OffsetItem temp;

		file->read((char*) (&(temp)), SizeOffsetItem);

		initializor->offsetTableList[dimensionIndex].push_back(temp);

	}
}
