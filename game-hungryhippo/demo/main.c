#include "ehhh.h"
#include "sdl_wrapper.h"
#include "vector.h"

/*** CONSTANTS ***/
char *USAGE_PATH = "static/usage.txt";
char *SYNOPSIS_FMT = "usage: %s [--help|-h] [OPTIONS]\n";
char *HINT_FMT = "Hint: do '%s --help'.\n";
const size_t DEFAULT_PLAYER_COUNT = 4;
const size_t DEFAULT_BALLS_PER_ROUND = 30;

/*** PROTOTYPES ***/
void print_file(char *path, FILE *stream);
void print_usage(FILE *out, char *progname);
int error_option_arg_missing(char *option);

/**
 * Return 0 is str1 matches any of the null-delimited strs2, else return
 * nonzero.
 */
int strscmp(char *str1, char *strs2, size_t strs2c);

/*** DEFINITIONS ***/

void print_file(char *path, FILE *out) {
    FILE *stream = fopen(path, "r");
    assert(stream != NULL);
    char c;
    while (true) {
        c = fgetc(stream);
        if (feof(stream)) {
            break;
        }
        fprintf(out, "%c", c);
    }
    fclose(stream);
}

void print_usage(FILE *out, char *progname) {
    fprintf(out, SYNOPSIS_FMT, progname);
    print_file(USAGE_PATH, out);
}

void print_hint(char *progname) {
    fprintf(stdout, HINT_FMT, progname);
}

int strscmp(char *str1, char *strs2, size_t strs2c) {
    for (size_t i = 0; i < strs2c; i++) {
        if (strcmp(str1, strs2) == 0) {
            return 0;
        }
        strs2 += strlen(strs2) + 1;
    }
    return 1;
}

/*** MAIN ***/
int main(int argc, char **argv) {
    // Parse arguments.
    char *progname = argv[0];
    if (argc == 1) {
        print_hint(progname);
    }
    size_t player_count = DEFAULT_PLAYER_COUNT;
    size_t balls_per_round = DEFAULT_BALLS_PER_ROUND;
    for (size_t i = 1; i < argc; i++) {
        char *a = argv[i];
        if (strscmp(a, "help\0--help\0-h", 3) == 0) {
            print_usage(stdout, progname);
            return EXIT_SUCCESS;
        } else if (strscmp(a, "--players\0-p", 2) == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Option requires an argument: %s\n", argv[i]);
                return EXIT_FAILURE;
            }
            player_count = strtol(argv[i + 1], NULL, 0);
            if (player_count > EHHH_MAX_PLAYERS) {
                fprintf(stderr,
                        "No more than %d players are allowed: %zu\n",
                        EHHH_MAX_PLAYERS,
                        player_count);
                print_usage(stderr, progname);
                return EXIT_FAILURE;
            }
            if (player_count < EHHH_MIN_PLAYERS) {
                fprintf(stderr,
                        "No fewer than %d players are allowed: %zu\n",
                        EHHH_MIN_PLAYERS,
                        player_count);
                print_usage(stderr, progname);
                return EXIT_FAILURE;
            }
            i++;
            continue;
        } else if (strscmp(a, "--balls-per-round\0-b", 2) == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Option requires an argument: %s\n", argv[i]);
                return EXIT_FAILURE;
            }
            balls_per_round = strtol(argv[i + 1], NULL, 0);
            if (balls_per_round > EHHH_MAX_BALLS_PER_ROUND) {
                fprintf(stderr,
                        "No more than %zu balls are allowed: %zu\n",
                        EHHH_MAX_BALLS_PER_ROUND,
                        balls_per_round);
                return EXIT_FAILURE;
            }
            if (balls_per_round <= 0) {
                fprintf(stderr,
                        "We need positively many balls for play: %zu\n",
                        balls_per_round);
                return EXIT_FAILURE;
            }
            i++;
            continue;
        } else {
            fprintf(stderr, "Unrecognized argument: %s\n", argv[i]);
            print_usage(stderr, progname);
            return EXIT_FAILURE;
        }
    }

    // Entry point.
    ehhh_t *ehhh = ehhh_init(player_count, balls_per_round);
    while (!ehhh_tick(ehhh, time_since_last_tick())) {}
    ehhh_free(ehhh);

    return EXIT_SUCCESS;
}
