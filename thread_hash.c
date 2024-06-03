// ############################################################################
// #
// #    Author:      Lorenzo D. Moon
// #    Instructor:  R. Jesse Chaney
// #    Course:      CS-333
// #    Assignment:  Lab 4: Threads from the Crypt
// #    Description:
// #
// ############################################################################

/* thread_hash.h
# define OPTIONS "i:o:d:hvt:n"
# define MICROSECONDS_PER_SECOND 1000000.0
# ifndef NICE_VALUE : define NICE_VALUE 10

// This comes from a terrific Stackover flow posting.
https://stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
# define FOREACH_ALGORITHM(ALGORITHM) \
        ALGORITHM(DES)           \
        ALGORITHM(NT)            \
        ALGORITHM(MD5)           \
        ALGORITHM(SHA256)        \
        ALGORITHM(SHA512)        \
        ALGORITHM(YESCRYPT)      \
        ALGORITHM(GOST_YESCRYPT) \
        ALGORITHM(BCRYPT)        \
        ALGORITHM(ALGORITHM_MAX)

# define GENERATE_ENUM(ENUM) ENUM,
# define GENERATE_STRING(STRING) #STRING,

typedef enum hash_algorithm_e {
    FOREACH_ALGORITHM(GENERATE_ENUM)
} hash_algorithm_t;

static const char *algorithm_string[] = {
    FOREACH_ALGORITHM(GENERATE_STRING)
};
*/

#include "thread_hash.h"
#include <crypt.h>     // Crypt functions
#include <fcntl.h>     // open, O_RDONLY, O_WRONLY, O_TRUNC, O_CREAT
#include <pthread.h>   // Thread functions
#include <stdio.h>     // fprintf, printf, stderr
#include <stdlib.h>    // EXIT_FAILURE, EXIT_SUCCESS, malloc, free
#include <string.h>    // strcmp, strlen, strncpy
#include <sys/mman.h>  // mmap, munmap
#include <sys/stat.h>  // stat, struct stat, S_IRUSR, S_IWUSR, S_IRGRP, S_IROTH
#include <sys/time.h>  // gettimeofday, struct timeval
#include <sys/types.h> // mode_t, off_t, uid_t, gid_t
#include <time.h>      // localtime, strftime, struct tm
#include <unistd.h>    // close, lseek, read, write, STDOUT_FILENO

// Noisy debugger for verbose debugging
#ifdef NOISY_DEBUG
#define NOISY_DEBUG_PRINT                                                      \
    fprintf(stderr, "DEBUG: %s:%s:%d\n", __FILE__, __func__, __LINE__)
#else // NOISY_DEBUG
#define NOISY_DEBUG_PRINT
#endif // NOISY_DEBUG

// Verbose mode prefix and postfix
#define V_PREFIX ">> "
#define V_POSTFIX " <<"

#define FALSE 0
#define TRUE 1

#define READBUF 1024

typedef struct program_settings_s {
    char *i_file;
    char *o_file;
    char *d_file;
    int threads;
    int verbose;
} program_settings_t;

typedef struct thread_stats_s {
    long thread_id;
    int cracked[ALGORITHM_MAX];
    struct timeval start_time;
    struct timeval end_time;
} thread_stats_t;

void default_settings(struct program_settings_s *settings);
void verbose(const char *msg, struct program_settings_s *settings);
void process_options(int argc, char *argv[],
                     struct program_settings_s *settings);
void detect_start_errors(struct program_settings_s *settings);
void help(void);
char *load_file(char *filename, int *word_count);
char **split_string(char *string, int *word_count);
int get_next_hash(void);
int detect_algorithm(char *hash);
void *crack_hash(void *arg);
void get_salt(char *hash, char *salt);
void print_stats(thread_stats_t *stats, int all);
double elapsed_time(struct timeval *t0, struct timeval *t1);

static int hash_count = 0;           // Number of hashes to solve
static char **hash_list;             // List of hashes
static char **dict_list;             // List of dictionary words
static int dict_count = 0;           // Number of words in dictionary
static thread_stats_t *thread_stats; // Array of thread stats
static FILE *output = NULL;

