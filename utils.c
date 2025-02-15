#include "utils.h"

int read_input_file(const char *input_file, char file_list[MAX_FILES][MAX_FILE_CHARS]) {
    int num_files;
    FILE *fp = fopen(input_file, "r");
    if (fp == NULL) {
        perror("Error opening input file");
        return -1;
    }

    fscanf(fp, "%d", &num_files);

    for (int i = 0; i < num_files; i++)
        fscanf(fp, "%s", file_list[i]);

    fclose(fp);

    return num_files;
}

void init_shared_data(shared_data_t *shared_data,
                      int num_files,
                      char file_list[MAX_FILES][MAX_FILE_CHARS],
                      int num_mappers,
                      int num_reducers) {
    shared_data->num_mappers = num_mappers;
    shared_data->num_reducers = num_reducers;

    // Number of files that have to be parsed
    shared_data->num_files = num_files;

    // Name of each file
    for (int i = 0; i < num_files; i++)
        strcpy(shared_data->file_list[i], file_list[i]);

    // Number of parsed files (by mappers)
    shared_data->parsed_files = 0;

    // Init the mutex used by mappers when they update the parsed_files number
    pthread_mutex_init(&shared_data->mutex, NULL);

    // Init the barrier that waits after all threads in order to let reducers begin the work
    pthread_barrier_init(&shared_data->barrier, NULL, num_mappers + num_reducers);
    // Init the barrier that waits after all reducers to continue to sort all words
    pthread_barrier_init(&shared_data->reducer_barrier, NULL, num_reducers);

    // Each mapper has a hashmap where they save the words processed    
    shared_data->mapper_hashMaps = malloc(num_mappers * sizeof(HashMap));

    // Mutexes used for writing in specific character lists
    shared_data->list_ch_mutexes = malloc(ALPHABET_SIZE * sizeof(pthread_mutex_t));
    
    // List of lists in which words of the same first character will be saved
    shared_data->ch_word_lists = malloc(ALPHABET_SIZE * sizeof(WordEntry *));
    shared_data->ch_word_counts = malloc(ALPHABET_SIZE * sizeof(int)); // the number of lists - 26
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        // List of words from each list
        shared_data->ch_word_lists[i] = malloc(MAX_WORDS * sizeof(WordEntry));
        shared_data->ch_word_counts[i] = 0; // number of words from each list
        // Init the mutex for each character list
        pthread_mutex_init(&shared_data->list_ch_mutexes[i], NULL);
    }
    
}


void free_memory(shared_data_t *shared_data) {
    // Destroy the mutexes and the barriers
    pthread_mutex_destroy(&shared_data->mutex);
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        pthread_mutex_destroy(&shared_data->list_ch_mutexes[i]);
        free(shared_data->ch_word_lists[i]);
    }
    pthread_barrier_destroy(&shared_data->barrier);
    pthread_barrier_destroy(&shared_data->reducer_barrier);
    
    free(shared_data->ch_word_lists);
    free(shared_data->ch_word_counts);
    free(shared_data->list_ch_mutexes);

    // Free hash maps
    for (int mapper_id = 0; mapper_id < shared_data->num_mappers; mapper_id++) {
            free_hash_map(&shared_data->mapper_hashMaps[mapper_id]);
        }
    free(shared_data->mapper_hashMaps);
}