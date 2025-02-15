# Map-Reduce Paradigm
#### Titoc Oana Alexandra 333CA

# Assignment #1a Parallel Calculation of an Inverted Index Using the Map-Reduce Paradigm

## Arguments for Threads

Given the implementation constraints such as avoiding global variables and creating a single thread flow for both mappers and reducers, I created a structure called **shared_data_t** in which I added all the information needed by the threads (both mappers and reducers). Additionally, in another structure called **thread_arg_t**, I placed a pointer to the first structure and the index of the current thread; this structure serves as the argument passed to the pthread_create function.

## Implementation Idea

## Mappers:

- The mappers will dynamically split the files they parse. Therefore, we have a variable in **shared_data_t** named **parsed_files** that tells us how many files have been parsed so far. Each mapper makes a mutex lock to check if there are more files to parse and, if so, to update the variable 'parsed_files';
- Each mapper has a hashMap where it will introduce the words it parses, with the key being the word and the value being the list of file IDs in which the current mapper found that word (in case the mapper processes more than one file);
- From the currently parsed file, the mapper reads each word and modifies it to contain only lowercase letters and to have no special characters;
- The final word is added to the mapper's hashmap;
- At the end, a barrier is added that waits for ALL threads, ensuring all mappers finish their tasks before the reducers begin their work.

## Reducers:

- The task of the reducers is to concatenate all the partial lists created by the mappers (in our case, all hashMaps), grouping all words that begin with the same letter and sorting them in descending order based on the number of files in which they appear;
- Initially, in the reducers' function, I added a barrier that waits for all threads to ensure that the mappers have completed their tasks;
- I made it so that all reducers handle the hashmap of a single mapper simultaneously, splitting the buckets from each hashmap (the number of buckets being **HASH_MAP_SIZE**) among the reducers: thus, the reducer with index 0 will always process the information in the buckets from index 0 to HASH_MAP_SIZE/num_reducers of each hashMap;
- In **shared_data_t**, I added a list of lists with words: the first list contains all the words that begin with 'a', the second all the words that begin with 'b', etc.;
- Each reducer processes the words from the buckets assigned to it and locks the mutex allocated to the list where the current word should be inserted, ensuring that another reducer processing a word that starts with the same letter does not overwrite the information;
- In this way, a new word is safely added or the file indices of existing words are updated;
- A barrier is added to wait for all reducers to finish completing the 26 lists (for the 26 letters of the alphabet);
- We equally distribute the 26 lists among the reducers: they must sort the words from their assigned lists and then write them to files named ***character.txt***.
