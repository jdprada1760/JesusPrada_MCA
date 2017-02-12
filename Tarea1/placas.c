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

/*
 * MPI Variables
 *
 */
int rank, world_size;

int tr(int i, int j);
void init(int x0, int x1, int y0, int y1, double *array);
double* allocateMem();

int main(int argc, char** argv)
{
	double h = L/m;
	// Send and receive requests, status
	MPI_Request send_req, rec_req, send_req2, rec_req2;
  	MPI_Status status;

	// Gets world size and rank
  	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
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
	double *V = allocateMem();
	double *Vtemp = allocateMem();
	// Si el procesador es 0, crea un Vtot para guardar todos los demás

	double *Vtot;
	if( rank == 0 ){
		Vtot = malloc(m*m*sizeof(double));
	}
	// Initializes V
	init(x0, x1, y0, y1, V);
	init(x0, x1, y0, y1, Vtemp);
	// Iteration over N
	for( n = 0; n < N; n++ )
	{	
		// printf("%d\n",n);
		// Empieza en 1 y termina en s-1 porque el porque los extremos no se actualizan 		
		for( i=1; i < s-1; i++ )
		{
			for( j=1; j < m-1; j++ )
			{	
				// Verifica que no esté en las placas
				// Tamaño vertical de cada procesador (sin compartir)
				int t = (int) m/world_size;
				// Deduce cual procesador contiene y0 e y1
				int range0 = (int) floor(y0/t);
				int range1 = (int) floor(y1/t);
				// Punto auxiliar para saber cual procesador comparte
				int p0 = y0 - t*range0;
				int p1 = y1 - t*range1;
				if ( (rank != range0) && (rank != range0-1) && (rank != range1) && (rank != range1-1) )
				{	
					average = (V[tr(i-1,j)] + V[tr(i+1,j)] + V[tr(i,j-1)] + V[tr(i,j+1)])/4.0;
					// Lo guarda en una variable diferente par evitar conflictos de actualización
					Vtemp[tr(i,j)] = average;
				}
				else{
					if( rank == range0 ){
						if( !(( i == p0 ) && ( j >= x0 ) && ( j <= x1 ))){
							average = (V[tr(i-1,j)] + V[tr(i+1,j)] + V[tr(i,j-1)] + V[tr(i,j+1)])/4.0;
							// Lo guarda en una variable diferente par evitar conflictos de actualización
							Vtemp[tr(i,j)] = average;
						}
					}
					else if( (rank == range0-1) && ((p0 == 0) || (p0 == 1)) ){
						p0 = s-2+p0;
						if( !(( i == p0 ) && ( j >= x0 ) && ( j <= x1 ))){
							average = (V[tr(i-1,j)] + V[tr(i+1,j)] + V[tr(i,j-1)] + V[tr(i,j+1)])/4.0;
							// Lo guarda en una variable diferente par evitar conflictos de actualización
							Vtemp[tr(i,j)] = average;
						}
					}
					else if( rank == range1 ){
						if( !(( i == p1 ) && ( j >= x0 ) && ( j <= x1 ))){
							average = (V[tr(i-1,j)] + V[tr(i+1,j)] + V[tr(i,j-1)] + V[tr(i,j+1)])/4.0;
							// Lo guarda en una variable diferente par evitar conflictos de actualización
							Vtemp[tr(i,j)] = average;
						}
					}
					else if( (rank == range1-1) && ((p1 == 0) || (p1 == 1)) ){
						p1 = s-2+p1;
						if( !(( i == p1 ) && ( j >= x0 ) && ( j <= x1 ))){
							average = (V[tr(i-1,j)] + V[tr(i+1,j)] + V[tr(i,j-1)] + V[tr(i,j+1)])/4.0;
							// Lo guarda en una variable diferente par evitar conflictos de actualización
							Vtemp[tr(i,j)] = average;
						}
					}				
				}
			}
		}
		// Actualiza la matriz de posiciones
		for( i=1; i < s-1; i++ )
		{
			for( j=1; j < m-1; j++ )
			{
				V[tr(i,j)] = Vtemp[tr(i,j)];
			}
		}
		// Comunica entre procesadores la actualización
		if( rank == 0 ){
			// Intercambia con el siguiente
			MPI_Isend(&V[tr(s-2,0)], m, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, &send_req);
			MPI_Irecv(&V[tr(s-1,0)], m, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, &rec_req);
		}
		else if( rank == world_size-1 ){
			// Intercambia con el anterior
			MPI_Isend(&V[tr(1,0)], m, MPI_DOUBLE, world_size-2, 0, MPI_COMM_WORLD, &send_req);
			MPI_Irecv(&V[tr(0,0)], m, MPI_DOUBLE, world_size-2, 0, MPI_COMM_WORLD, &rec_req);
		}
		else{
			// Intercambia con el siguiente
			MPI_Isend(&V[tr(s-2,0)], m, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, &send_req);
			MPI_Irecv(&V[tr(s-1,0)], m, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, &rec_req);
			// Intercambia con el anterior
			MPI_Isend(&V[tr(1,0)], m, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, &send_req2);
			MPI_Irecv(&V[tr(0,0)], m, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, &rec_req2);
			MPI_Wait(&send_req2, &status);
  			MPI_Wait(&recv_req2, &status);
		}
		MPI_Wait(&send_req1, &status);
  		MPI_Wait(&rec_req1, &status);

	}
	// Guarda la información en el Vtot
	if( rank == 0 ){
		// Guarda la información propia
		for( i = 0; i < s-1 ; i ++ ){
			for( j = 0; j < m-1; j++){
				Vtot[m*i+j] = V[tr(i,j)];
			}
		}
		// Obtiene la información de cada procesador
		for( i = 1; i < world_size; i++ ){
			MPI_Irecv(&Vtot[i*(s-1)*m], m*(s-1), MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &recv_req);
		}
	}
	else{
		MPI_Isend(&V[tr(1,0)], m*(s-2), MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, &send_req2);
	}

	
	for( i=0; i < m*m; i++ )
	{
		printf("%f\n", Vtot[i]);
	}
	
	// Libera memoria
	free(V);
	MPI_Barrier(MPI_COMM_WORLD);
  	MPI_Finalize();
}

