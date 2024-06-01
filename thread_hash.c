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

#define MAX_FILE_NAME 256

struct program_settings_s {
    char *input_file;
    char *output_file;
    char *dictionary;
    int threads;
    int verbose;
} settings_t;

void verbose(const char *msg, struct program_settings_s *settings);
void process_options(int argc, char *argv[],
                     struct program_settings_s *settings);
void detect_start_errors(struct program_settings_s *settings);
void help(void);
char **ragged_array(char *filename, int *word_count);

int main(int argc, char *argv[]) {
    struct program_settings_s settings;

    settings.input_file = NULL;
    settings.output_file = NULL;
    settings.dictionary = NULL;
    settings.threads = 1;
    settings.verbose = 0;

    NOISY_DEBUG_PRINT;
    process_options(argc, argv, &settings);
    NOISY_DEBUG_PRINT;

    // DELETE THIS SECTION WHEN ALGORITHM ENUM IS IMPLEMENTE
    fprintf(stderr, "%s\n", algorithm_string[DES]);
    // DELETE THIS SECTION WHEN DONE
    //
    /*
    if (!input_file) {
        fprintf(stderr, "must give name for hashed password input file with
    -i\n"); exit(EXIT_FAILURE);
    }

    if (threads < 1) {
        exit(EXIT_FAILURE);
    }

    // Open the input file
    if ((ifd = open(input_file, O_RDONLY)) < 0) {
        fprintf(stderr, "failed to open input file");
        exit(EXIT_FAILURE);
    }

    // Open the output file
    if (!output_file) {
        ofd = STDOUT_FILENO;
    } else {
        if ((ofd = open(output_file, O_WRONLY | O_TRUNC | O_CREAT,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
    fprintf(stderr, "failed to open output file"); exit(EXIT_FAILURE);
        }
    }

    // Open the dictionary file
    if ((dfd = open(dictionary, O_RDONLY)) < 0) {
        fprintf(stderr, "failed to open input file");
        exit(EXIT_FAILURE);
    }

    */
    return EXIT_SUCCESS;
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
            settings->input_file = optarg;
            break;
        case 'o':
            settings->output_file = optarg;
            break;
        case 'd':
            settings->dictionary = optarg;
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
}

void detect_start_errors(struct program_settings_s *settings) {
    if (!settings->dictionary) {
        fprintf(stderr, "must give name for dictionary input file with -d\n");
        exit(EXIT_FAILURE);
    }

    if (!settings->input_file) {
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
