/*
 * APD - Tema 1
 * Octombrie 2020
 */

#include "tema1_par.h"

void *thread_function(void *arg) {
	thread_args tArgs = *(thread_args*)arg;
	pargs pArgs = tArgs.pArgs;
	params par;

	// -- Julia --
	int start, end;
	int tID = tArgs.thread_id;
	int tCount = tArgs.pArgs.threads; 

	if(tID == 0) {
		// Read input parameters
		read_input_file(pArgs.in_jul, tArgs.par);
		par = *tArgs.par;

		*tArgs.width = (par.x_max - par.x_min) / par.resolution;
		*tArgs.height = (par.y_max - par.y_min) / par.resolution;

		// Allocate memory for result
		tArgs.result->data = allocate_memory(*tArgs.width, *tArgs.height);
	}

	// Wait for the first thread to finish its operations
	pthread_barrier_wait(tArgs.barrier);

	// "Receive data" processed by the first thread
	par = *tArgs.par;
	int width = *tArgs.width;
	int height = *tArgs.height;

	// Compute the start and end indices for the thread
    start = tID * (double) width / tCount;
    end = min((tID + 1) * (double) width / tCount, width);

	// Run Julia algorithm
	for (int w = start; w < end; w++) {
		for (int h = 0; h < height; h++) {
			int step = 0;
			complex z = { .a = w * par.resolution + par.x_min,
							.b = h * par.resolution + par.y_min };

			while (sqrt(pow(z.a, 2.0) + pow(z.b, 2.0)) < 2.0 && 
			step < par.iterations) {
				complex z_aux = { .a = z.a, .b = z.b };

				z.a = pow(z_aux.a, 2) - pow(z_aux.b, 2) + par.c_julia.a;
				z.b = 2 * z_aux.a * z_aux.b + par.c_julia.b;

				step++;
			}

			tArgs.result->data[h][w] = step % 256;
		}
	}

	// Wait for the Julia algorithm to finish
	pthread_barrier_wait(tArgs.barrier);

    start = tID * (double) height / 2 * tCount;
    end = min((tID + 1) * (double) height / 2 * tCount, height / 2);

	// Transform result from mathematical coordinates to screen coordinates
	for (int i = start; i < end; i++) {
		int *aux = tArgs.result->data[i];
		tArgs.result->data[i] = tArgs.result->data[height - i - 1];
		tArgs.result->data[height - i - 1] = aux;
	}

	// Wait for the Julia result to be processed
	pthread_barrier_wait(tArgs.barrier);

	if(tID == 0) {
		// Write output file
		write_output_file(pArgs.out_jul, tArgs.result->data, width, height);

		// Free result memory
		free_memory(tArgs.result->data, height);

		// -- Mandelbrot --
		// Read input parameters
		read_input_file(pArgs.in_man, tArgs.par);
		par = *tArgs.par;

		*tArgs.width = (par.x_max - par.x_min) / par.resolution;
		*tArgs.height = (par.y_max - par.y_min) / par.resolution;

		// Allocate memory for result
		tArgs.result->data = allocate_memory(*tArgs.width, *tArgs.height);
	}

	// Wait for the first thread to finish its operations :
	// print Julia result, free memory, read data for the Mandelbrot algorithm 
	pthread_barrier_wait(tArgs.barrier);

	// "Receive data" processed by the first thread
	par = *tArgs.par;
	width = *tArgs.width;
	height = *tArgs.height;

    start = tID * (double) width / tCount;
    end = min((tID + 1) * (double) width / tCount, width);

	// Run Mandelbrot algorithm
	for (int w = start; w < end; w++) {
		for (int h = 0; h < height; h++) {
			complex c = { .a = w * par.resolution + par.x_min,
							.b = h * par.resolution + par.y_min };
			complex z = { .a = 0, .b = 0 };
			int step = 0;

			while (sqrt(pow(z.a, 2.0) + pow(z.b, 2.0)) < 2.0 && 
			step < par.iterations) {
				complex z_aux = { .a = z.a, .b = z.b };

				z.a = pow(z_aux.a, 2.0) - pow(z_aux.b, 2.0) + c.a;
				z.b = 2.0 * z_aux.a * z_aux.b + c.b;

				step++;
			}

			tArgs.result->data[h][w] = step % 256;
		}
	}

	// Wait for the Mandelbrot algorithm to finish
	pthread_barrier_wait(tArgs.barrier);

    start = tID * (double) height / 2 * tCount;
    end = min((tID + 1) * (double) height / 2 * tCount, height / 2);

	// Transform result from mathematical coordinates to screen coordinates
	for (int i = start; i < end; i++) {
		int *aux = tArgs.result->data[i];
		tArgs.result->data[i] = tArgs.result->data[height - i - 1];
		tArgs.result->data[height - i - 1] = aux;
	}

	// Wait for the Julia result to be processed
	pthread_barrier_wait(tArgs.barrier);

	if(tID == 0) {
		// Write output file
		write_output_file(pArgs.out_man, tArgs.result->data, width, height);

		// Free result memory
		free_memory(tArgs.result->data, height);
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	Matrix result;
	int width, height;
	params par;

	// Parse program arguments
	pargs args = get_args(argc, argv);

	int tCount = args.threads;

	pthread_t tid[tCount];
	thread_args tArgs[tCount];

	// Initalises the barrier
	pthread_barrier_t barrier;

	pthread_barrier_init(&barrier, NULL, tCount);
	
	// Initialises threads
	for (int i = 0; i < tCount; i++) {
		tArgs[i].thread_id = i;
		tArgs[i].pArgs = args;
		tArgs[i].barrier = &barrier;
		tArgs[i].result = &result;
		tArgs[i].par = &par;
		tArgs[i].width = &width;
		tArgs[i].height = &height;
		pthread_create(&tid[i], NULL, thread_function, &tArgs[i]);
	}

	// Wait for threads to end
	for (int i = 0; i < tCount; i++) {
		pthread_join(tid[i], NULL);
	}
	
	// Destroy the barrier
	pthread_barrier_destroy(&barrier);

	return 0;
}