int main(int argc, char *argv[]) {
    struct program_settings_s settings; // Settings for the program
    char *hashes;                       // Hashes from input file
    char *dictionary;                   // Dictionary from input file
    pthread_t *threads;                 // Array of threads
    long thread_id;                     // Thread ID
    thread_stats_t all_stats;           // Stats for all threads

    output = stdout; // Default output to stdout

    NOISY_DEBUG_PRINT;
    default_settings(&settings);
    NOISY_DEBUG_PRINT;
    process_options(argc, argv, &settings);
    NOISY_DEBUG_PRINT;
    detect_start_errors(&settings);
    NOISY_DEBUG_PRINT;
    hashes = load_file(settings.i_file, &hash_count);
    NOISY_DEBUG_PRINT;
    dictionary = load_file(settings.d_file, &dict_count);
    NOISY_DEBUG_PRINT;
    hash_list = split_string(hashes, &hash_count);
    NOISY_DEBUG_PRINT;
    dict_list = split_string(dictionary, &dict_count);
    NOISY_DEBUG_PRINT;
    // Allcate Memory for Threads, Stats, Start, and End Times
    threads = (pthread_t *)malloc(settings.threads * sizeof(pthread_t));
    NOISY_DEBUG_PRINT;
    thread_stats =
        (thread_stats_t *)malloc(settings.threads * sizeof(thread_stats_t));
    NOISY_DEBUG_PRINT;
    memset(threads, 0, settings.threads * sizeof(pthread_t));
    memset(thread_stats, 0, settings.threads * sizeof(thread_stats_t));
    memset(&all_stats, 0, sizeof(thread_stats_t));
    NOISY_DEBUG_PRINT;
    // Create Threads
    all_stats.thread_id = settings.threads;
    gettimeofday(&all_stats.start_time, NULL);
    for (thread_id = 0; thread_id < settings.threads; ++thread_id) {
        pthread_create(&threads[thread_id], NULL, crack_hash,
                       (void *)thread_id);
    }
    NOISY_DEBUG_PRINT;
    // Join Threads
    for (thread_id = 0; thread_id < settings.threads; ++thread_id) {
        pthread_join(threads[thread_id], NULL);
    }

    for (thread_id = 0; thread_id < settings.threads; ++thread_id) {
        for (int i = 0; i < ALGORITHM_MAX; ++i) {
            all_stats.cracked[i] += thread_stats[thread_id].cracked[i];
        }
    }
    gettimeofday(&all_stats.end_time, NULL);
    print_stats(&all_stats, TRUE);

    // Clean up
    if (output != stdout) {
        fclose(output);
    };
    free(hashes);
    free(dictionary);
    free(hash_list);
    free(dict_list);
    free(threads);
    free(thread_stats);
    return EXIT_SUCCESS;
}

void default_settings(struct program_settings_s *settings) {
    settings->i_file = NULL;
    settings->o_file = NULL;
    settings->d_file = NULL;
    settings->threads = 1;
    settings->verbose = 0;
}

void verbose(const char *msg, struct program_settings_s *settings) {
    if (settings->verbose) {
        fprintf(stderr, "%s%s%s\n", V_PREFIX, msg, V_POSTFIX);
    }
}

void process_options(int argc, char *argv[],
                     struct program_settings_s *settings) {
    int opt = -1;

    // Make sure everything is properly sorted before we start
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
    }

    opt = 0;
    optind = 0;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i':
            settings->i_file = optarg;
            break;
        case 'o':
            settings->o_file = optarg;
            break;
        case 'd':
            settings->d_file = optarg;
            break;
        case 'h':
            help();
            exit(EXIT_SUCCESS);
            break;
        case 'v':
            settings->verbose += 1;
            fprintf(stderr, "Verbose mode enabled: Level %d\n",
                    settings->verbose);
            break;
        case 't':
            settings->threads = atoi(optarg);
            break;
        case 'n':
            nice(NICE_VALUE);
            break;
        default:
            fprintf(stderr,
                    "oopsie - unrecognized command line option \"%s\"\n",
                    argv[optind]);
            exit(EXIT_FAILURE);
            break;
        }
    }

    if (settings->o_file) {
        output = fopen(settings->o_file, "w");
        if (output == NULL) {
            fprintf(stderr, "fopen failed on %s\n", settings->o_file);
            exit(EXIT_FAILURE);
        }
    }

    if (settings->verbose) {
        fprintf(stderr, "Running with %d threads\n", settings->threads);
        fprintf(stderr, "Input file: %s\n", settings->i_file);
        fprintf(stderr, "Output file: %s\n", settings->o_file);
        fprintf(stderr, "Dictionary file: %s\n", settings->d_file);
        fprintf(stderr, "Threads: %d\n", settings->threads);
    }
}

