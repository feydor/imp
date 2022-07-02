#include "palette.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * @brief loads the <palette>, if not found loads a default
 */
int load_palette(U32Vec *buffer, const char *palette) {
    if (palette) {
        FILE *palette_fp = NULL;
        if (!(palette_fp = fopen(palette, "r"))) {
            fprintf(stderr, "Palette file not found: '%s'\n", palette);
            return -1;
        }

        int MAX_CHARS = 1024;
        char line[MAX_CHARS];
        while (fgets(line, MAX_CHARS, palette_fp)) {
            char *tok = NULL;
            tok = strtok(line, ",");
            while (tok && isalnum(*tok)) {
                uint32_t n = strtoul(tok, NULL, 16);
                U32Vec_push(buffer, n);
                tok = strtok(NULL, ",");
            }
        }
        fclose(palette_fp);
    } else {
        uint32_t default_palette[] = {
            0x800000, 0x9A6324, 0x808000, 0x469990, 0x000075, 0x000000,
            0xe6194B, 0xf58231, 0xffe119, 0xbfef45, 0x3cb44b, 0x42d4f4,
            0x4363d8, 0x911eb4, 0xf032e6, 0xa9a9a9, 0xfabed4, 0xffd8b1,
            0xfffac8, 0xaaffc3, 0xdcbeff, 0xffffff};
        U32Vec_from(buffer, default_palette,
                    sizeof(default_palette) / sizeof(default_palette[0]));
    }
    return 0;
}
