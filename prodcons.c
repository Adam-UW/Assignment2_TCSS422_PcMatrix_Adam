/*
 *  prodcons module
 *  Producer Consumer module
 * 
 *  Implements routines for the producer consumer module based on 
 *  chapter 30, section 2 of Operating Systems: Three Easy Pieces
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 *  Fall 2016
 */

// Include only libraries for this module
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "counter.h"
#include "matrix.h"
#include "pcmatrix.h"
#include "prodcons.h"

// Define Locks and Condition variables here
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t hasCapacity = PTHREAD_COND_INITIALIZER; 
pthread_cond_t hasMatrix  = PTHREAD_COND_INITIALIZER;

// Producer consumer data structures
Matrix * temp;
int next = 0; //where to fill into the buffer
int use = 0;  //where to pull from the buffer
int count = 0; //number of elements in buffer

// Bounded buffer put() get()
int put(Matrix * value)
{
  bigmatrix[next] = value;
  next = (next + 1) % BOUNDED_BUFFER_SIZE;
  count++;
  return next;
}

Matrix * get() 
{
  temp = bigmatrix[use];
  use = (use + 1) % BOUNDED_BUFFER_SIZE;
  count--;
  return temp;
}


// Matrix PRODUCER worker thread
void *prod_worker(void *arg)
{
  int produced, sum;
  Matrix *m = NULL;
  struct prodcons *ProdStats = (ProdConsStats *) arg;
  for (produced = 0; produced < NUMBER_OF_MATRICES; produced++) {
    m = GenMatrixRandom();
    pthread_mutex_lock(&mutex);
    while (count == BOUNDED_BUFFER_SIZE) {
      pthread_cond_wait(&hasCapacity, &mutex);
    }
    put(m);

    sum = SumMatrix(m);
    ProdStats->matrixtotal = ProdStats->matrixtotal + 1;
    ProdStats->sumtotal = sum + ProdStats->sumtotal;
    pthread_cond_signal(&hasMatrix);
    pthread_mutex_unlock(&mutex);
  } 			
  return NULL;
}

// Matrix CONSUMER worker thread
void *cons_worker(void *arg)
{
  Matrix *m1 =NULL;
  Matrix *m2 =NULL;
  Matrix *m3 =NULL;
  struct prodcons *ConsStats = (ProdConsStats *) arg;
  int consumed, consumedsum;
  for (consumed = 0; consumed < NUMBER_OF_MATRICES; consumed++){
    pthread_mutex_lock(&mutex);
    while (count == 0){
	pthread_cond_wait(&hasMatrix, &mutex);
    }
    if ((m1 == NULL) && (m2 == NULL)){
      m1 = get();
      ConsStats->matrixtotal = ConsStats->matrixtotal + 1;
      consumedsum = SumMatrix(m1) + ConsStats->sumtotal;
      ConsStats->sumtotal = consumedsum;
    } else if ( (m1 != NULL) && ( m2 == NULL)){
      m2 = get();
      ConsStats->matrixtotal = ConsStats->matrixtotal + 1;
      consumedsum = SumMatrix(m2) + ConsStats->sumtotal;
      ConsStats->sumtotal = consumedsum;
      m3 = MatrixMultiply(m1,m2);
      ConsStats->multtotal = ConsStats->multtotal + 1;
      if (m3 != NULL){
	DisplayMatrix(m1, stdout);
	printf("    X\n");
	DisplayMatrix(m2,stdout);
	printf("    =\n");
	DisplayMatrix(m3,stdout);
	printf("\n");
       	FreeMatrix(m3);
        FreeMatrix(m2);
	FreeMatrix(m1);
	m1 =NULL;
	m2 =NULL;
	m3 =NULL;
      } else {
        FreeMatrix(m2);  
	m2 =NULL;
      }
    }
    pthread_cond_signal(&hasCapacity); // signal
    pthread_mutex_unlock(&mutex); // unlock
  }
  return NULL;
}

