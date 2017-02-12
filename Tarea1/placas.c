#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*  
 *  CONSTANTES DEL PROBLEMA
 *  L  -> Tamaño del cuadrado
 *  l  -> Tamaño de las placas
 *  d  -> Separación entre placas
 *  V0 -> DDP entre las placas
 *  m  -> Número de cuadros en L (256)
 *  N  -> Número de iteraciones a realizar
*  s   -> Tamaño vertical de la grid para el procesador
 *  
 */
int L = 5, l = 2, d = 1, V0 = 100, m = 256, N, s;
double h = L/m;

/*
 * MPI Variables
 *
 */
int rank, world_size;

void init(int x0, int x1, int y0, int y1, double **array);
double** allocateMem();

int main(int argc, char** argv)
{
	// Gets world size
  	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	
	/* 
	 *  x0,x1  -> X posición de las placas
	 *  y0,y1  -> Y posición de la placa de abajo y arriba respectivamente
	 *  i      -> Índice de fila
	 *  j      -> Índice de columna 
	 *  n      -> Progreso general del método n < N
	 *  average-> Promedio actual entre cuadros
	 *
	 */
	int x0, x1, y0, y1, i=1, j=1, n=0;
	double average;
	if( rank == 0 || rank == world_size-1 ){
		// El primer y ultimo procesador solo comparte una fila
		s = m/world_size +1;
	}
	else{
		// Los demás procesadores comparten 2		
		s = m/world_size +2;
	}
	
	// Calcula m y N (parámetro de convergencia perhaps)
	N = 2*m*m;
	// printf("N=%d,m=%d\n",N,n);
	// Calcula las posiciones de la placa
	x0 = m/2 - l/(h*2) - 1;
	x1 = m/2 + l/(h*2) - 1;
	y0 = m/2 - d/(h*2) - 1;
	y1 = m/2 + d/(h*2) - 1;
	// Allocate memoria para V ( Guarda el potencial actual de la grid )
	double **V = allocateMem();
	double **Vtemp = allocateMem();

	// Initializes V
	init(x0, x1, y0, y1, V);
	init(x0, x1, y0, y1, Vtemp);
	// Iteration over N
	for( n = 0; n < N; n++ )
	{	
		printf("%d\n",n);
		// Empieza en 1 y termina en m-1 porque el contorno no cambia 		
		for( i=1; i < m-1; i++ )
		{
			for( j=1; j < m-1; j++ )
			{	
				// Verifica que no esté en las placas
				if ( !( j >= x0 && j <= x1 && i == y0 ) && !( j >= x0 && j <= x1 && i == y1 ) )
				{
					average = (V[i-1][j] + V[i+1][j] + V[i][j-1] + V[i][j+1])/4.0;
					// Lo guarda en una variable diferente par evitar conflictos de actualización
					Vtemp[i][j] = average;
				}
			}
		}
		// Actualiza la matriz de posiciones
		for( i=1; i < m-1; i++ )
		{
			for( j=1; j < m-1; j++ )
			{
				V[i][j] = Vtemp[i][j];
			}
		}

	}
	
	for( i=0; i < m; i++ )
	{
		for( j=0; j < m; j++ )
		{
			printf("%f\n", V[i][j]);
		}
	}
	
	// Libera memoria
	for( a = 0; a < m; a++ ){
		free(V[a]);
	}
	free(V);
	return 0;
}

/*
 *  Aparta memoria dependiendo del rank del procesador y el world_size
 */
double** allocateMem(){
	double **temp = malloc(s*sizeof(double*));
	int a;
	for( a = 0; a < s; a++ ){
		V[a] = malloc(m*sizeof(double));
		Vtemp[a] = malloc(m*sizeof(double));
	}
	return temp;
}


/*
 *  Inicializa los valores de la grid
 */
void init(int x0, int x1, int y0, int y1, double **array)
{	
	int a;
	// Barras
	for( a = x0; a <= x1; a++ )
	{	
		// Tamaño vertical de cada procesador (sin compartir)
		int t = m/world_size;
		// Deduce cual procesador contiene y0 e y1
		int range0 = (int) floor(y0/t);
		int range1 = (int) floor(y1/t);
		// Punto auxiliar para saber cual procesador comparte
		int p0 = y0 - t*range0;
		int p1 = y1 - t*range1;
		// Actualiza dependiendo del rank
		if(rank == range0){
			array[p0][a] = V0/2;
		if(rank == range1){
			array[p1][a] = V0/2;
		}
		// Si es 0 o 1 se comparte con el anterior procesador
		if( p0 == 0 || p0 == 1 ){
			// Si es el procesador 1, hay que considerar el cambio de tamaño al procesador 0
			if( (rank == 0) && (range0 == 1)){
				array[t-1+p0][a] = V0/2;	
			}
			else if( rank == range0 - 1 ){
				array[t+p0][a] = V0/2;	
			}
		}
		if( p1 == 0 || p1 == 1 ){
			// Si es el procesador 1, hay que considerar el cambio de tamaño al procesador 0
			if( (rank == 0) && (range1 == 1)){
				array[t-1+p1][a] = V0/2;	
			}
			else if( rank == range1 - 1 ){
				array[t+p1][a] = V0/2;	
			}
		}	
		
	}
	// Bordes
	for( a = 0; a < m; a++ ){
		if( rank == 0 ){
			array[0][a] = 0;
		}
		if( rank == world_size-1 ){
			array[s-1][a] = 0;
		}		
	}
	for( a = 0; a < s; a++ ){
		array[0][a] == 0;
		array[m-1][a] == 0;
	}
}







