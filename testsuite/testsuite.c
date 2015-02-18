/*
 ============================================================================
 Name        : exampleProgram.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Uses shared library to print greeting
               To run the resulting executable the LD_LIBRARY_PATH must be
               set to ${project_loc}/libprotobuf2jansson/.libs
               Alternatively, libtool creates a wrapper shell script in the
               build directory of this program which can be used to run it.
               Here the script will be called exampleProgram.
 ============================================================================
 */

#include "../include/protobuf2jansson.h"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../libprotobuf2jansson/varint.h"
#include "../libprotobuf2jansson/meta_descriptor.json.h"

#include "../libprotobuf2jansson/minunit.h"
int tests_run = 0;

static char *dummy_test() {
    int result = 42;
    mu_assert_equal(result, 42);
    return 0;
}

static char *test_varint_decode() {
    uint8_t  zero[] = {0x00};
    uint8_t  one[]  = {0x01};
    uint8_t  one_two_three[]               = {0x01, 0x02, 0x03};
    uint8_t  one_hundred_and_twenty_nine[] = {0x81, 0x01};
    uint8_t  incomplete[]                  = {0x81, 0x81};
    uint8_t  unparsable_junk[]             = {0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89};

    int result;
    uint64_t varint; // reused a for each subtest
    size_t bytes_parsed;

    result = p2j_varint_decode(zero, sizeof(zero), &varint, &bytes_parsed);
    mu_assert_equal(result, P2J_VARINT_PARSED);
    mu_assert_equal(bytes_parsed, 1);
    mu_assert_equal(varint, 0);

    result = p2j_varint_decode(one, sizeof(one), &varint, &bytes_parsed);
    mu_assert_equal(result, P2J_VARINT_PARSED);
    mu_assert_equal(bytes_parsed, 1);
    mu_assert_equal(varint, 1);

    result = p2j_varint_decode(one_two_three, sizeof(one_two_three), &varint, &bytes_parsed);
    mu_assert_equal(result, P2J_VARINT_PARSED);
    mu_assert_equal(bytes_parsed, 1);
    mu_assert_equal(varint, 1);

    result = p2j_varint_decode(one_hundred_and_twenty_nine, sizeof(one_hundred_and_twenty_nine), &varint, &bytes_parsed);
    mu_assert_equal(result, P2J_VARINT_PARSED);
    mu_assert_equal(bytes_parsed, 2);
    mu_assert_equal(varint, 129);

    result = p2j_varint_decode(incomplete, sizeof(incomplete), &varint, &bytes_parsed);
    mu_assert_equal(result, P2J_VARINT_MORE_BYTES_NEEDED);

    result = p2j_varint_decode(unparsable_junk, sizeof(unparsable_junk), &varint, &bytes_parsed);
    mu_assert_equal(result, P2J_VARINT_MALFORMED);

    return 0;
}

static char *test_p2j_get_message_by_name() {
    json_error_t error;

    json_t *meta_desc = json_loads((char *)p2j_meta_descriptor_json, 0, &error);

    mu_assert(meta_desc);

    json_t *message = p2j_get_message_by_name(meta_desc, ".google.protobuf.FileDescriptorSet", &error);
    mu_assert(message);

    mu_assert(json_is_object(message));
    //mu_print_json(message);
    const char *name = json_string_value(json_object_get(message, "name"));
    mu_assert_equal_str(name, "FileDescriptorSet");
    //free(name);

    json_decref(meta_desc);
    return 0;
}

static char *test_p2j_get_message_by_name2() {
    json_error_t error;
    json_t *meta_desc = json_loads((char *)p2j_meta_descriptor_json, 0, &error);
    mu_assert(meta_desc);

    json_t *message = p2j_get_message_by_name(meta_desc, ".google.protobuf.DescriptorProto.ExtensionRange", &error);
    mu_assert(message);

    mu_assert(json_is_object(message));
    //mu_print_json(message);
    const char *name = json_string_value(json_object_get(message, "name"));
    mu_assert_equal_str(name, "ExtensionRange");
    //free(name);

    json_decref(meta_desc);
    return 0;
}