void detect_start_errors(struct program_settings_s *settings) {
    if (!settings->d_file) {
        fprintf(stderr, "must give name for dictionary input file with -d\n");
        exit(EXIT_FAILURE);
    }

    if (!settings->i_file) {
        fprintf(stderr,
                "must give name for hashed password input file with -i\n");
        exit(EXIT_FAILURE);
    }

    if (settings->threads < 1) {
        fprintf(stderr, "invalid thread count %d\n", settings->threads);
        exit(EXIT_FAILURE);
    }
}

void help(void) {
    fprintf(
        stderr,
        "help text\n"
        "       ./thread_hash ...\n"
        "       Options: i:o:d:hvt:n\n"
        "               -i file         input file name (required)\n"
        "               -o file         output file name (default stdout)\n"
        "               -d file         dictionary file name (default stdout)\n"
        "               -t #            number of threads to create (default "
        "1)\n"
        "               -v              enable verbose mode\n"
        "               -h              helpful text\n");
}

char *load_file(char *filename, int *word_count) {
    int fd;
    struct stat st;
    char *file_data;
    char buffer[READBUF];
    int bytes_read = 0;

    if ((fd = open(filename, O_RDONLY)) < 0) {
        fprintf(stderr, "open failed on %s\n", filename);
        exit(EXIT_FAILURE);
    }

    if (fstat(fd, &st) < 0) {
        fprintf(stderr, "fstat failed on %s\n", filename);
        exit(EXIT_FAILURE);
    }

    file_data = (char *)malloc(st.st_size + 1);
    memset(file_data, 0, st.st_size + 1);
    if (file_data == NULL) {
        fprintf(stderr, "malloc failed\n");
        exit(EXIT_FAILURE);
    }

    while ((bytes_read = read(fd, buffer, READBUF)) > 0) {
        strncat(file_data, buffer, bytes_read);
    }

    file_data[st.st_size] = '\0';

    // Count newlines to get word count
    for (int i = 0; i < st.st_size; i++) {
        if (file_data[i] == '\n') {
            *word_count += 1;
        }
    }

    close(fd);
    return file_data;
}

char **split_string(char *string, int *word_count) {
    char **words;
    char *word;
    int i = 0;

    words = (char **)malloc(*word_count * sizeof(char *));
    memset(words, 0, *word_count * sizeof(char *));
    if (words == NULL) {
        fprintf(stderr, "malloc failed\n");
        exit(EXIT_FAILURE);
    }

    word = strtok(string, "\n");
    while (word != NULL) {
        words[i] = word;
        i++;
        word = strtok(NULL, "\n");
    }

    return words;
}

int get_next_hash(void) {
    static int next_hash = 0;
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    int cur_row = 0;

    pthread_mutex_lock(&lock);
    cur_row = next_hash++;
    pthread_mutex_unlock(&lock);

    return cur_row;
}

int detect_algorithm(char *hash) {
    /*
    # define FOREACH_ALGORITHM(ALGORITHM) \
        ALGORITHM(DES)           \
        ALGORITHM(NT)            \
        ALGORITHM(MD5)           \
        ALGORITHM(SHA256)        \
        ALGORITHM(SHA512)        \
        ALGORITHM(YESCRYPT)      \
        ALGORITHM(GOST_YESCRYPT) \
        ALGORITHM(BCRYPT)        \
        ALGORITHM(ALGORITHM_MAX)
    */

    if (hash[0] != '$') {
        return DES;
    }

    switch (hash[1]) {
    case '3':
        return NT;
        break;
    case '1':
        return MD5;
        break;
    case '5':
        return SHA256;
        break;
    case '6':
        return SHA512;
        break;
    case 'y':
        return YESCRYPT;
        break;
    case 'g':
        return GOST_YESCRYPT;
        break;
    case '2':
        return BCRYPT;
        break;
    default:
        fprintf(stderr, "Unknown hash algorithm\n");
        exit(EXIT_FAILURE);
        break;
    }
}

