/***********************************************************************
 * Copyright (c) 2013, 2014, 2015 Thomas Daede, Cory Fields            *
 * Distributed under the MIT software license, see the accompanying    *
 * file COPYING or https://www.opensource.org/licenses/mit-license.php.*
 ***********************************************************************/

#include <inttypes.h>
#include <stdio.h>

#include "../include/secp256k1.h"
#include "assumptions.h"
#include "util.h"
#include "group.h"
#include "ecmult_gen.h"
#include "ecmult_gen_prec_impl.h"

int main(int argc, char **argv) {
    const char outfile[] = "src/ecmult_gen_static_prec_table.h";
    FILE* fp;
    int bits;

    (void)argc;
    (void)argv;

    fp = fopen(outfile, "w");
    if (fp == NULL) {
        fprintf(stderr, "Could not open %s for writing!\n", outfile);
        return -1;
    }

    fprintf(fp, "/* This file was automatically generated by gen_ecmult_gen_static_prec_table. */\n");
    fprintf(fp, "/* See ecmult_gen_impl.h for details about the contents of this file. */\n");
    fprintf(fp, "#ifndef SECP256K1_ECMULT_GEN_STATIC_PREC_TABLE_H\n");
    fprintf(fp, "#define SECP256K1_ECMULT_GEN_STATIC_PREC_TABLE_H\n");

    fprintf(fp, "#include \"group.h\"\n");

    fprintf(fp, "#define S(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) "
            "SECP256K1_GE_STORAGE_CONST(0x##a##u,0x##b##u,0x##c##u,0x##d##u,0x##e##u,0x##f##u,0x##g##u,"
            "0x##h##u,0x##i##u,0x##j##u,0x##k##u,0x##l##u,0x##m##u,0x##n##u,0x##o##u,0x##p##u)\n");

    fprintf(fp, "#ifdef EXHAUSTIVE_TEST_ORDER\n");
    fprintf(fp, "static secp256k1_ge_storage secp256k1_ecmult_gen_prec_table[ECMULT_GEN_PREC_N(ECMULT_GEN_PREC_BITS)][ECMULT_GEN_PREC_G(ECMULT_GEN_PREC_BITS)];\n");
    fprintf(fp, "#else\n");
    fprintf(fp, "static const secp256k1_ge_storage secp256k1_ecmult_gen_prec_table[ECMULT_GEN_PREC_N(ECMULT_GEN_PREC_BITS)][ECMULT_GEN_PREC_G(ECMULT_GEN_PREC_BITS)] = {\n");

    for (bits = 2; bits <= 8; bits *= 2) {
        int g = ECMULT_GEN_PREC_G(bits);
        int n = ECMULT_GEN_PREC_N(bits);
        int inner, outer;

        secp256k1_ge_storage* table = checked_malloc(&default_error_callback, n * g * sizeof(secp256k1_ge_storage));
        secp256k1_ecmult_gen_create_prec_table(table, &secp256k1_ge_const_g, bits);

        fprintf(fp, "#if ECMULT_GEN_PREC_BITS == %d\n", bits);
        for(outer = 0; outer != n; outer++) {
            fprintf(fp,"{");
            for(inner = 0; inner != g; inner++) {
                fprintf(fp, "S(%"PRIx32",%"PRIx32",%"PRIx32",%"PRIx32",%"PRIx32",%"PRIx32",%"PRIx32",%"PRIx32
                            ",%"PRIx32",%"PRIx32",%"PRIx32",%"PRIx32",%"PRIx32",%"PRIx32",%"PRIx32",%"PRIx32")",
                        SECP256K1_GE_STORAGE_CONST_GET(table[outer * g + inner]));
                if (inner != g - 1) {
                    fprintf(fp,",\n");
                } 
            }
            if (outer != n - 1) {
                fprintf(fp,"},\n");
            } else {
                fprintf(fp,"}\n");
            }
        }
        fprintf(fp, "#endif\n");
        free(table);
    }

    fprintf(fp, "};\n");
    fprintf(fp, "#endif /* EXHAUSTIVE_TEST_ORDER */\n");
    fprintf(fp, "#undef SC\n");
    fprintf(fp, "#endif /* SECP256K1_ECMULT_GEN_STATIC_PREC_TABLE_H */\n");
    fclose(fp);

    return 0;
}
