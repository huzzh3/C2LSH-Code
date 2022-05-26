/*
 * CollisionCountLSH_AlgorithmInitializor.h
 *
 *  Created on:  2011-03-27
 *  Modified on: 2012-06-07
 *      Author: Junhao Gan
 */

#ifndef COLLISIONCOUNTLSH_ALGORITHMINITIALIZOR_H_
#define COLLISIONCOUNTLSH_ALGORITHMINITIALIZOR_H_

#include <sys/stat.h>                //mkdir()
#include <sys/types.h>				//mkdir()
#include <fstream>

#include <vector>
#include <map>
// #include <ext/hash_map>
#include <unordered_map>
using namespace __gnu_cxx;
using namespace std;

//#ifndef USE_MAP
//#define HashValueMap unordered_map<int, vector<int> >
//#else
//#define HashValueMap map<int, vector<int> >
//#endif

#define NewHashTable unordered_map<int,vector<int> >

// Hash function structure
typedef struct _HashFunction {
	// Vector a in LSH function
	double *a;

	// Real number b in LSH function
	double b;
} HashFunction, *PHashFunctionT;

// Offset item
typedef struct _OffsetItem {
	// hash bucket ID
	int bucketID;

	// the offset of start position in the hash table file.
	int offset;
} OffsetItem, *POffsetItemT;

// File info item
typedef struct _FileInfoItem {
	// File stream
	fstream file;

	// Left clip
	int left_end;

	// Right clip
	int right_end;

	// Current left pointer
	int left_cur;

	// Current right pointer
	int right_cur;

	_FileInfoItem() {
	}

} FileInfoItem, *PFileInfoItemT;

// Global variables.
DECLARE_EXTERN int SizeFileInfoItem EXTERN_INIT(=sizeof(FileInfoItem));
DECLARE_EXTERN int SizeOffsetItem EXTERN_INIT(=sizeof(OffsetItem));

typedef struct _C2LSH_Initializor {
	// All data points
	PPointT* dataSetPoints;

	// Dataset size
	int nPoints;

	// Dimensionality
	int dimension;

	// The maxmium coordinate value
	double maxCoordinate;

	// Parameter w
	double w;

	// Approximation ratio
	double c;

	// <m_1> in our paper
	int k1;

	// <m_2> in our paper
	int k2;

	// Hash table number(<m> in our paper)
	int k;

	// <p1> of LSH functions
	double p1;

	// <p2> of LSH functions
	double p2;

	// <alpha> in our paper
	double a;

	// The probability of <property_1> which is 1 - <delta>
	double ps;

	// This parameter is fixed to be 2 as a constant in our paper.
	int m;

	// <beta> in our paper which is the allowable false positive percentage.
	double b;

	// <l> = <alpha> * <m> in our paper
	int ak;

	// <Ct> in our paper
	int Ct;

	// The collision count threshold <Ct> or <l>.
	int countThreshold;

	// The least number of candidate to stop. (<k> + <beta> * <n> in our paper)
	int pointsMetThreshold;

	// Hash function list
	PHashFunctionT hashFunctionList;

	// Use in RAM mode.
	// Added on 2012-06-23 by Junho.
	int** hashTables;

	// Hash tables with each item (key, point ID list)
	vector<NewHashTable > hashTableList;

	// Offset table List
	vector<vector<OffsetItem> > offsetTableList;

	// File info list
	PFileInfoItemT fileInfoList;

	// Output file folder path
	char folderPath[100];

} C2LSH_Initializor, *PC2LSH_InitializorT;

/*
 * Initialize the algorithm parameter structure
 */
void Initialize(PC2LSH_InitializorT initializor, PPointT* dataSetPoints, int nPoints, int dimension,
		double maxCoordinate, char* folderPath, double c = 3, bool useCt = true);

/*
 * Write all projected points to file
 */
void WriteProjectedPoints(PC2LSH_InitializorT initializor);

/*
 * Read hash functions and offset tables. (Used in External mode)
 */
void ReadIndexInfo(PC2LSH_InitializorT initializor);

/*
 * Read the hash tables from file. (Used in RAM mode)
 */
void ReadHashTables(PC2LSH_InitializorT initializor);

/*
 * Print the algorithm parameter structure
 */
void PrintAlgorithm(PC2LSH_InitializorT initializor);

/*
 * Read hash functions from file
 */
void ReadHashFunctionFromFile(PC2LSH_InitializorT initializor);

/*
 * Output hash functions to file
 */
void WriteHashFunctionToFile(PC2LSH_InitializorT initializor);

/*
 * Output a hash table to file
 */
void WriteOneHashTable(PC2LSH_InitializorT initializor, int dimensionIndex);

/*
 * Read an offset table from file
 */
void ReadOneOffsetTable(PC2LSH_InitializorT initializor, int dimensionIndex);

#endif /* COLLISIONCOUNTLSH_ALGORITHMINITIALIZOR_H_ */
