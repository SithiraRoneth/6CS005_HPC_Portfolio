#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#define MAX_WORD_LEN 100
#define MAX_WORDS 10000
#define MAX_UNIQUE_WORDS 5000

typedef struct {
    char word[MAX_WORD_LEN];
    int count;
} WordCount;

char words[MAX_WORDS][MAX_WORD_LEN];
int total_words = 0;

WordCount final_count[MAX_UNIQUE_WORDS];
int final_unique_count = 0;

int next_index = 0;
int slice_size = 10;

pthread_mutex_t index_lock;
pthread_mutex_t result_lock;

void clean_word(char original[], char cleaned[]) {
    int j = 0;

    for (int i = 0; original[i] != '\0'; i++) {
        if (isalnum((unsigned char)original[i])) {
            cleaned[j] = tolower((unsigned char)original[i]);
            j++;
        }
    }

    cleaned[j] = '\0';
}

int read_input_file(char file_name[]) {
    FILE *file = fopen(file_name, "r");

    if (file == NULL) {
        printf("Error: Cannot open input file.\n");
        return 0;
    }

    char temp[MAX_WORD_LEN];
    char cleaned[MAX_WORD_LEN];

    while (fscanf(file, "%99s", temp) == 1) {
        clean_word(temp, cleaned);

        if (strlen(cleaned) > 0) {
            strcpy(words[total_words], cleaned);
            total_words++;

            if (total_words >= MAX_WORDS) {
                break;
            }
        }
    }

    fclose(file);
    return 1;
}

void add_word(WordCount list[], int *unique_count, char word[]) {
    for (int i = 0; i < *unique_count; i++) {
        if (strcmp(list[i].word, word) == 0) {
            list[i].count++;
            return;
        }
    }

    strcpy(list[*unique_count].word, word);
    list[*unique_count].count = 1;
    (*unique_count)++;
}

void add_word_with_count(char word[], int count) {
    for (int i = 0; i < final_unique_count; i++) {
        if (strcmp(final_count[i].word, word) == 0) {
            final_count[i].count += count;
            return;
        }
    }

    strcpy(final_count[final_unique_count].word, word);
    final_count[final_unique_count].count = count;
    final_unique_count++;
}

void *count_words(void *arg) {
    WordCount local_count[MAX_UNIQUE_WORDS];
    int local_unique_count = 0;

    while (1) {
        int start;
        int end;

        pthread_mutex_lock(&index_lock);

        if (next_index >= total_words) {
            pthread_mutex_unlock(&index_lock);
            break;
        }

        start = next_index;
        end = start + slice_size;

        if (end > total_words) {
            end = total_words;
        }

        next_index = end;

        pthread_mutex_unlock(&index_lock);

        for (int i = start; i < end; i++) {
            add_word(local_count, &local_unique_count, words[i]);
        }
    }

    pthread_mutex_lock(&result_lock);

    for (int i = 0; i < local_unique_count; i++) {
        add_word_with_count(local_count[i].word, local_count[i].count);
    }

    pthread_mutex_unlock(&result_lock);

    return NULL;
}

int compare_words(const void *a, const void *b) {
    WordCount *word1 = (WordCount *)a;
    WordCount *word2 = (WordCount *)b;

    return strcmp(word1->word, word2->word);
}

void write_result_file(char file_name[], int thread_count) {
    FILE *file = fopen(file_name, "w");

    if (file == NULL) {
        printf("Error: Cannot create result file.\n");
        return;
    }

    fprintf(file, "Word Occurrence Counting using Pthread\n");
    fprintf(file, "Total Words: %d\n", total_words);
    fprintf(file, "Threads Used: %d\n", thread_count);
    fprintf(file, "--------------------------------------\n");
    fprintf(file, "%-20s %s\n", "Word", "Frequency");
    fprintf(file, "--------------------------------------\n");

    for (int i = 0; i < final_unique_count; i++) {
        fprintf(file, "%-20s %d\n", final_count[i].word, final_count[i].count);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <number_of_threads>\n", argv[0]);
        return 1;
    }

    char *input_file = argv[1];
    int thread_count = atoi(argv[2]);

    if (thread_count <= 0) {
        printf("Error: Number of threads must be greater than 0.\n");
        return 1;
    }

    if (!read_input_file(input_file)) {
        return 1;
    }

    if (total_words == 0) {
        printf("Error: No words found in input file.\n");
        return 1;
    }

    if (thread_count > total_words) {
        thread_count = total_words;
    }

    slice_size = total_words / thread_count;

    if (slice_size < 1) {
        slice_size = 1;
    }

    pthread_t threads[thread_count];

    pthread_mutex_init(&index_lock, NULL);
    pthread_mutex_init(&result_lock, NULL);

    for (int i = 0; i < thread_count; i++) {
        pthread_create(&threads[i], NULL, count_words, NULL);
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    qsort(final_count, final_unique_count, sizeof(WordCount), compare_words);

    write_result_file("6CS005_HPC_Assignment/Task1_Pthread_WordCount/result.txt", thread_count);

    pthread_mutex_destroy(&index_lock);
    pthread_mutex_destroy(&result_lock);

    printf("Task 1 completed successfully.\n");
    printf("Output written to result.txt\n");

    return 0;
}
