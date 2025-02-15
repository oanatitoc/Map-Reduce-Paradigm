#ifndef REDUCER_H
#define REDUCER_H

#include "utils.h"

int compare_word_entries(const void *a, const void *b);
int compare_ints(const void *a, const void *b);
void add_word_to_ch_list(const char *word, file_id_node_t *file_ids, shared_data_t *shared_data, int ch_index);
void sort_and_write_ch_list(shared_data_t *shared_data, int ch_index);

#endif