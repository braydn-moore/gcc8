#include "utils/string_utils.h"
#include <ctype.h>

int char_to_hex(char c){
    if (!is_hex(c)){
        fprintf(stderr, "Invalid hex character\n");
        return -1;
    }

    if (c >= 'A')
        return 10 + (c - 'A');
    
    return c - '0';
}

int bin_value(char low, char high){
    return (char_to_hex(low) << 4) | char_to_hex(high);
}

int main(int argc, char **argv){
    if (argc != 3){
        fprintf(stderr, "Usage: <input file> <output file>\n");
        return -1;
    }

    FILE* const in = fopen(argv[1], "r");
    FILE* const out = fopen(argv[2], "w");

    for (int c = fgetc(in); c != EOF; c = fgetc(in)){
        if (isspace(c))
            continue;
        
        fprintf(out, "%c", bin_value(c, fgetc(in)));
    }

    fclose(in);
    fclose(out);
    return 0;
}