/*
 *  Aparta memoria dependiendo del rank del procesador y el world_size
 */
double* allocateMem(){
	double *temp = malloc(s*m*sizeof(double*));
	return temp;
}

/*
 *  Inicializa los valores de la grid
 */
void init(int x0, int x1, int y0, int y1, double *array)
{	
	int a;
	// Barras
	for( a = x0; a <= x1; a++ )
	{	
		// Tamaño vertical de cada procesador (sin compartir)
		int t = m/world_size;
		// Deduce cual procesador contiene y0 e y1
		int range0 = (int) floor((y0+1)/t);
		int range1 = (int) floor((y1+1)/t);
		// Punto auxiliar para saber cual procesador comparte
		int p0 = y0 - t*range0;
		int p1 = y1 - t*range1;
		// Actualiza dependiendo del rank
		if(rank == range0){
			array[tr(p0,a)] = V0/2;
		}
		if(rank == range1){
			array[tr(p1,a)] = V0/2;
		}
		// Si es 0 o 1 se comparte con el anterior procesador
		if( p0 == 0 || p0 == 1 ){
			if( rank == range0 - 1 ){
				array[tr(s-2+p0,a)] = V0/2;	
			}
		}
		if( p1 == 0 || p1 == 1 ){
			if( rank == range1 - 1 ){
				array[tr(s-2+p1,a)] = V0/2;	
			}
		}	
		
	}
	// Bordes
	for( a = 0; a < m; a++ ){
		if( rank == 0 ){
			array[tr(0,a)] = 0;
		}

		if( rank == world_size-1 ){
			array[tr(s-1,a)] = 0;
		}		
	}
	for( a = 0; a < s; a++ ){
		array[tr(0,a)] = 0;
		array[tr(m-1,a)] = 0;
	}
}


/*
 * Transforma i,j -> index
 */ 
int tr(int i, int j){
	return m*i+j;
}





