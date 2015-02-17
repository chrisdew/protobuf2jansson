#ifndef libprotobuf2jansson_H
#define libprotobuf2jansson_H

#include "stdio.h"
#include <jansson.h>

extern void print_hello ();

extern char *p2j_package_and_name_from_type_name(char *type_name__destroyed, char **package_out, char **name_list_out, size_t name_list_length);

extern json_t *p2j_get_field_by_number(json_t *message, json_int_t number);

extern json_t *p2j_get_message_by_name(json_t *descriptor, const char *name);

extern json_t *p2j_protobuf2json_object(json_t *desc, const char *message_name, const char *buffer, size_t length);



#endif
