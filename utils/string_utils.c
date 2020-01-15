#include "string_utils.h"

char* ufgets(FILE* file, size_t* length){
    const int alloc_ahead = 128;
    int num_reallocs = 1;

    char* ret = (char*)malloc(alloc_ahead);
    if (ret == NULL)
        goto failed_to_read;

    char c;
    int i;
    for (i = 0; (c = fgetc(file)) != '\n' && c != EOF; i++){
        if (i == num_reallocs * alloc_ahead - 1){
            char* try_realloc = (char*)realloc(ret, (++num_reallocs) * alloc_ahead);
            if (try_realloc == NULL){
                free(ret);
                goto failed_to_read;
            }
            ret = try_realloc;
        }
        ret[i] = c;
    }

    ret[i] = '\0';
    *length = i;
    return ret;


failed_to_read:
    if (length) *length = -1;
	return NULL;  
}

bool is_valid_digits(char* val, char** to_hex){
    int len = strlen(val);
    if (strncasecmp(val, "0x", 2) == 0){
        for (int i = 2; i<len; i++)
            if (!is_hex(val[i]))
                return false;
        
        snprintf(*to_hex, 3, "%02s", val+2);

        return true;
    }

    for (int i = 0; i<len; i++)
        if (val[i]<'0' || val[i] > '9')
            return false;
    
    int tmp = atoi(val);
    if (tmp > 255)
        return false;
    
    if (to_hex != NULL)
        snprintf(*to_hex, 3, "%02X", tmp);

    return true;
}