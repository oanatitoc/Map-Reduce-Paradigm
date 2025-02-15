#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "mapper.h"
#include "reducer.h"


void *mapper_thread(void *arg) {
    thread_arg_t *thread_arg = (thread_arg_t *)arg;
    shared_data_t *shared_data = thread_arg->shared_data;
    int mapper_id = thread_arg->id;

    // Init the hash map for this mapper
    HashMap *hash_map = &shared_data->mapper_hashMaps[mapper_id];
    init_hash_map(hash_map);

    while (1) {
        // The index of the next file that has to be parsed
        int file_index;

        // Lock the mutex in order to securely check & update the number of parsed files
        pthread_mutex_lock(&shared_data->mutex);

        // Check if there is no left file to parse
        if (shared_data->parsed_files >= shared_data->num_files) {
            pthread_mutex_unlock(&shared_data->mutex);
            break;
        }

        // Get the index of the file that has to be parsed & increment the number of parsed files
        file_index = shared_data->parsed_files;
        shared_data->parsed_files++;

        pthread_mutex_unlock(&shared_data->mutex);

        // Get the name of the file that is going to be processed
        char *file_name = shared_data->file_list[file_index];
        fprintf(stderr, "Mapper %d processing file: %s\n", mapper_id, file_name);

        // Open the file
        FILE *file = fopen(file_name, "r");
        if (file == NULL) {
            fprintf(stderr, "Error opening file\n");
            break;
        }
        // Make the words as specified (lower case and without special chars) and add them in the hash map
        check_all_words_from_file(file, hash_map, file_index);
        
        fclose(file);
    }

    // Wait for all threads
    pthread_barrier_wait(&shared_data->barrier);

    pthread_exit(NULL);
}

void *reducer_thread(void *arg) {
    thread_arg_t *thread_arg = (thread_arg_t *)arg;
    shared_data_t *shared_data = thread_arg->shared_data;
    int reducer_id = thread_arg->id;

    // Wait for all threads
    pthread_barrier_wait(&shared_data->barrier);

    // Compute the range of buckets that each reducer has to process
    int buckets_per_reducer = HASH_MAP_SIZE / shared_data->num_reducers;
    int extra_buckets = HASH_MAP_SIZE % shared_data->num_reducers;

    int start_bucket = reducer_id * buckets_per_reducer + (reducer_id < extra_buckets ? reducer_id : extra_buckets);
    int end_bucket = start_bucket + buckets_per_reducer + (reducer_id < extra_buckets ? 1 : 0);

    // Get through all mappers' hash maps
    for (int mapper_id = 0; mapper_id < shared_data->num_mappers; mapper_id++) {
        HashMap *hash_map = &shared_data->mapper_hashMaps[mapper_id];

        // Process only the assigned buckets
        for (int bucket = start_bucket; bucket < end_bucket; bucket++) {
            WordNode *current = hash_map->buckets[bucket];
            while (current != NULL) {
                // Get the word that has to be processed
                char *word = current->word;
                file_id_node_t *file_ids = current->file_ids;

                // Get the list index where the word has to be added
                int ch_file_index = word[0] - 'a';

                // Add the word in the specific list
                add_word_to_ch_list(word, file_ids, shared_data, ch_file_index);

                current = current->next;
            }
        }
    }

    // Wait for all reducers to finish to add all words in the lists of words that start with the same character
    pthread_barrier_wait(&shared_data->reducer_barrier);

    // Compute the number of character lists per reducer
    int ch_lists_per_reducer = ALPHABET_SIZE / shared_data->num_reducers;
    int extra_ch_lists = ALPHABET_SIZE % shared_data->num_reducers;

    int start = reducer_id * ch_lists_per_reducer + (reducer_id < extra_ch_lists ? reducer_id : extra_ch_lists);
    int end = start + ch_lists_per_reducer + (reducer_id < extra_ch_lists ? 1 : 0);

    // Sort and write character lists
    for (int i = start; i < end; i++) {
        sort_and_write_ch_list(shared_data, i);
    }

    pthread_exit(NULL);
}



int main(int argc, char *argv[]) {
    int num_mappers, num_reducers;
    char *input_file;
    char file_list[MAX_FILES][MAX_FILE_CHARS];
    int num_files;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <number_of_mappers> <number_of_reducers> <input_file>\n", argv[0]);
        exit(-1);
    }

    num_mappers = atoi(argv[1]);
    num_reducers = atoi(argv[2]);

    if (num_mappers <= 0 || num_reducers <= 0) {
        fprintf(stderr, "Invalid number of mappers or reducers\n");
        exit(-1);
    }

    input_file = argv[3];

    // Parse the input file
    num_files = read_input_file(input_file, file_list);
    if (num_files < 0) {
        fprintf(stderr, "Error reading input file\n");
        exit(-1);
    }

    pthread_t threads[num_mappers + num_reducers]; // All threads
    thread_arg_t thread_args[num_mappers + num_reducers];
    shared_data_t shared_data; // All data that threads have access to

    init_shared_data(&shared_data, num_files, file_list, num_mappers, num_reducers);

    // Create threads depending on their type (mapper or reducer)
    for (int i = 0; i < num_mappers + num_reducers; i++) {
        if (i < num_mappers) {
            thread_args[i].id = i;
            thread_args[i].shared_data = &shared_data;
            if (pthread_create(&threads[i], NULL, mapper_thread, &thread_args[i]) != 0) {
                printf("Failed to create mapper thread");
                exit(EXIT_FAILURE);
            }
        } else {
            int j = i - num_mappers;
            thread_args[i].id = j;
            thread_args[i].shared_data = &shared_data;
            if (pthread_create(&threads[i], NULL, reducer_thread, &thread_args[i]) != 0) {
                printf("Failed to create reducer thread");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    // Joining all threads
    for (int i = 0; i < num_mappers + num_reducers; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            printf("Failed to join mapper thread");
            exit(EXIT_FAILURE);
        }
    }
    
    // Free resources
    free_memory(&shared_data);

    return 0;
}
