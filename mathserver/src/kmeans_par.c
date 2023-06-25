#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#include "configs.h"
#include "tools.h"

typedef struct point
{
  float x;     // The x-coordinate of the point
  float y;     // The y-coordinate of the point
  int cluster; // The cluster that the point belongs to
} point;

struct parallel_thread_states
{
  int from;
  int to;
  bool state_changed_during_threads_operation;
};

int number_of_samples_in_file; // number of entries in the data file
point data[MAX_POINTS];      // Data coordinates
point cluster[MAX_CLUSTERS]; // The coordinates of each cluster center (also
                             // called centroid)
struct argument_flags arg_flag;

int lines_in_fp()
{
  FILE *fp = fopen(arg_flag.INPUT_FILE_NAME, "r");
  int count = 0;
  char buf[200];
  while (fgets(buf, sizeof(buf), fp) != NULL)
  {
    count++;
  }
  return count;
}

void read_data()
{
  FILE *fp = fopen(arg_flag.INPUT_FILE_NAME, "r");
  if (fp == NULL)
  {
    perror("Cannot open the file");
    exit(EXIT_FAILURE);
  }

  number_of_samples_in_file = lines_in_fp();

  // Initialize points from the data file
  // float temp;
  for (int i = 0; i < number_of_samples_in_file; i++)
  {
    fscanf(fp, "%f %f", &data[i].x, &data[i].y);
    data[i].cluster = -1; // Initialize the cluster number to -1
  }

  // Initialize centroids randomly
  srand(0); // Setting 0 as the random number generation seed
  for (int i = 0; i < arg_flag.NUMBER_OF_CLUSTERS; i++)
  {
    int r = rand() % number_of_samples_in_file;
    cluster[i].x = data[r].x;
    cluster[i].y = data[r].y;
  }
  fclose(fp);
}

int get_closest_centroid(int i, int k)
{
  /* find the nearest centroid */
  int nearest_cluster = -1;
  double xdist, ydist, dist, min_dist;
  min_dist = dist = INT_MAX;
  for (int c = 0; c < k; c++)
  { // For each centroid
    // Calculate the square of the Euclidean distance between that centroid and
    // the point
    xdist = data[i].x - cluster[c].x;
    ydist = data[i].y - cluster[c].y;
    dist = xdist * xdist + ydist * ydist; // The square of Euclidean distance
    if (dist <= min_dist)
    {
      min_dist = dist;
      nearest_cluster = c;
    }
  }
  return nearest_cluster;
}

// Used for parallelism.
// Input: parallel_thread_states
// Output: void
void *calculate_cluster_in_parallel(void *params)
{
  struct parallel_thread_states *thread_arguments = (struct parallel_thread_states *)params;

  int from = thread_arguments->from;
  int to = thread_arguments->to;
  int new_cluster_centroid, old_cluster_centroid;
  for (int i = from; i < to; i++)
  {
    old_cluster_centroid = data[i].cluster;
    new_cluster_centroid = get_closest_centroid(i, arg_flag.NUMBER_OF_CLUSTERS);
    data[i].cluster = new_cluster_centroid;
    if (old_cluster_centroid != new_cluster_centroid)
    {
      thread_arguments->state_changed_during_threads_operation = true;
    }
  }
  pthread_exit(NULL);
}

bool assign_clusters_to_points()
{
  struct parallel_thread_states *thread_arguments = malloc(MAX_NUM_THREADS * sizeof(struct parallel_thread_states));
  pthread_t *children = malloc(MAX_NUM_THREADS * sizeof(pthread_t));

  bool state_changed_during_threads_operation = false;

  // create threads for calculating the new cluster for each point
  for (size_t i = 0; i < MAX_NUM_THREADS; i++)
  {
    int from = (int) ceil(i * ((double)number_of_samples_in_file / MAX_NUM_THREADS));
    int to = (int) ceil((i + 1) * ((double)number_of_samples_in_file / MAX_NUM_THREADS));

    // printf("from: %d, to: %d\n", from, to);
    thread_arguments[i].from = from;
    thread_arguments[i].to = to;
    thread_arguments[i].state_changed_during_threads_operation = false;

    pthread_create(&(children[i]),
                   NULL,
                   calculate_cluster_in_parallel,
                   (void *)&(thread_arguments[i]));
  }

  // joining threads
  for (int i = 0; i < MAX_NUM_THREADS; i++)
  {
    pthread_join(children[i], NULL);
  }

  // update data and check if something changed
  for (int i = 0; i < MAX_NUM_THREADS; i++)
  {
    if (thread_arguments[i].state_changed_during_threads_operation)
    {
      state_changed_during_threads_operation = true;
    }
  }

  free(children);
  free(thread_arguments);
  return state_changed_during_threads_operation;
}

void update_cluster_centers()
{
  /* Update the cluster centers */
  int c;
  int count[MAX_CLUSTERS] = {0}; // Array to keep track of the number of points in each cluster
  point temp[MAX_CLUSTERS] = {{0.0}};

  for (int i = 0; i < number_of_samples_in_file; i++)
  {
    // printf("%d\n",i);
    c = data[i].cluster;
    count[c]++;
    temp[c].x += data[i].x;
    temp[c].y += data[i].y;
  }

  for (int i = 0; i < arg_flag.NUMBER_OF_CLUSTERS; i++)
  {
    cluster[i].x = temp[i].x / count[i];
    cluster[i].y = temp[i].y / count[i];
  }
}

void kmeans()
{
  bool somechange;
  int iter = 0;
  do
  {
    iter++; // Keep track of number of iterations
    somechange = assign_clusters_to_points();
    update_cluster_centers();
  } while (somechange);
}

void write_results()
{
  FILE *fp = fopen(arg_flag.OUTPUT_FILE_NAME, "w");
  if (fp == NULL)
  {
    perror("Cannot open the file");
    exit(EXIT_FAILURE);
  }
  else
  {
    for (int i = 0; i < number_of_samples_in_file; i++)
    {
      fprintf(fp, "%.2f %.2f %d\n", data[i].x, data[i].y, data[i].cluster);
    }
  }
}

int main(int argc, char **argv)
{
  parshe_arguments(MODE_AS_KMEANS_PAR, &arg_flag,argc, argv);
  read_data();
  kmeans();
  write_results();
}