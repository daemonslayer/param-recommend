#ifndef UNIVIE_NMFLIB_GETATTRVALVSCLASSOCCURENCE_H
#define UNIVIE_NMFLIB_GETATTRVALVSCLASSOCCURENCE_H

/**
  * InfoGain calculates the InformationGain and GainRatio of the matrix "data" according to the classification given by classColumn
  * 
  * Parameter
  * data		double*		IN, matrix to calculate IG and GR for, instance times attribute matrix, attributes stored column wise
  * m			int		IN, first dimension of matrix data (rows)
  * n			int		IN, second dimension of matrix data (columns)
  * classColumn 	int*		IN, array of class attributes for every entity (row) in data
  * numDistClasses	int		IN, number of distinct classes for the instances in matrix "data"
  * indexInfoGain	int*		OUT, array of index of attributes sorted descendingly wrt information gain
  * infoGainSorted	double*		OUT, array of the according information gains of these attributes
  * indexGainRatio	int*		OUT, arry of index of attributes sorted descendingly wrt gain ratio
  * gainRatioSorted	double*		OUT, array of the according gain ratio of these attributes
  *
  */

//note numDistClasses is the number of rows of classvalueCount, in case of isClass = 0 => it is _not_ the number of unique values in classColumn
void getattrValVsClassOccurence(double * attrCol, int m, double * classCol, int numDistClasses, double * classvalueCount, double * uniqueAttrVals, int * numUniqueAttrVals, int * classCountForAttrIndex);

#endif