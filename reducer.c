#include "reducer.h"

int compare_word_entries(const void *a, const void *b) {
    WordEntry *entryA = (WordEntry *)a;
    WordEntry *entryB = (WordEntry *)b;

    if (entryB->count != entryA->count) {
        // Descending sort after the number of file ids
        return entryB->count - entryA->count;
    } else {
        // Alphabetical order
        return strcmp(entryA->word, entryB->word);
    }
}

int compare_ints(const void *a, const void *b) {
    return (*(int *)a) - (*(int *)b);
}

void add_word_to_ch_list(const char *word, file_id_node_t *file_ids, shared_data_t *shared_data, int ch_file_index) {
    // Lock the mutex for this specific file
    pthread_mutex_lock(&shared_data->list_ch_mutexes[ch_file_index]);

    // Get the existing word_list and the number of words in this character file list
    WordEntry *words_list = shared_data->ch_word_lists[ch_file_index];
    int *number_words = &shared_data->ch_word_counts[ch_file_index];

    // Check if the word already exists in the file
    int found = 0;
    for (int i = 0; i < *number_words; i++) {
        if (strcmp(words_list[i].word, word) == 0) {
            
            // Check if the file ids are already assigned to the word
            file_id_node_t *fid_node = file_ids;
            while (fid_node != NULL) {
                int file_id = fid_node->file_id;
                int exists = 0;
                for (int k = 0; k < words_list[i].count; k++) {
                    if (words_list[i].file_ids[k] == file_id) {
                        exists = 1;
                        break;
                    }
                }
                // Else, add the file id
                if (!exists) {
                    words_list[i].file_ids[words_list[i].count++] = file_id;
                }
                fid_node = fid_node->next;
            }
            found = 1;
            break;
        }
    }

    // If the word doesn't exist => add it
    if (!found) {
        strcpy(words_list[*number_words].word, word);
        // Copy file_ids
        file_id_node_t *fid_node = file_ids;
        int idx = 0;
        while (fid_node != NULL) {
            words_list[*number_words].file_ids[idx++] = fid_node->file_id;
            fid_node = fid_node->next;
        }

        // Set the number of file ids assigned to the word
        words_list[*number_words].count = idx;

        // Increment the total number of words in file
        (*number_words)++;
    }

    pthread_mutex_unlock(&shared_data->list_ch_mutexes[ch_file_index]);
}


void sort_and_write_ch_list(shared_data_t *shared_data, int ch_index) {
    WordEntry *word_list = shared_data->ch_word_lists[ch_index];
    int word_count = shared_data->ch_word_counts[ch_index];

    // Sort the word lists after the number of file ids
    qsort(word_list, word_count, sizeof(WordEntry), compare_word_entries);

    // Create the file beginning with the specific character
    char filename[10];
    sprintf(filename, "%c.txt", 'a' + ch_index);

    // Open the file
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening output file\n");
        return;
    }

    for (int i = 0; i < word_count; i++) {
        WordEntry *entry = &word_list[i];
        // Sort the file ids
        qsort(entry->file_ids, entry->count, sizeof(int), compare_ints);

        // Write with the specific output
        fprintf(file, "%s:[", entry->word);
        for (int k = 0; k < entry->count; k++) {
            fprintf(file, "%d%s", entry->file_ids[k], (k < entry->count - 1) ? " " : "");
        }
        fprintf(file, "]\n");
    }

    fclose(file);
}
