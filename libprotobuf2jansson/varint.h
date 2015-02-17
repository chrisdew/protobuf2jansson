#ifndef VARINT_H
#define VARINT_H

#include <jansson.h>

enum p2j_varint_result_enum { P2J_VARINT_PARSED, P2J_VARINT_MORE_BYTES_NEEDED, P2J_VARINT_MALFORMED };

int p2j_varint_decode(const uint8_t *const buffer, const size_t buffer_length, uint64_t *varint_out, size_t *bytes_parsed_out);


#endif
