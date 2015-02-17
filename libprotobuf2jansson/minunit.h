#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

#define MAX_ERROR_MESSAGE_LENGTH 1024

#define mu_return_null_if_false(test) do { \
  if (!(test)) {                                                 \
    fprintf(stderr,_Generic(                                 \
        (test),                                          \
        long long:       "[Pseudo-EXCEPTION] %20.20s:%3.0d: %s: (%lld) != true\n", \
        unsigned long:   "[Pseudo-EXCEPTION] %20.20s:%3.0d: %s: (%lu) != true\n", \
        int:             "[Pseudo-EXCEPTION] %20.20s:%3.0d: %s: (%i) != true\n",  \
        unsigned char:   "[Pseudo-EXCEPTION] %20.20s:%3.0d: %s: (%u) != true\n",  \
        char *:          "[Pseudo-EXCEPTION] %20.20s:%3.0d: %s: (%s) != true\n",  \
        const char *:    "[Pseudo-EXCEPTION] %20.20s:%3.0d: %s: (%s) != true\n",  \
        unsigned char *: "[Pseudo-EXCEPTION] %20.20s:%3.0d: %s: (%p) != true\n"   \
      ), __FILE__, __LINE__, #test, test             \
    );                                                       \
    fflush(stderr);                                          \
    exit(111); \
    return NULL; \
  } \
} while (0)

#define mu_return_null_if(test) do { \
  if ((test)) {                                                 \
    fprintf(stderr,_Generic(                                 \
        (test),                                          \
        long long:       "[Pseudo-EXCEPTION] %20.20s:%3.0d: %s: (%lld) == true\n", \
        unsigned long:   "[Pseudo-EXCEPTION] %20.20s:%3.0d: %s: (%lu) == true\n", \
        int:             "[Pseudo-EXCEPTION] %20.20s:%3.0d: %s: (%i) == true\n",  \
        unsigned char:   "[Pseudo-EXCEPTION] %20.20s:%3.0d: %s: (%u) == true\n",  \
        char *:          "[Pseudo-EXCEPTION] %20.20s:%3.0d: %s: (%s) == true\n",  \
        const char *:    "[Pseudo-EXCEPTION] %20.20s:%3.0d: %s: (%s) == true\n",  \
        unsigned char *: "[Pseudo-EXCEPTION] %20.20s:%3.0d: %s: (%p) == true\n"   \
      ), __FILE__, __LINE__, #test, test             \
    );                                                       \
    fflush(stderr);                                          \
    exit(112); \
    return NULL; \
  } \
} while (0)

#define mu_return_null_if_null(test) do { \
  if (!(test)) {                                                 \
    fprintf(stderr, "[Pseudo-EXCEPTION] %20.20s:%3.0d:%30.30s %s: (%p) == NULL\n", \
      __FILE__, __LINE__, __func__, #test, test             \
    );                                                       \
    fflush(stderr);                                          \
    exit(111); \
    return NULL; \
  } \
} while (0)

#define mu_fatal(M, ...) do {                                                                            \
    fprintf(stderr, "[FATAL] %20.20s:%3.0d " M "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    fflush(stderr);                                                                                            \
    exit(1); \
} while (0)


#define mu_assert(test) do {                                     \
  if (!(test)) {                                                 \
    char *mu_message = malloc(MAX_ERROR_MESSAGE_LENGTH);            \
    if (mu_message == NULL) { printf("malloc failed"); exit(1); }   \
    snprintf(mu_message, MAX_ERROR_MESSAGE_LENGTH, _Generic(        \
        (test),                                                  \
        json_t *:      "%s:%i asserted: %s, reality: %s == %p",  \
        void *:        "%s:%i asserted: %s, reality: %s == %p",  \
        long long:     "%s:%i asserted: %s, reality: %s == %lld", \
        unsigned long: "%s:%i asserted: %s, reality: %s == %lu", \
        int:           "%s:%i asserted: %s, reality: %s == %i",  \
        unsigned char: "%s:%i asserted: %s, reality: %s == %u",  \
        const char *:  "%s:%i asserted: %s, reality: %s == %s",   \
        char *:        "%s:%i asserted: %s, reality: %s == %s"   \
      ),                                                         \
      __FILE__, __LINE__, #test, #test, test);                   \
    return mu_message;                                              \
  } \
} while (0)

#define mu_print(variable) do {                            \
  fprintf(stderr,_Generic(                                 \
      (0,variable),                                          \
      unsigned long:   "[DEBUG] %20.20s:%3.0d:%30.30s %s: %lu\n", \
      long long:       "[DEBUG] %20.20s:%3.0d:%30.30s %s: %lld\n", \
      int:             "[DEBUG] %20.20s:%3.0d:%30.30s %s: %i\n",  \
      unsigned char:   "[DEBUG] %20.20s:%3.0d:%30.30s %s: %u\n",  \
      const char *:    "[DEBUG] %20.20s:%3.0d:%30.30s %s: %s\n",  \
      char *:          "[DEBUG] %20.20s:%3.0d:%30.30s %s: %s\n",  \
      unsigned char *: "[DEBUG] %20.20s:%3.0d:%30.30s %s: %p\n"   \
    ), __FILE__, __LINE__, __func__, #variable, variable             \
  );                                                       \
  fflush(stderr);                                          \
} while (0)

