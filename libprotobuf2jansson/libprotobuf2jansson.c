#include "../include/protobuf2jansson.h"

#include <stdint.h>
#include <stddef.h>
#include <jansson.h>
#include <string.h>
#include <ctype.h>
#include "minunit.h"
#include "varint.h"
#include "meta_descriptor.json.h"

#define WIRE_TYPE_VARINT 0
#define WIRE_TYPE_64_BIT 1
#define WIRE_TYPE_LENGTH_DELIMITED 2
#define WIRE_TYPE_32_BIT 3

#define TYPE_INT32 "TYPE_INT32"
#define TYPE_INT64 "TYPE_INT64"
#define TYPE_UINT32 "TYPE_UINT32"
#define TYPE_UINT64 "TYPE_UINT64"
#define TYPE_SINT32 "TYPE_SINT32"
#define TYPE_SINT64 "TYPE_SINT64"
#define TYPE_BOOL "TYPE_BOOL"
#define TYPE_ENUM "TYPE_ENUM"

#define TYPE_FIXED64 "TYPE_FIXED64"
#define TYPE_SFIXED64 "TYPE_SFIXED64"
#define TYPE_DOUBLE "TYPE_DOUBLE"

#define TYPE_STRING "TYPE_STRING"
#define TYPE_BYTES "TYPE_BYTES"
#define TYPE_MESSAGE "TYPE_MESSAGE"

#define TYPE_FIXED32 "TYPE_FIXED32"
#define TYPE_SFIXED32 "TYPE_SFIXED32"
#define TYPE_FLOAT "TYPE_FLOAT"

#define LABEL_REPEATED "LABEL_REPEATED"

int wire_type_from_message_type(char *message_type) {
  if (strcmp(TYPE_INT32, message_type) == 0) return WIRE_TYPE_VARINT;
  if (strcmp(TYPE_INT64, message_type) == 0) return WIRE_TYPE_VARINT;
  if (strcmp(TYPE_UINT32, message_type) == 0) return WIRE_TYPE_VARINT;
  if (strcmp(TYPE_UINT64, message_type) == 0) return WIRE_TYPE_VARINT;
  if (strcmp(TYPE_SINT32, message_type) == 0) return WIRE_TYPE_VARINT;
  if (strcmp(TYPE_SINT64, message_type) == 0) return WIRE_TYPE_VARINT;
  if (strcmp(TYPE_BOOL, message_type) == 0) return WIRE_TYPE_VARINT;
  if (strcmp(TYPE_ENUM, message_type) == 0) return WIRE_TYPE_VARINT;

  if (strcmp(TYPE_FIXED64, message_type) == 0) return WIRE_TYPE_64_BIT;
  if (strcmp(TYPE_SFIXED64, message_type) == 0) return WIRE_TYPE_64_BIT;
  if (strcmp(TYPE_DOUBLE, message_type) == 0) return WIRE_TYPE_64_BIT;

  if (strcmp(TYPE_STRING, message_type) == 0) return WIRE_TYPE_LENGTH_DELIMITED;
  if (strcmp(TYPE_BYTES, message_type) == 0) return WIRE_TYPE_LENGTH_DELIMITED;
  if (strcmp(TYPE_MESSAGE, message_type) == 0) return WIRE_TYPE_LENGTH_DELIMITED;

  if (strcmp(TYPE_FIXED32, message_type) == 0) return WIRE_TYPE_32_BIT;
  if (strcmp(TYPE_SFIXED32, message_type) == 0) return WIRE_TYPE_32_BIT;
  if (strcmp(TYPE_FLOAT, message_type) == 0) return WIRE_TYPE_32_BIT;

  exit(1);
}

json_t *p2j_get_field_by_number(json_t *message, json_int_t number) {
    mu_return_null_if_false(json_is_object(message));
    json_t *fields = json_object_get(message, "field");

    // loop through the array until we find the field
    for (size_t j = 0; j < json_array_size(fields); j++) {
        json_t *field = json_array_get(fields, j);
        json_t *field_number = json_object_get(field, "number");
        mu_return_null_if_false(json_is_integer(field_number));
        mu_print(json_integer_value(field_number));
        mu_print(number);
        if (json_integer_value(field_number) == number) {
            //json_incref(field); // returns a borrowed field
            return field;
        }
    }
    return NULL;
}

