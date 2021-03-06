/*
 * Copyright (c) 2015 Kazuho Oku
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include <stdio.h>
#include <string.h>
#include "golombset.h"

#define FIXED_BITS_LENGTH 5

static int encode(void)
{
    uint64_t keys[4096];
    size_t num_keys = 0;
    unsigned char buf[65536];

    while (1) {
        uint64_t v;
        if (fscanf(stdin, "%" SCNu64 "\n", &v) != 1)
            break;
        keys[num_keys++] = v;
    }
    if (!feof(stdin)) {
        perror("failed to parse input");
        return 111;
    }

    golombset_encoder_t ctx = {};
    ctx.dst = buf;
    ctx.dst_max = buf + sizeof(buf);
    ctx.fixed_bits_length = FIXED_BITS_LENGTH;
    if (golombset_encode(&ctx, keys, num_keys, GOLOMBSET_ENCODE_CALC_FIXED_BITS) != 0) {
        fprintf(stderr, "failed to encode the values\n");
        return 111;
    }

    fwrite(buf, 1, ctx.dst - buf, stdout);
    return 0;
}

static int decode(void)
{
    unsigned char buf[65536];
    size_t buf_size;
    uint64_t keys[4096];
    size_t i, num_keys = sizeof(keys) / sizeof(keys[0]);

    buf_size = fread(buf, 1, sizeof(buf), stdin);
    if (ferror(stdin) || !feof(stdin)) {
        perror("failed to read input");
        return 111;
    }

    golombset_decoder_t ctx = {};
    ctx.src = buf;
    ctx.src_max = buf + buf_size;
    ctx.fixed_bits_length = FIXED_BITS_LENGTH;
    if (golombset_decode(&ctx, keys, &num_keys, 0) != 0) {
        fprintf(stderr, "failed to decode the values\n");
        return 111;
    }

    for (i = 0; i != num_keys; ++i)
        printf("%" PRIu64 "\n", keys[i]);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr,
                "Usage: %s [--encode|--decode]\n"
                "\n"
                "When --encode option is specified, reads a line-separated list of unsigned\n"
                "numbers from stdin and emits the compressed representation of the numbers to\n"
                "stdout.  Or does the reverse when --decode is used.\n"
                "\n",
                argv[0]);
        return 111;
    }

    if (strcmp(argv[1], "--encode") == 0) {
        return encode();
    } else if (strcmp(argv[1], "--decode") == 0) {
        return decode();
    } else {
        fprintf(stderr, "unknown argument: %s\n", argv[1]);
        return 111;
    }
}
