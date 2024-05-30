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
//
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

#include "thread_hash.h"

// Noisy debugger for verbose debugging
#ifdef NOISY_DEBUG
#define NOISY_DEBUG_PRINT                                                      \
    fprintf(stderr, "DEBUG: %s:%s:%d\n", __FILE__, __func__, __LINE__)
#else // NOISY_DEBUG
#define NOISY_DEBUG_PRINT
#endif // NOISY_DEBUG


int main(int argc, char *argv[]) { 
    return EXIT_SUCCESS; 
}

viktar_action_t process_options(void) {
    // Reminder: Leftover arguments are argv[optint] through argv[argc-1]
    viktar_action_t action = ACTION_NONE; // The action to perform
    int opt = 0;

    // After using the test script I have an issue with get_opt moving
    // Around the argv array. I'm going ot use getopt once to sort the
    // options, and then use getopt again to get the file name in the right
    // order. This is a hack, but it works.

    while ((opt = getopt(ARGC, ARGV, OPTIONS)) != -1) {
    }

    opt = 0;
    optind = 0;

    NOISY_DEBUG_PRINT;
    while ((opt = getopt(ARGC, ARGV, OPTIONS)) != -1) {
        switch (opt) {
   }
    NOISY_DEBUG_PRINT;

    return action;
}
