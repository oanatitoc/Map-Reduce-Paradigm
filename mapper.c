#include "mapper.h"

unsigned int hash_function(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash % HASH_MAP_SIZE;
}

void init_hash_map(HashMap *hash_map) {
    hash_map->buckets = malloc(HASH_MAP_SIZE * sizeof(WordNode *));
    for (int i = 0; i < HASH_MAP_SIZE; i++) {
        hash_map->buckets[i] = NULL;
    }
}


void free_hash_map(HashMap *hash_map) {
    for (int i = 0; i < HASH_MAP_SIZE; i++) {

        // Free each bucket
        WordNode *curr_node = hash_map->buckets[i];
        while (curr_node != NULL) {
            WordNode *temp = curr_node;
            curr_node = curr_node->next;

            // Free each file id node
            file_id_node_t *fid_current = temp->file_ids;
            while (fid_current != NULL) {
                file_id_node_t *fid_temp = fid_current;
                fid_current = fid_current->next;
                free(fid_temp);
            }

            free(temp->word);
            free(temp);
        }
    }
    free(hash_map->buckets);
}

void add_word_to_hash_map(HashMap *hash_map, const char *word, int file_id) {
    unsigned int index = hash_function(word);
    WordNode *current = hash_map->buckets[index];

    // Search for the word in bucket
    while (current != NULL) {
        if (strcmp(current->word, word) == 0) {

            // Word found, so search for the file id
            file_id_node_t *fid_node = current->file_ids;
            while (fid_node != NULL) {
                if (fid_node->file_id == file_id) {
                    // File ID exists => do nothing
                    return;
                }
                fid_node = fid_node->next;
            }

            // File ID not found => add it
            file_id_node_t *new_fid_node = malloc(sizeof(file_id_node_t));
            
            // Add it on the first position (faster than on the last position)
            new_fid_node->file_id = file_id;
            new_fid_node->next = current->file_ids;
            current->file_ids = new_fid_node;

            return;
        }
        current = current->next;
    }

    // The word was not found => add a new wordNode (on the first position)
    WordNode *new_word_node = malloc(sizeof(WordNode));
    new_word_node->word = strdup(word);
    new_word_node->next = hash_map->buckets[index];
    hash_map->buckets[index] = new_word_node;

    // Also, add the file id
    file_id_node_t *new_fid_node = malloc(sizeof(file_id_node_t));
    new_fid_node->file_id = file_id;
    new_fid_node->next = NULL;
    new_word_node->file_ids = new_fid_node;
}

void check_all_words_from_file(FILE *file, HashMap *hash_map, int file_index) {
    char word[MAX_WORD_LENGTH];
    while (fscanf(file, "%s", word) != EOF) {
        char final_word[MAX_WORD_LENGTH];
        int j = 0;
        for (int i = 0; word[i] != '\0'; i++) {
            
            // Exclude special characters
            char c = (unsigned char)word[i];
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                final_word[j++] = tolower((unsigned char)word[i]);
            }
        }
        final_word[j] = '\0';

        if (strlen(final_word) > 0) {
            add_word_to_hash_map(hash_map, final_word, file_index + 1);
        }
    }
}
