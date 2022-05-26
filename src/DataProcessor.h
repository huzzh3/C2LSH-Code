/*
 * DataProcessor.h
 *
 *  Created on: 2011-9-26
 *      Author: Junhao Gan
 */

#ifndef DATAPROCESSOR_H_
#define DATAPROCESSOR_H_

// Read a point from file.
PPointT readPoint(FILE *fileHandle);

// Reads in the data set points from <filename> in the array
// <dataSetPoints>. Each point get a unique number in the field
// <index> to be easily indentified.
void readDataSetFromFile(char *filename);

// Read the data set points to a specified array with size = n.
void readDataSetFromFile(PPointT* pointList, int n, char *filename);

// Output the ground truth file.
void writeGroundTruth(char* queryFilePath, char* groundTruthFilePath, int nQueries);

// Read the ground truth file.
void readGroundTruth(double** groundTruthFileList, int nQueries, char* groundTruthFilePath);

#endif /* DATAPROCESSOR_H_ */
