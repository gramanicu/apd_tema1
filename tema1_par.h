#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

/**
 * @brief Check if the condition is met. If it doesn't, print message and exit
 */
#define MUST(condition, message)    \
    if (!(condition)) {             \
        fprintf( stderr, message);  \
        exit(-1);                	\
    }

// Defines a complex number
typedef struct _complex {
	double a;
	double b;
} complex;

// Defines the parameters for a program run
typedef struct _params {
	int is_julia, iterations;
	double x_min, x_max, y_min, y_max, resolution;
	complex c_julia;
} params;

// Defines a structure that contains the parameters the program uses
typedef struct _pargs {
	char *in_jul;
	char *in_man;
	char *out_jul;
	char *out_man;
	int threads;
} pargs;

// Defines a int matrix
typedef struct _Matrix {
	int **data;
} Matrix;

// Defines the arguments passed to the threads
typedef struct _thread_args {
	int thread_id;
	int *width;
	int *height;
	Matrix *result;
	pargs pArgs;
	pthread_barrier_t *barrier;
	params *par;
} thread_args;


/**
 * @brief Returns the minimum of two numbers
 * 
 * @param a The first number
 * @param b The second number
 * @return double The smallest of the ones
 */
double min(double a, double b) {
    return a < b ? a : b;
}

// Parse the program arguments
pargs get_args(int argc, char **argv)
{
	if (argc < 6) {
		printf("Numar insuficient de parametri:\n\t"
				"./tema1 fisier_intrare_julia fisier_iesire_julia "
				"fisier_intrare_mandelbrot fisier_iesire_mandelbrot treads\n");
		exit(1);
	}

	pargs args;
	args.in_jul = argv[1];
	args.out_jul = argv[2];
	args.in_man = argv[3];
	args.out_man = argv[4];
	args.threads = atoi(argv[5]);

	return args;
}

// Read from the input files
void read_input_file(char *in_filename, params* par)
{
	FILE *file = fopen(in_filename, "r");
	if (file == NULL) {
		printf("Eroare la deschiderea fisierului de intrare!\n");
		exit(1);
	}

	MUST(fscanf(file, "%d", &par->is_julia) != EOF, "Problem reading data\n");
	MUST(fscanf(file, "%lf %lf %lf %lf",
			&par->x_min, &par->x_max, &par->y_min, &par->y_max) != EOF , 
			"Problem parsing data\n");
	MUST(fscanf(file, "%lf", &par->resolution) != EOF, "Problem parsing data\n");
	MUST(fscanf(file, "%d", &par->iterations) != EOF, "Problem parsing data\n");

	if (par->is_julia) {
		MUST(fscanf(file, "%lf %lf", &par->c_julia.a, &par->c_julia.b) != EOF,
			"Problem parsing data\n");
	}

	fclose(file);
}

// Write data in the output files
void write_output_file(char *out_filename, int **result, int width, int height)
{
	int i, j;

	FILE *file = fopen(out_filename, "w");
	if (file == NULL) {
		printf("Eroare la deschiderea fisierului de iesire!\n");
		return;
	}

	fprintf(file, "P2\n%d %d\n255\n", width, height);
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			fprintf(file, "%d ", result[i][j]);
		}
		fprintf(file, "\n");
	}

	fclose(file);
}

// Allocate memory for the result
int **allocate_memory(int width, int height)
{
	int **result;
	int i;

	result = (int**) malloc(height * sizeof(int*));
	if (result == NULL) {
		printf("Eroare la malloc!\n");
		exit(1);
	}

	for (i = 0; i < height; i++) {
		result[i] = (int*) malloc(width * sizeof(int));
		if (result[i] == NULL) {
			printf("Eroare la malloc!\n");
			exit(1);
		}
	}

	return result;
}

// Free the allocated memory
void free_memory(int **result, int height)
{
	int i;

	for (i = 0; i < height; i++) {
		free(result[i]);
	}
	free(result);
}