#define mu_print_json(variable) do { \
  char *mu_json = json_dumps(variable, JSON_COMPACT|JSON_ENCODE_ANY); \
  fprintf(stderr, "[DEBUG] %20.20s:%3.0d:%30.30s %s: %s\n",  \
    __FILE__, __LINE__, __func__, #variable, mu_json             \
  );                                                       \
  free(mu_json); \
  fflush(stderr);                                          \
} while (0)


/*
  if (_Generic(                                                                           \
      (actual),                                                                           \
      unsigned long: actual != expected,                                                  \
      int:           actual != expected,                                                  \
      unsigned char: actual != expected,                                                  \
      char *:        strcmp(actual, expected) != 0                                        \
    ),                                                                                    \
    ) {                                                                                   \
*/
#define mu_assert_equal(actual, expected) do {                                            \
  if (actual != expected) {                                                               \
    char *mu_message = malloc(MAX_ERROR_MESSAGE_LENGTH);                                     \
    if (mu_message == NULL) { printf("malloc failed"); exit(1); }                            \
    snprintf(mu_message, MAX_ERROR_MESSAGE_LENGTH, _Generic(                                 \
        (actual),                                                                         \
        unsigned long: "%s:%i required: %s == %s, reality: %s == %lu",                    \
        int:           "%s:%i required: %s == %s, reality: %s == %i",                     \
        unsigned char: "%s:%i required: %s == %s, reality: %s == %u",                     \
        char *:        "%s:%i required: %s == %p, reality: %s == %p"                      \
      ),                                                                                  \
      __FILE__, __LINE__, #actual, #expected, #actual, actual);                           \
    return mu_message;                                                                       \
  }                                                                                       \
} while (0)

#define mu_assert_not_equal(actual, not_expected) do {                                            \
  if (actual == not_expected) {                                                               \
    char *mu_message = malloc(MAX_ERROR_MESSAGE_LENGTH);                                     \
    if (mu_message == NULL) { printf("malloc failed"); exit(1); }                            \
    snprintf(mu_message, MAX_ERROR_MESSAGE_LENGTH, _Generic(                                 \
        (actual),                                                                         \
        unsigned long: "%s:%i required: %s != %s, reality: %s == %lu",                    \
        int:           "%s:%i required: %s != %s, reality: %s == %i",                     \
        unsigned char: "%s:%i required: %s != %s, reality: %s == %u",                     \
        char *:        "%s:%i required: %s != %p, reality: %s == %p"                      \
      ),                                                                                  \
      __FILE__, __LINE__, #actual, #not_expected, #actual, actual);                           \
    return mu_message;                                                                       \
  }                                                                                       \
} while (0)

#define mu_assert_equal_str(actual, expected) do {                                            \
  if (strcmp(actual,expected) != 0) {                                                               \
    char *mu_message = malloc(MAX_ERROR_MESSAGE_LENGTH);                                     \
    if (mu_message == NULL) { printf("malloc failed"); exit(1); }                            \
    snprintf(mu_message, MAX_ERROR_MESSAGE_LENGTH,                                  \
      "%s:%i required: %s == %s, reality: %s == %s",                      \
      __FILE__, __LINE__, #actual, #expected, #actual, actual);                           \
    return mu_message;                                                                       \
  }                                                                                       \
} while (0)

#define mu_run_test(test) do { \
  char *message = test();      \
  tests_run++;                 \
  if (message) return message; \
} while (0)

extern int tests_run;
