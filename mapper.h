#ifndef MAPPER_H
#define MAPPER_H

#include "utils.h"

unsigned int hash_function(const char *str);
void init_hash_map(HashMap *hash_map);
void free_hash_map(HashMap *hash_map);
void add_word_to_hash_map(HashMap *hash_map, const char *word, int file_id);
void check_all_words_from_file(FILE *file, HashMap *hash_map, int file_index);

#endif // MAPPER_H