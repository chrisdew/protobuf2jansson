#ifndef libprotobuf2jansson_H
#define libprotobuf2jansson_H

#include "stdio.h"
#include <jansson.h>

#define P2J_OPTION_RETAIN_SNAKE_CASE                          (1 << 0)
#define P2J_OPTION_WILL_NOT_HANDLE_FRAGMENTED_REPEATED_FIELDS (1 << 1)
#define P2J_OPTION_64_BIT_VALUES_AS_STRINGS                   (1 << 2)
#define P2J_OPTION_ENUMS_AS_STRINGS                           (1 << 3)


extern void print_hello ();

extern char *p2j_camel_case_in_place(char *snake_case__mutated);

extern char *p2j_package_and_name_from_type_name(char *type_name__destroyed, char **package_out, char **name_list_out, size_t name_list_length, json_error_t *error__out);

extern json_t *p2j_get_field_by_number(json_t *message, json_int_t number, json_error_t *error__out);

extern json_t *p2j_get_message_by_name(json_t *descriptor, const char *name, json_error_t *error__out);

extern json_t *p2j_protobuf2json_object(json_t *desc, const char *message_name, const char *buffer, size_t length, int options, json_error_t *error__out);



#endif
