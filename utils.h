#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>

#define MAX_FILES 500
#define MAX_FILE_CHARS 500
#define MAX_WORDS 100000

#define MAX_WORD_LENGTH 500
#define ALPHABET_SIZE 26


#define MAX_WORDS_IN_FILE 100000


#define HASH_MAP_SIZE 10007 // Prime number

typedef struct file_id_node {
    int file_id;
    struct file_id_node *next;
} file_id_node_t;

typedef struct WordNode {
    char *word;
    file_id_node_t *file_ids;
    struct WordNode *next;
} WordNode;

typedef struct {
    WordNode **buckets;
} HashMap;


typedef struct {
    char word[MAX_WORD_LENGTH]; // The word
    int file_ids[MAX_FILES];    // Word's file ids
    int count;                  // Number of word's file ids
} WordEntry;

typedef struct {
    int num_mappers;
    int num_reducers;
    int num_files;
    char file_list[MAX_FILES][MAX_FILE_CHARS];
    int parsed_files;
    pthread_mutex_t mutex;
    pthread_barrier_t barrier;
    pthread_barrier_t reducer_barrier;
    HashMap *mapper_hashMaps;
    pthread_mutex_t *list_ch_mutexes;
    WordEntry **ch_word_lists;
    int *ch_word_counts; 
} shared_data_t;


typedef struct {
    int id;
    shared_data_t *shared_data;
} thread_arg_t;


int read_input_file(const char *input_file, char file_list[MAX_FILES][MAX_FILE_CHARS]);
void init_shared_data(shared_data_t *shared_data,
                      int num_files,
                      char file_list[MAX_FILES][MAX_FILE_CHARS],
                      int num_mappers,
                      int num_reducers);
void free_hash_map(HashMap *hash_map);
void free_memory(shared_data_t *shared_data);



#endif // UTILS_H