static char *test_p2j_package_and_name_from_type_name() {
   json_error_t error;
   char destroyable[] = ".foo.bar.Nested.Name";
   char *package;
   char *name_list[] = {NULL, NULL, NULL, NULL};
   char *result = p2j_package_and_name_from_type_name(destroyable, &package, name_list, sizeof(name_list)/sizeof(name_list[0]), &error);
   mu_assert(result);
   mu_assert_equal_str(package,"foo.bar");
   mu_assert_equal_str(name_list[0],"Nested");
   mu_assert_equal_str(name_list[1],"Name");
   mu_assert_equal(name_list[2],NULL);

   return 0;
}

// Test with meta_descriptor.desc as the data because we cannot use any other descriptors
// until we can convert them to JSON, which requires this to work...
static char *test_p2j_protobuf2json_object() {
    // read the .desc file into memory
    int fd = open("/home/chris/workspace/protobuf2jansson/testsuite/meta_descriptor.desc", O_RDONLY);
    mu_assert_not_equal(fd, -1);
    struct stat stat;
    fstat(fd, &stat);
    char protobuf_data[stat.st_size]; // it's only 4K, so we're OK on the stack
    size_t num_read = read(fd, protobuf_data, sizeof(protobuf_data));
    mu_assert_equal(num_read, (size_t) stat.st_size);
    //mu_print(num_read);

    // get a descriptor, in Jansson form, to be used to decode desc
    json_error_t error;
    json_t *desc = json_loads((char *)p2j_meta_descriptor_json, 0, &error);
    mu_assert(desc);

    json_t *result = p2j_protobuf2json_object(desc, ".google.protobuf.FileDescriptorSet", protobuf_data, sizeof(protobuf_data), P2J_OPTION_WILL_NOT_HANDLE_FRAGMENTED_REPEATED_FIELDS | P2J_OPTION_ENUMS_AS_STRINGS, &error);
    mu_assert(result);
    mu_print_json(result);
    json_decref(result);

    json_decref(desc);
    return 0;
}

static char *test_json_stringn() {
    char *buffer = "src/meta_descriptor.proto123";
    json_t *value = json_stringn(buffer, 25);
    printf("value (as pointer): |%p|\n", value);
    char *json_string = json_dumps(value, JSON_COMPACT|JSON_ENCODE_ANY);
    printf("json_string: |%s| (as_pointer): |%p|\n", json_string, json_string);
    free(json_string);
    json_decref(value);
    return 0;
}

static char *test_p2j_camel_case_in_place() {
	char snake_case[] = "this_is_a_test"; // NB: this initialises a mutable version on the stack
	char *camelCase = p2j_camel_case_in_place(snake_case);
	mu_print(camelCase);
    mu_assert_equal(strcmp(camelCase, "thisIsATest"), 0);
    return 0;
}

static char *test_p2j_camel_case_in_place1() {
	char snake_case[] = "this_is_a_"; // NB: this initialises a mutable version on the stack
	char *camelCase = p2j_camel_case_in_place(snake_case);
	mu_print(camelCase);
    mu_assert_equal(strcmp(camelCase, "thisIsA_"), 0);
    return 0;
}

static char *test_p2j_camel_case_in_place2() {
	char snake_case[] = ""; // NB: this initialises a mutable version on the stack
	char *camelCase = p2j_camel_case_in_place(snake_case);
	mu_print(camelCase);
    mu_assert_equal(strcmp(camelCase, ""), 0);
    return 0;
}

static char *test_p2j_camel_case_in_place3() {
	char snake_case[] = "_"; // NB: this initialises a mutable version on the stack
	char *camelCase = p2j_camel_case_in_place(snake_case);
	mu_print(camelCase);
    mu_assert_equal(strcmp(camelCase, "_"), 0);
    return 0;
}

static char *all_tests() {
    mu_run_test(dummy_test);
    mu_run_test(test_varint_decode);
    mu_run_test(test_p2j_package_and_name_from_type_name);
    mu_run_test(test_p2j_get_message_by_name);
    mu_run_test(test_p2j_get_message_by_name2);
    mu_run_test(test_p2j_protobuf2json_object);
    mu_run_test(test_json_stringn);
    mu_run_test(test_p2j_camel_case_in_place);
    mu_run_test(test_p2j_camel_case_in_place1);
    mu_run_test(test_p2j_camel_case_in_place2);
    mu_run_test(test_p2j_camel_case_in_place3);
    return 0;
}

int main() {
    char *result = all_tests();
    if (result != 0) {
        printf("TEST FAILED: %s\n", result);
        free(result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
