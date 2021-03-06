// information gain C-Adaptation
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>		//to check number of supported cores
#include <omp.h>


#include "common.h"
#include "cpmidx_double.h"
#include "blaslapack.h"
#include "randnumber.h"
#include "outputtiming.h"
#include "calculatenorm.h"
#include "calculatemaxchange.h"
#include "getentropy.h"
#include "getinfogainforattribute.h"
#include "infogain.h"
#include "pthread.h"


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

void * infoGainThread(void * arg) {
  //cast arg to infoGainThreadArgument and copy it to variable a
  infoGainThreadArgument a = *((infoGainThreadArgument*) arg);
  
  int i,j;			//loop index variables
  struct timeval start, end;	//timing variables
  double splitinfo = 0.0;
  
  for (i = a.start; i <= a.end; ++i) {
    //gettimeofday(&start, 0);
    
    a.indexedInfoGainSorted[i].val = getInfoGainForAttribute(a.data + i * a.numInstances, a.classColumn, a.numDistClasses, a.classvalueCount, a.numInstances, a.H);
    a.indexedInfoGainSorted[i].idx = i;
    
    splitinfo = getEntropy(a.data + i * a.numInstances, a.numInstances, 0, a.attributevalueCount, a.numInstances);
    if (splitinfo != 0.0) {
      a.indexedGainRatioSorted[i].val = a.indexedInfoGainSorted[i].val / splitinfo;
    }
    else {
      a.indexedGainRatioSorted[i].val = 0.0;
    }
    a.indexedGainRatioSorted[i].idx = i;
    //gettimeofday(&end, 0);
    // save timing in seconds
    //timing[i] = (double) ( ( 1.0 * end.tv_sec - start.tv_sec) + 1E-6 * (1.0 * (end.tv_usec - start.tv_usec)) );

  }  
}

int infoGain(double * data, int m, int n, double * classColumn, int numDistClasses, idx_double * indexedInfoGainSorted, idx_double * indexedGainRatioSorted) {
 
  
  //check number of supported cores
  int numThreads = sysconf( _SC_NPROCESSORS_ONLN );
  int numProcs = omp_get_num_procs();
  int i;			//index variable used in for-loops
  int numInstances = m;		//number of instances in matrix data
  int numAttributes = n;	//number of attributes in matrix data
  double H = 0.0;		//entropy
  double * classvalueCount = (double*) malloc(sizeof(double) * numDistClasses * 2);
  double * attributevalueCount = (double*) malloc(sizeof(double) * m * 2 * numThreads);
  
  // get Entropy of the classification 
  H = getEntropy(classColumn, m, 1, classvalueCount, numDistClasses);


  
  double * timing = (double*) malloc(sizeof(double) * numAttributes * 1);	//timing in seconds
  double splitinfo = 0.0;
  struct timeval start, end;
  
  //TODO start threads and assign the different attributes to them
  
  pthread_t * threads = (pthread_t*) malloc(sizeof(pthread_t) * numThreads);
  int * threadReturnValue = (int*) malloc(sizeof(int) * numThreads);
  infoGainThreadArgument * threadArgument = (infoGainThreadArgument*) malloc(sizeof(infoGainThreadArgument) * numThreads);
  
  int interval = n / numThreads;
  for (i = 0; i < numThreads; ++i) {
    threadArgument[i].start = i * interval;
    threadArgument[i].end = (i+1) * interval -1;
    threadArgument[i].data = data;
    threadArgument[i].classColumn = classColumn;
    threadArgument[i].numDistClasses = numDistClasses;
    threadArgument[i].classvalueCount = classvalueCount;
    threadArgument[i].attributevalueCount = attributevalueCount + i * 2 * numInstances;
    threadArgument[i].numInstances = numInstances;
    threadArgument[i].H = H;
    threadArgument[i].indexedInfoGainSorted = indexedInfoGainSorted;
    threadArgument[i].indexedGainRatioSorted = indexedGainRatioSorted;
  }
  //make sure the last thread runs until the last attribute column
  threadArgument[numThreads - 1].end = n - 1;
    
  
  for (i = 0; i < numThreads; ++i) {  
    threadReturnValue[i] = pthread_create( threads + i, NULL, infoGainThread, (void*) (threadArgument + i));
  }
  for (i = 0; i < numThreads; ++i) {
    pthread_join(threads[i], NULL);
  }
  
//   for (i = 0; i < numAttributes; ++i) {	
//   
//     gettimeofday(&start, 0);
//     
//     indexedInfoGainSorted[i].val = getInfoGainForAttribute(data + i * m, classColumn, numDistClasses, classvalueCount, numInstances, H);
// //TODELETE
// printf("indexedInfoGainSorted[%d].val = %f\n", i, indexedInfoGainSorted[i].val);
//     indexedInfoGainSorted[i].idx = i;
//     //getEntropy(attributeColumn, noClassification, null) --- null since classvalueCount is not needed
//     //TODO allocate second variable "attributevalueCount" of size m times 2
//     splitinfo = getEntropy(data + i * m, m, 0, attributevalueCount, m);
//     if (splitinfo != 0.0) {
//       indexedGainRatioSorted[i].val = indexedInfoGainSorted[i].val / splitinfo;
//     }
//     else {
//       indexedGainRatioSorted[i].val = 0.0;
//     }
//     indexedGainRatioSorted[i].idx = i;
//     gettimeofday(&end, 0);
//     // save timing in seconds
//     timing[i] = (double) ( ( 1.0 * end.tv_sec - start.tv_sec) + 1E-6 * (1.0 * (end.tv_usec - start.tv_usec)) );
// 
//   }
  
  //TODO sort infoGain and GainRatio and store new indices of those values in descending order
  qsort((void*) indexedInfoGainSorted, numAttributes, sizeof(idx_double), cmpidx_double);
  qsort((void*) indexedGainRatioSorted, numAttributes, sizeof(idx_double), cmpidx_double);
  
  
  //free allocated memory
  free(classvalueCount);
  free(attributevalueCount);
  free(timing);
  
  // return 0 if everything worked fine, else return error code
  return 0;
}