/*
struct crypt_data
{
    // THE HASH
    char output[CRYPT_OUTPUT_SIZE];

    // THE SALT
    char setting[CRYPT_OUTPUT_SIZE];

    // THE PASSPHRASE
    char input[CRYPT_MAX_PASSPHRASE_SIZE];

    // NOT NEEDED I DONT THINK
    char reserved[CRYPT_DATA_RESERVED_SIZE];

    // SET TO ZERO
    char initialized;

    // NOT NEEDED TO USE
    char internal[CRYPT_DATA_INTERNAL_SIZE];
};

*/

void *crack_hash(void *arg) {
    // Get the data from the argument and convert it to a long
    long thread_id = (long)arg;
    int hash_index = -1;
    int algo = -1;
    char salt[CRYPT_OUTPUT_SIZE];
    struct crypt_data c_data;

    // Get the start time for the thread
    thread_stats[thread_id].thread_id = thread_id;
    gettimeofday(&thread_stats[thread_id].start_time, NULL);

    for (hash_index = get_next_hash(); hash_index < hash_count;
         hash_index = get_next_hash()) {
        // Step 1: Get the next hash to crack
        // Step 2: Find the algorithm used to create the hash
        // Step 3: Get the salt from the hash
        // Step 4: Hash a word from the dictionary with the salt
        // Step 5: Compare the hash to the hash we are trying to crack
        // Step 6: If the hashes match, we have cracked the hash and can stop
        //         Otherwise go back to step 4
        // Step 7: Print the cracked hash to the output file
        //
        // NOTE TO SELF. GET THE SALT BY STARTING AT THE END.
        char *full_hash = hash_list[hash_index];
        algo = detect_algorithm(full_hash);
        memset(&c_data, 0, sizeof(struct crypt_data));
        memset(salt, 0, CRYPT_OUTPUT_SIZE);
        get_salt(full_hash, salt);
        fprintf(stderr, "thread %2ld: cracking %s\n", thread_id, full_hash);
        for (int i = 0; i < dict_count; ++i) {
            crypt_rn(dict_list[i], salt, &c_data, sizeof(struct crypt_data));
            if (strcmp(full_hash, c_data.output) == 0) {
                fprintf(output, "cracked  %s  %s\n", dict_list[i], full_hash);
                break;
            }
        }
        thread_stats[thread_id].cracked[algo] += 1;
    }

    // Get the end time for the thread
    gettimeofday(&thread_stats[thread_id].end_time, NULL);
    print_stats(&thread_stats[thread_id], FALSE);
    return NULL;
}

void get_salt(char *hash, char *salt) {
    int i = 0;

    // If it's a DES hash
    if (hash[0] != '$') {
        strncpy(salt, hash, 2);
        return;
    }

    // If it's a BCRYPT hash
    if (detect_algorithm(hash) == BCRYPT) {
        strncpy(salt, hash, 29);
        return;
    }

    // If it's any other hash find the $ from the right
    // and copy everything up to it
    for (i = strlen(hash) - 1; i >= 0; --i) {
        if (hash[i] == '$') {
            break;
        }
    }

    strncpy(salt, hash, i);
    return;
}

void print_stats(thread_stats_t *stats, int all) {
    double et = elapsed_time(&stats->start_time, &stats->end_time);
    int total = 0;

    for (int i = 0; i < ALGORITHM_MAX; ++i) {
        total += stats->cracked[i];
    }

    if (all) {
        fprintf(stderr, "total:  %2ld  ", stats->thread_id);
    } else {
        fprintf(stderr, "thread: %2ld  ", stats->thread_id);
    }
    fprintf(stderr, "%7.2f sec ", et);
    for (int i = 0; i < ALGORITHM_MAX; ++i) {
        fprintf(stderr, "%16s:%6d ", algorithm_string[i], stats->cracked[i]);
    }
    fprintf(stderr, " total:%9d\n", total);
}

double elapsed_time(struct timeval *t0, struct timeval *t1) {
    double et =
        (((double)(t1->tv_usec - t0->tv_usec)) / MICROSECONDS_PER_SECOND) +
        ((double)(t1->tv_sec - t0->tv_sec));

    return et;
}