// destroys type_name, by changing the last dot into a \x00 and returning pointers to package and name within the destroyed argument
// returns pointer to package name
// update: it looks like it's not that last dot that is the separator, but the first dot is followed by a captial letter
char *p2j_package_and_name_from_type_name(char *type_name__destroyed, char **package_out, char **name_list_out, size_t name_list_length) {
    mu_return_null_if_false(type_name__destroyed);
    size_t type_name_length = strlen(type_name__destroyed);
    mu_return_null_if(type_name_length < 2);
    mu_return_null_if(type_name__destroyed[0] != '.');

    // find the first dot which is followed by a capital letter, p starts at the first character of type_name_destroyed (the following character is, by definition, \x00.
    size_t i = 0;
    char *p;
    for (p = type_name__destroyed; p <= type_name__destroyed + type_name_length; p++) {
      if (*p == '.' && isupper(*(p+1))) {
        // p now points at the first dot which is followed by a capital letter
        if (i > (name_list_length - 1)) return NULL; // abort if there are more parts than storage
        name_list_out[i++] = p + 1;
        //mu_print(*name_list_out[i-1]);
        *p = '\x00'; // replace the last dot, which separated package from name, with a null to terminate the precedingi string
      }
    }
    // now get the package string
    *package_out = type_name__destroyed + 1;
    //mu_print(*package_out);

    return *package_out; // success
}

/*
char *p2j_package_and_name_from_type_name(char *type_name__destroyed, char **package_out, char **name_out) {
    mu_return_null_if_false(type_name__destroyed);
    size_t type_name_length = strlen(type_name__destroyed);
    mu_return_null_if(type_name_length < 2);
    mu_return_null_if(type_name__destroyed[0] != '.');

    // find the last dot, p starts at the last character of type_name_destroyed (the following character is, by definition, \x00.
    char *p; // = type_name__destroyed + type_name_length - 1;
    for (p = type_name__destroyed + type_name_length - 1; *p != '.' && p > type_name__destroyed; p--);
    // p now points at the last dot
    *name_out = p + 1;
    //mu_print(*name_out);
    // now get the package string
    *package_out = type_name__destroyed + 1;
    //mu_print(*package_out);
    *p = '\x00'; // replace the last dot, which separated package from name, with a null to terminate the package string

    return *package_out; // success
}
*/

json_t *p2j_get_nested_message_by_name(json_t *nodes, char **remaining_names) {
    //mu_print_json(nodes);
    //mu_print(remaining_names[0]);

    // nodes is a json array, either file.messageType at the top level,
    // or file.messageType[n].nestedType[n].nestedType[m]... at lower levels
    mu_return_null_if_null(remaining_names[0]);

    // loop through the array until we find the message
    for (size_t j = 0; j < json_array_size(nodes); j++) {
        json_t *node = json_array_get(nodes, j);
        json_t *jname = json_object_get(node, "name");
        mu_return_null_if_false(json_is_string(jname));
        //mu_print(json_string_value(jname));
        //mu_print(name);

        if (strcmp(json_string_value(jname), remaining_names[0]) == 0) {
            if ((*(remaining_names + 1)) == NULL) { // at the leaf
                return node;
            } else { // otherwise recurse into the next level
                json_t *nestedTypes = json_object_get(node, "nestedType");
                mu_return_null_if_null(nestedTypes);
                return p2j_get_nested_message_by_name(nestedTypes, remaining_names + 1);
            }
        }
    }

    return NULL;
}

