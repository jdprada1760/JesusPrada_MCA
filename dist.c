#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define N 100
#define l 3.0
#define ini 1
#define fin 20
#deifine filename "dist.data"

/*
 *  Creates N random numbers between ini and fin according
 *  to the distribution propto exp(-x/l)
 */

int main(){
  // Creates file to write data
  FILE* fil = fopen(filename,"w");
  int i;
  for(i=0;i<N;i+=0){
    //random x
    float x = ini+(fin-ini)*rand()/((float)RAND_MAX);
    // Probability of obtaining x
    float px = (1/l)*exp(-x/(l));
    // Probability auxiliar
    float py = rand()/((float)RAND_MAX);
    // if py <= px it takes the number if not, it tries again
    if( py <= px ){
      fprintf(fil,"%f\n",x);
      i++;
    }
  }
  fclose(fil);
  return 0;
}
