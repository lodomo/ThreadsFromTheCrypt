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

struct program_settings_s {
    char *i_file;
    char *o_file;
    char *d_file;
    int threads;
    int verbose;
} settings_t;

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

static int hash_count = 0;              // Number of hashes to solve
static char **hash_list;                // List of hashes
static char **dict_list;                // List of dictionary words

int main(int argc, char *argv[]) {
    struct program_settings_s settings; // Settings for the program
    char *hashes;                       // Hashes from input file
    char *dictionary;                   // Dictionary from input file
    int dict_count = 0;                 // Number of words in dictionary
    pthread_t *threads;                 // Array of threads
    long thread_id;                     // Thread ID

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
    // Create Threads
    threads = (pthread_t *)malloc(settings.threads * sizeof(pthread_t));
    memset(threads, 0, settings.threads * sizeof(pthread_t));
    for (thread_id = 0; thread_id < settings.threads; ++thread_id) {
        pthread_create(&threads[thread_id], NULL, crack_hash, (void *)thread_id);
    }
    NOISY_DEBUG_PRINT;
    // Join Threads
    for (thread_id = 0; thread_id < settings.threads; ++thread_id) 
    {
        pthread_join(threads[thread_id], NULL);
    }

    // DELETE THIS SECTION WHEN THESE VARIABLES HAVE BEEN USED ELSEWHERE
    // PREVENTS COMPILER WARNINGS
    // DELETE THIS SECTION WHEN DONE

    free(hashes);
    free(dictionary);
    free(hash_list);
    free(dict_list);
    free(threads);
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

void *crack_hash(void *arg) {
    int hash_index = -1;

    for (hash_index = get_next_hash(); hash_index < hash_count; hash_index = get_next_hash()) {
        fprintf(stderr, "Cracking hash %d\n", hash_index);
        fprintf(stderr, "Hash: %s\n", hash_list[hash_index]);
        fprintf(stderr, "Algorithm: %s\n", algorithm_string[detect_algorithm(hash_list[hash_index])]);
    }

    return NULL;
}