// TODO: this may be inefficient and could be sped up by the use of a hash table
json_t *p2j_get_message_by_name(json_t *descriptor, const char *type_name) {
    char *package;
    //mu_return_null_if_false(descriptor);
    size_t type_name_length = strlen(type_name) + 1;
    char tmp[type_name_length];
    mu_return_null_if_false(strncpy(tmp, type_name, type_name_length + 1));
    //mu_print((char *)tmp);
    //mu_return_null_if(tmp[0] != '.');
    char *name_list[] = {NULL, NULL, NULL, NULL};
    mu_return_null_if_false(p2j_package_and_name_from_type_name(tmp, &package, name_list, sizeof(name_list)/sizeof(name_list[0])));

    mu_return_null_if_false(json_is_object(descriptor));
    json_t *files = json_object_get(descriptor, "file");
    mu_return_null_if_false(json_is_array(files));

    // loop through the files (if there is more than one)
    for (size_t i = 0; i < json_array_size(files); i++) {
        //TODO: only look at "files" that declasre the correct package

        json_t *file = json_array_get(files, i);
        mu_return_null_if_false(json_is_object(descriptor));
        json_t *message_types = json_object_get(file, "messageType");
        mu_return_null_if_false(json_is_array(message_types));

        return p2j_get_nested_message_by_name(message_types, name_list);

    }

    return NULL;
}

json_t *p2j_protobuf2varint(json_t *desc, const char *type, const char *message_name, const char *buffer, size_t length, size_t *bytes_parsed) {
    mu_print((uint8_t *)desc);
    mu_print(type);
    mu_print(message_name);
    mu_print((uint8_t *)buffer);
    mu_print(length);
    mu_print((uint8_t *)bytes_parsed);

    size_t num;
    size_t varint;
    enum p2j_varint_result_enum result = p2j_varint_decode((uint8_t *)buffer, length, (size_t *) &varint, &num);
    mu_return_null_if(result != P2J_VARINT_PARSED);

    json_t *value = NULL;
    if (strcmp(type, TYPE_INT32) == 0) {
        value = json_integer(varint);
    } else if (strcmp(type, TYPE_SINT32) == 0) {
        exit(3);
    } else if (strcmp(type, TYPE_BOOL) == 0) {
        value = json_boolean(varint);
    } else if (strcmp(type, TYPE_ENUM) == 0) {
        // FIXME: we should allow this to return either a JSON int or a JSNO string under user control
        value = json_integer(varint);
    }
    *bytes_parsed = num;
    mu_print_json(value);
    return value;
}
//json_t *p2j_protobuf2_64bit(json_t *desc, char *type, char *message_name, char *buffer, size_t length, size_t *bytes_parsed) {
//    return NULL;
//}
json_t *p2j_protobuf2length_delimited(json_t *desc, const char *type, const char *message_name, const json_t *field, const char *buffer, size_t length, size_t *bytes_parsed) {
    mu_print((uint8_t *)desc);
    mu_print(type);
    mu_print(message_name);
    mu_print((uint8_t *)buffer);
    mu_print(length);
    mu_print((uint8_t *)bytes_parsed);


    size_t num;
    json_int_t message_length;
    enum p2j_varint_result_enum result = p2j_varint_decode((uint8_t *)buffer, length, (size_t *) &message_length, &num);
    mu_return_null_if(result != P2J_VARINT_PARSED);
    buffer += num; length -= num;

    mu_print(message_length);

    mu_print(num);

    *bytes_parsed = num + message_length;

    json_t *value = NULL;
    if (strcmp(type, TYPE_STRING) == 0) {
        value = json_stringn(buffer, message_length);
    } else if (strcmp(type, TYPE_BYTES) == 0) {
        // FIXME: base64, HTML or hex encode this
        value = json_stringn(buffer, message_length);
    } else if (strcmp(type, TYPE_MESSAGE) == 0) {
        value = p2j_protobuf2json_object(desc, message_name, buffer, message_length);
    }
    *bytes_parsed = num + message_length;
    mu_print_json(value);
    return value;
}
//json_t *p2j_protobuf2_32bit(json_t *desc, char *type, char *message_name, char *buffer, size_t length, size_t *bytes_parsed) {
//    return NULL;
//}


