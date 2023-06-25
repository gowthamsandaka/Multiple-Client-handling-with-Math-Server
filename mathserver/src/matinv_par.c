#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "configs.h"
#include "tools.h"

typedef double matrix[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE];

matrix A;           
matrix I = {{0.0}};
int client_nr = 0;
struct argument_flags arg_flag;

void store_matrix_into_file(matrix M, char file_name[]);
void *matinv_parallel_computations(void *params);

void find_inverse();
void Init_Matrix(void);
void Print_Matrix(matrix M, char name[]);

struct parallel_thread_states
{
    int p;
    int start_row;
    int end_row;
};

void *matinv_parallel_computations(void *params)
{
    struct parallel_thread_states *thread_arguments = (struct parallel_thread_states *)params;
    int start_row = thread_arguments->start_row;
    int end_row = thread_arguments->end_row;
    int p = thread_arguments->p;
    for (int row = start_row; row < end_row; row++)
    {
        double multiplier = A[row][p];
        if (row != p) // skip current pivot row
        {
            for (int col = 0; col < arg_flag.MATRIX_PROBLEM_SIZE; col++)
            {
                A[row][col] = A[row][col] - A[p][col] * multiplier; /* Elimination step on A */
                I[row][col] = I[row][col] - I[p][col] * multiplier; /* Elimination step on I */
            }
            assert(A[row][p] == 0.0); // this will also rise runtime error if 
        }
    }
    pthread_exit(NULL);
}

void find_inverse()
{
    pthread_t *parallel_threads = malloc(MAX_NUM_THREADS * sizeof(pthread_t));
    struct parallel_thread_states *thread_arguments = malloc(MAX_NUM_THREADS * sizeof(struct parallel_thread_states));

    int col;

    /* Bringing the matrix A to the identity form */
    for (int p = 0; p < arg_flag.MATRIX_PROBLEM_SIZE; p++)
    { /* Outer loop */
        double pivalue = A[p][p];
        for (col = 0; col < arg_flag.MATRIX_PROBLEM_SIZE; col++)
        {
            A[p][col] = A[p][col] / pivalue; /* Division step on A */
            I[p][col] = I[p][col] / pivalue; /* Division step on I */
        }
        // if(!(A[p][p]==1.0)){
        //     printf("DIAGONAL [%d][%d]%f\n",p,p,A[p][p]);
        // }
        assert(A[p][p] == 1.0); // raise runtime error if A[p][p] is not 1 

        for (size_t i = 0; i < MAX_NUM_THREADS; i++)
        {

            thread_arguments[i].p = p;
            thread_arguments[i].start_row = (int) ceil(i * ((double)arg_flag.MATRIX_PROBLEM_SIZE / MAX_NUM_THREADS));
            thread_arguments[i].end_row = (int) ceil((i + 1) * ((double)arg_flag.MATRIX_PROBLEM_SIZE / MAX_NUM_THREADS));

            pthread_create(&(parallel_threads[i]),
                           NULL,
                           matinv_parallel_computations,
                           (void *)&thread_arguments[i]); // args to that function
        }

        for (int id = 0; id < MAX_NUM_THREADS; id++)
        {
            pthread_join(parallel_threads[id], NULL); // will wait for the threads to finish one by one
        }
    }
    free(thread_arguments);    // deallocate args vector
    free(parallel_threads); // deallocate array
}

void Init_Matrix()
{
    int row, col;

    // Set the diagonal elements of the inverse matrix to 1.0
    // So that you get an identity matrix to begin with
    for (row = 0; row < arg_flag.MATRIX_PROBLEM_SIZE; row++)
    {
        for (col = 0; col < arg_flag.MATRIX_PROBLEM_SIZE; col++)
        {
            if (row == col)
                I[row][col] = 1.0;
        }
    }

    if (strcmp(arg_flag.MATRIX_INV_INIT_TYPE, "rand") == 0)
    {
        for (row = 0; row < arg_flag.MATRIX_PROBLEM_SIZE; row++)
        {
            for (col = 0; col < arg_flag.MATRIX_PROBLEM_SIZE; col++)
            {
                if (row == col) /* diagonal dominance */
                    A[row][col] = (double)(rand() % arg_flag.MAX_RANDOM_NUMBER) + 5.0;
                else
                    A[row][col] = (double)(rand() % arg_flag.MAX_RANDOM_NUMBER) + 1.0;
            }
        }
    }
    if (strcmp(arg_flag.MATRIX_INV_INIT_TYPE, "fast") == 0)
    {
        for (row = 0; row < arg_flag.MATRIX_PROBLEM_SIZE; row++)
        {
            for (col = 0; col < arg_flag.MATRIX_PROBLEM_SIZE; col++)
            {
                if (row == col) /* diagonal dominance */
                    A[row][col] = 5.0;
                else
                    A[row][col] = 2.0;
            }
        }
    }
}

void store_matrix_into_file(matrix M, char file_name[])
{
    FILE *file_pointer_to_operate;
    file_pointer_to_operate = fopen(file_name, "w");
    if (file_pointer_to_operate == NULL)
    {
        printf("Error! unable to open file [%s] for write mode",arg_flag.OUTPUT_FILE_NAME);
        exit(1);
    }
    int row, col;
    fprintf(file_pointer_to_operate, "Inversed Matrix:\n");
    for (row = 0; row < arg_flag.MATRIX_PROBLEM_SIZE; row++)
    {
        for (col = 0; col < arg_flag.MATRIX_PROBLEM_SIZE; col++)
            fprintf(file_pointer_to_operate, " %5.2f", M[row][col]);
        fprintf(file_pointer_to_operate, "\n");
    }
    fprintf(file_pointer_to_operate, "\n\n");
    fclose(file_pointer_to_operate);
}

void Print_Matrix(matrix M, char name[])
{
    int row, col;

    printf("%s Matrix:\n", name);
    for (row = 0; row < arg_flag.MATRIX_PROBLEM_SIZE; row++)
    {
        for (col = 0; col < arg_flag.MATRIX_PROBLEM_SIZE; col++)
            printf(" %5.2f", M[row][col]);
        printf("\n");
    }
    printf("\n\n");
}

int main(int argc, char **argv)
{
    // parshe arguments and set default values if not given
    parshe_arguments(MODE_AS_MATINV_PAR, &arg_flag, argc, argv); 
    Init_Matrix();            /* Init the matrix	*/
    find_inverse();
    store_matrix_into_file(I, arg_flag.OUTPUT_FILE_NAME);
}