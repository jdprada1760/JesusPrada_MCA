#include <stdio.h>
#include <stdlib.h>

/*  
 *  CONSTANTES DEL PROBLEMA
 *  L  -> Tamaño del cuadrado
 *  l  -> Tamaño de las placas
 *  d  -> Separación entre placas
 *  V0 -> DDP entre las placas
 *  m  -> Número de cuadros en L (256)
 *  N  -> Número de iteraciones a realizar
 *  
 */
int L = 5, l = 2, d = 1, V0 = 100, m, N;
double h = 0.02;

void init(int x0, int x1, int y0, int y1, double **array);

int main()
{

	/* 
	 *  x0,x1  -> X posición de las placas
	 *  y0,y1  -> Y posición de la placa de abajo y arriba respectivamente
	 *  i      -> Índice de fila
	 *  j      -> Índice de columna 
	 *  n      -> Progreso general del método n < N
	 *  average-> Promedio actual entre cuadros
	 */
	int x0, x1, y0, y1, i=1, j=1, n=0;
	double average;
	
	// Calcula m y N (parámetro de convergencia perhaps)
	m = L/h;
	N = 2*m*m;
	printf("N=%d,m=%d\n",N,n);
	// Calcula las posiciones (izi)
	x0 = m/2 - l/(h*2) - 1;
	x1 = m/2 + l/(h*2) - 1;
	y0 = m/2 - d/(h*2) - 1;
	y1 = m/2 + d/(h*2) - 1;
	// Allocate memoria para V ( Guarda el potencial actual de la grid )
	double **V = malloc(m*sizeof(double*));
	double **Vtemp = malloc(m*sizeof(double*));
	int a;
	for( a = 0; a < m; a++ ){
		V[a] = malloc(m*sizeof(double));
		Vtemp[a] = malloc(m*sizeof(double));
	}
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

void init(int x0, int x1, int y0, int y1, double **array)
{	
	int a;
	for( a = x0; a <= x1; a++ )
	{
		array[y0][a] = V0/2;
		array[y1][a] = -V0/2;
	}
	for( a = 0; a < m; a++ ){
		array[0][a] = 0;
		array[a][0] = 0;
		array[m-1][a] = 0;
		array[a][m-1] = 0;
	}
}