json_t *p2j_protobuf2json_object(json_t *desc, const char *message_name, const char *buffer, size_t length) {
    mu_print((char *)">>>>>>>>");
    //json_error_t error;

    mu_print(message_name);

    json_t *ret = json_object();

    while (length > 0) {
        mu_print((unsigned char*)buffer);
        mu_print(length);
        // first (try to) read a varint from the buffer, containing the field number (<<3) and the type
        size_t bytes_parsed = 0;
        uint64_t varint = 0;
        enum p2j_varint_result_enum result = p2j_varint_decode((uint8_t *)buffer, length, &varint, &bytes_parsed);
        mu_print(varint);
        mu_print(bytes_parsed);
        buffer += bytes_parsed; length -= bytes_parsed;
        mu_return_null_if(result != P2J_VARINT_PARSED);


        json_int_t field_num = varint >> 3;
        json_int_t field_wire_type = varint & 0x7;

        mu_print(field_num);
        mu_print(field_wire_type);

        // get the appropriate message descriptor
        json_t *message = p2j_get_message_by_name(desc, message_name);
        mu_return_null_if(message == NULL);
        //mu_print_json(message);

        // check that the field wire type agrees with one present in the descriptor
        // note: multiple field types maps to single wire_types - e.g. TYPE_STRING and TYPE_MESSAGE are both wire_type 2, hence the lookup
        // https://developers.google.com/protocol-buffers/docs/encoding#structure
        json_t *field = p2j_get_field_by_number(message, field_num);
        mu_return_null_if(field == NULL);
        //mu_print_json(field);
        char *type = (char *)json_string_value(json_object_get(field, "type"));
        mu_print(type);
        json_int_t looked_up_field_wire_type = wire_type_from_message_type(type); // see notes for field_wire_type_lookup
        mu_print(looked_up_field_wire_type);
        mu_return_null_if(field_wire_type != looked_up_field_wire_type);

        char *key = (char *)json_string_value(json_object_get(field, "name"));
        mu_return_null_if_null(key);
        json_t *value = NULL; // child value to add to this object, with a key from the field.name
        bytes_parsed = 0;
        switch (field_wire_type) {
            case WIRE_TYPE_VARINT:
                value = p2j_protobuf2varint(desc, type, message_name, buffer, length, &bytes_parsed);
                //exit(3);
                break;
            case WIRE_TYPE_64_BIT:
                //value = p2j_protobuf2_64bit(desc, type, message_name, buffer, length, &bytes_parsed);
                exit(4);
                break;
            case WIRE_TYPE_LENGTH_DELIMITED:
                mu_print((uint8_t*)buffer);
                const char *type_name = json_string_value(json_object_get(field, "typeName"));
                mu_print(type_name);
                value = p2j_protobuf2length_delimited(desc, type, type_name, field, buffer, length, &bytes_parsed);
                mu_print(bytes_parsed);

                break;
            case WIRE_TYPE_32_BIT:
                //value = p2j_protobuf2_32bit(desc, type, message_name, buffer, length, &bytes_parsed);
                exit(5);
                break;
        }

/*
        fflush(stdout);
        fflush(stderr);
        printf("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
        fflush(stdout);
        fflush(stderr);

        mu_print_json(field);

        mu_print(key);
        mu_print_json(value);

        fflush(stdout);
        fflush(stderr);
        printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
        fflush(stdout);
        fflush(stderr);
*/




        buffer += bytes_parsed; length -= bytes_parsed;
        mu_return_null_if_null(value);

        const char *field_label = json_string_value(json_object_get(field, "label"));
        mu_print(field_label);
        if (strcmp(field_label, LABEL_REPEATED) == 0) {
            // repeated fields in protocol buffers are represented as arrays in JSON
            json_t *array = json_object_get(ret, key);
            if (array == NULL) { // array not created yet
                mu_print((char *)"created new array");
                array = json_array();
                mu_return_null_if_null(array);
                json_object_set(ret, key, array);
                json_decref(array);
            }
            mu_print_json(array);
            mu_print_json(ret);
            mu_print(key);
            mu_return_null_if_false(json_is_array(array));
            json_array_append(array, value);
            mu_print((char *)"appended to array");
            mu_print_json(array);
        } else {
            // FIXME: for correctness, this should check for an existing object property "key"
            // and perform a deep tree merge
            json_object_set(ret, key, value);
        }
        json_decref(value);

        mu_print((char *)"========");
    }
    mu_print((char *)"<<<<<<<<");
    return ret;
}




void
print_hello(){
  printf("$(message)\n");
}