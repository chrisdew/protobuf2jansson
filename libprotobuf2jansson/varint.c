#include <stdio.h>
#include <stdint.h>
#include "minunit.h"
#include "varint.h"

int p2j_varint_decode(const uint8_t *const buffer, const size_t buffer_length, uint64_t *varint_out, size_t *bytes_parsed_out) {
    uint64_t accumulator = 0;
    uint8_t  position    = 0;
    for (const uint8_t *p = buffer; p < buffer + buffer_length; p++) {
        uint8_t value = *p & 0x7f;
        accumulator += value * (1 << position);
        //mu_print(accumulator);
        //mu_print(position);
        if (!(*p & 0x80)) {    // last byte of varint
            //mu_print(p);
            //mu_print(buffer);
            *varint_out = accumulator;
            *bytes_parsed_out = p - buffer + 1; // number of uint8's parsed 
            return P2J_VARINT_PARSED;
        }
        position += 7;         // the seven low bits carry the data
        if (position > 56) {
            // TODO: this is a horrible alternaitve to throwing an exception
            *varint_out = 0;
            *bytes_parsed_out = 0;
            return P2J_VARINT_MALFORMED;
        }
    }
    // we've run off of the end of the buffer, without fully decoding a varint
    // so return that zero uint8s processed
    *varint_out = 0; // tidy up
    *bytes_parsed_out = 0;
    return P2J_VARINT_MORE_BYTES_NEEDED;
}

