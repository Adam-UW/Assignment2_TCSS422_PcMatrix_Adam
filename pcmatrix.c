/*
 *  pcmatrix module
 *  Primary module providing control flow for the pcMatrix program
 * 
 *  Creates producer and consumer threads and launches them to produce matrices
 *  and consume matrices.  Each thread produces a total sum of the value of
 *  randomly generated elements.  Producer sum and consumer sum must match.
 *
 *  University of Washington, Tacoma
 *  TCSS 422 - Operating Systems
 *  Fall 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include "matrix.h"
#include "counter.h"
#include "prodcons.h"
#include "pcmatrix.h"


int main (int argc, char * argv[])
{

  int numw = NUMWORK;

   if (argc==1)
  {
    BOUNDED_BUFFER_SIZE=MAX;
    NUMBER_OF_MATRICES=LOOPS;
    MATRIX_MODE=DEFAULT_MATRIX_MODE;
    printf("USING DEFAULTS: worker_threads=%d bounded_buffer_size=%d matricies=%d matrix_mode=%d\n",numw,BOUNDED_BUFFER_SIZE,NUMBER_OF_MATRICES,MATRIX_MODE);
  }
  else
  {
    if (argc==2)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=MAX;
      NUMBER_OF_MATRICES=LOOPS;
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==3)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=LOOPS;
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==4)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=atoi(argv[3]);
      MATRIX_MODE=DEFAULT_MATRIX_MODE;
    }
    if (argc==5)
    {
      numw=atoi(argv[1]);
      BOUNDED_BUFFER_SIZE=atoi(argv[2]);
      NUMBER_OF_MATRICES=atoi(argv[3]);
      MATRIX_MODE=atoi(argv[4]);
    }
    printf("USING: worker_threads=%d bounded_buffer_size=%d matricies=%d 	       matrix_mode=%d\n",numw,BOUNDED_BUFFER_SIZE,NUMBER_OF_MATRICES,MATRIX_MODE);
  }

  bigmatrix = (Matrix **) malloc (sizeof(Matrix *) * BOUNDED_BUFFER_SIZE);

  time_t t;
 
  // Seed the random number generator with the system time
  srand((unsigned) time(&t));

  // Create the threads
  pthread_t Prodthreads[numw];
  pthread_t Consthreads[numw];

  //our stats for each thread
  ProdConsStats* ProdStats = malloc(sizeof(ProdConsStats));
  ProdConsStats* ConsStats = malloc(sizeof(ProdConsStats));

  //initialize the stats
  ProdStats->sumtotal=0, ProdStats->multtotal=0, ProdStats->matrixtotal=0;
  ConsStats->sumtotal=0, ConsStats->multtotal=0, ConsStats->matrixtotal=0;

  printf("Producing %d matrices in mode %d.\n", NUMBER_OF_MATRICES, MATRIX_MODE);
  printf("Using a shared buffer of size=%d\n", BOUNDED_BUFFER_SIZE);
  printf("With %d producer and consumer thread(s).\n",numw);
  printf("\n");

  //create the threads
  for (int i = 0; i < numw; i++) {
  	pthread_create(&Prodthreads[i], NULL, prod_worker, ProdStats);
 	pthread_create(&Consthreads[i], NULL, cons_worker, ConsStats);
  }

  //wait for completion for the parents to continue
  for (int i = 0; i < numw; i++) {
	pthread_join(Prodthreads[i], NULL);
	pthread_join(Consthreads[i], NULL);
  }
  //set all of the totals
  int prs = ProdStats->sumtotal;
  int cos = ConsStats->sumtotal;
  int prodtot = ProdStats->matrixtotal;
  int constot = ConsStats->matrixtotal;
  int consmul = ConsStats->multtotal;

  printf("Sum of Matrix elements --> Produced=%d = Consumed=%d\n",prs,cos);
  printf("Matrices produced=%d consumed=%d multiplied=%d\n",prodtot,constot,consmul);

  return 0;
}
