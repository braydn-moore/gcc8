#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>
#include <ctype.h>

#include "utils/binary_tree.h"
#include "utils/string_utils.h"

static FILE* input, *output;
static char* input_name, *output_name;
static node_t* head;

static bool failure;
static bool free_tree;

#define is_valid_register(val) (strlen(val) == 2 && (val[0] == 'v' || val[0] == 'V') && is_hex(val[1]))

void shutdown(){
    if (input) fclose(input);
    if (output) fclose(output);
    if (failure)
        remove(output_name);
    if (free_tree)
        destroy_tree(head);
}

void open_files(char* input_file, char* output_file){
    input = fopen(input_file, "r");
    if (input == NULL){
        fprintf(stderr, "[x] Could not open input file\n");
        exit(1);
    }

    output = fopen(output_file, "w");
    if (output == NULL){
        fprintf(stderr, "Could not open output file\n");
        exit(1);
    }

    atexit(shutdown);
}

void scan_for_labels(node_t** head){
    unsigned address = 0x202;
    char* line;
    size_t len = 0;
    int line_num = 0;
    // read our file under we reach the end
    while (1){
        if (feof(input))
            return;

        line = ufgets(input, &len);
        char* to_free = line;
        if (len == -1)
            return;
        
        char* semicolon = strchr(line, ';');
        if (semicolon) *semicolon = '\0';

        // check for a colon, if there is none then we can ignore this
        // line as there is no label
        char* colon = strchr(line, ':');
        if (colon){
            char* label = strtok(line, "\t :");
            if (tree_insert(head, strdup(label), address) == NULL){
                fprintf(stderr, "[x] Double declaration of %s at line %d\n", label, line_num);
                exit(1);
            }
        }

        char* operand = strtok(colon ? NULL : line, "\t ");
        if (operand)
            address += strcasecmp(operand, "DB") == 0? 1 : 2;

    break_line:
        free(to_free);
        line_num++;
    }
}

void gen_entry(node_t* node, char* entry){
    node_t* entry_node = tree_search(node, entry);
    if (!entry_node){
        fprintf(stderr, "[x] Error, no entry function main found\n");
        exit(1);
    }

    fprintf(output, "1%03X\n", entry_node->addr);
}

typedef int (*hex_gen)(char* operands, node_t* head);

int add(char* operand, node_t* head){
    char* operand1 = strtok(operand, "\t ,");
    char* operand2 = strtok(NULL, "\t ");

    char* hex = malloc(3);
    int ret = 0;

    if (strlen(operand1) == 2 && (operand1[0] == 'v' || operand1[0] == 'V') && is_hex(operand1[1]) && is_valid_digits(operand2, &hex))
        fprintf(output, "7%c%s\n", operand1[1], hex);
    
    else if (is_valid_register(operand1) && is_valid_register(operand2))
        fprintf(output, "8%c%c4\n", operand1[1], operand2[1]);

    else if (operand1[0] == 'I' && is_valid_register(operand2))
        fprintf(output, "F%c1E\n", operand2[1]);
    else
        ret = 1;
    
    free(hex);
    return ret;
}

int and(char* operand, node_t* labels){
    char* operand1 = strtok(operand, "\t, ");
    char* operand2 = strtok(NULL, "\t ");

    if (is_valid_register(operand1) && is_valid_register(operand2))
        fprintf(output, "8%c%c2\n", operand1[1], operand2[1]);
    else
        return 1;
    
    return 0;
}

int load(char* operand, node_t* labels){
    char* operand1 = strtok(operand, "\t, ");
    char* operand2 = strtok(NULL, "\t ");

    int ret = 0;

    char* hex = malloc(3);

    if (strlen(operand1) == 2 && operand1[0] == 'D' && operand1[1] == 'T' && is_valid_register(operand2))
        fprintf(output, "F%c15\n", operand2[1]);

    else if (strlen(operand1) == 2 && operand1[0] == 'S' && operand1[1] == 'T' && is_valid_register(operand2))
        fprintf(output, "F%c18\n", operand2[1]);
    else if (strlen(operand1) == 1 && operand1[0] == 'F' && is_valid_register(operand2))
        fprintf(output, "F%c29\n", operand2[1]);
    else if (strlen(operand1) == 1 && operand1[0] == 'B' && is_valid_register(operand2))
        fprintf(output, "F%c33\n", operand2[1]);
    else if (is_valid_register(operand1) && strlen(operand2) == 2 && operand2[0] == 'D' && operand2[1] == 'T')
        fprintf(output, "F%c07\n", operand1[1]);
    else if (is_valid_register(operand1) && is_valid_register(operand2))
        fprintf(output, "8%c%c0\n", operand1[1], operand2[1]);
    else if (is_valid_register(operand1) && strlen(operand2) == 1 && operand2[0] == 'K')
        fprintf(output, "F%c0A\n", operand1[1]);
    else if (is_valid_register(operand1) && is_valid_digits(operand2, &hex))
        fprintf(output, "6%c%s\n", operand1[1], hex);
    else if (strlen(operand2) == 3 && operand2[0] == '[' && operand2[1] == 'I' && operand2[2] == ']'
                && is_valid_register(operand1))
        fprintf(output, "F%c65\n", operand1[1]);
    else if (strlen(operand1) == 3 && operand1[0] == '[' && operand1[1] == 'I' && operand1[2] == ']'
                && is_valid_register(operand2))
        fprintf(output, "F%c55\n", operand2[1]);
    else if (strlen(operand1) == 1 && operand1[0] == 'I'){
        node_t* found_node = tree_search(labels, operand2);
        if (found_node == NULL)
            ret = 2;
        else
            fprintf(output, "A%03X\n", found_node->addr);
    }
    else
        ret = 1;
    
    free(hex);
    return ret;
}

int or(char* operand, node_t* labels){
    char* operand1 = strtok(operand, "\t, ");
    char* operand2 = strtok(NULL, "\t ");

    if (is_valid_register(operand1) && is_valid_register(operand2))
        fprintf(output, "8%c%c1\n", operand1[1], operand2[1]);
    else
        return 1;
    
    return 0;
}

int drw(char* operands, node_t* labels){
    char* operand1 = strtok(operands, "\t, ");
    char* operand2 = strtok(NULL, "\t, ");
    char* nibble = strtok(NULL, "\t ");

    char* hex = malloc(3);
    memset(hex, 0, 3);

    int ret = 0;

    if (is_valid_register(operand1) && is_valid_register(operand2)
            && is_valid_digits(nibble, &hex))
                fprintf(output, "D%c%c%c\n", operand1[1], operand2[1], hex[1]);
    else
        ret = 1;
    
    free(hex);
    return ret;
}

int call(char* operands, node_t* labels){
    char* addr_to_call = strtok(operands, "\t, ");
    node_t* found = tree_search(labels, addr_to_call);
    if (found == NULL)
        return 1;
    
    fprintf(output, "2%03X\n", found->addr);
    return 0;
}

int cls(char* operands, node_t* labels){
    fprintf(output, "00E0\n");
    return 0;
}

int db(char* operands, node_t* labels){
    char* value = strtok(operands, "\t, ");
    char* hex = malloc(3);
    memset(hex, 0, 3);
    int ret = 0;

    if (is_valid_digits(value, &hex))
        fprintf(output, "%s\n", hex);
    else
        ret = 1;

    free(hex);
    return ret;
}

int jmp(char* operands, node_t* labels){
    char* operand1 = strtok(operands, "\t ,");
    char* operand2 = strtok(NULL, "\t ");
    node_t* found;

    if (is_valid_register(operand1) && (found = tree_search(labels, operand2)))
        fprintf(output, "B%03X\n", found->addr);
    else if ((found = tree_search(labels, operand1)))
        fprintf(output, "1%03X\n", found->addr);
    else
        return 1;
    
    return 0;
}

int ret(char* operands, node_t* labels){
    fprintf(output, "00EE\n");
    return 0;
}

int rnd(char* operands, node_t* labels){
    char* operand1 = strtok(operands, "\t, ");
    char* operand2 = strtok(NULL, "\t ");

    char* hex = malloc(3);
    int ret = 0;

    if (is_valid_register(operand1) && is_valid_digits(operand2, &hex))
        fprintf(output, "C%c%s\n", operand1[1], hex);
    else
        ret = 1;
    
    return ret;
}

int shift_left(char* operands, node_t* labels){
    char* operand1 = strtok(operands, "\t, ");
    char* operand2 = strtok(NULL, "\t ");

    if (is_valid_register(operand1) && is_valid_register(operand2))
        fprintf(output, "8%c%cE\n", operand1[1], operand2[1]);
    else
        return 1;

    return 0;
}

int shift_right(char* operands, node_t* labels){
    char* operand1 = strtok(operands, "\t, ");
    char* operand2 = strtok(NULL, "\t ");

    if (is_valid_register(operand1) && is_valid_register(operand2))
        fprintf(output, "8%c%c6\n", operand1[1], operand2[1]);
    else
        return 1;
    
    return 0;
}

int sub(char* operands, node_t* labels){
    char* operand1 = strtok(operands, "\t, ");
    char* operand2 = strtok(NULL, "\t ");

    if (is_valid_register(operand1) && is_valid_register(operand2))
        fprintf(output, "8%c%c5\n", operand1[1], operand2[1]);
    else
        return 1;
    
    return 0;
}

int subn(char* operands, node_t* labels){
    char* operand1 = strtok(operands, "\t, ");
    char* operand2 = strtok(NULL, "\t ");

    if (is_valid_register(operand1) && is_valid_register(operand2))
        fprintf(output, "8%c%c7\n", operand1[1], operand2[1]);
    else
        return 1;
    
    return 0;
}

int se(char* operands, node_t* labels){
    char* operand1 = strtok(operands, "\t, ");
    char* operand2 = strtok(NULL, "\t ");

    char* hex = malloc(3);
    memset(hex, 0, 3);
    int ret = 0;

    if (is_valid_register(operand1) && is_valid_register(operand2))
        fprintf(output, "5%c%c0\n", operand1[1], operand2[1]);
    else if (is_valid_register(operand1) && is_valid_digits(operand2, &hex))
        fprintf(output, "3%c%s\n", operand1[1], hex);
    else
        ret = 1;
    
    free(hex);
    return ret;
}

int sne(char* operands, node_t* labels){
    char* operand1 = strtok(operands, "\t, ");
    char* operand2 = strtok(NULL, "\t ");

    char* hex = malloc(3);
    int ret = 0;

    if (is_valid_register(operand1) && is_valid_register(operand2))
        fprintf(output, "9%c%c0\n", operand1[1], operand2[1]);
    else if (is_valid_register(operand1) && is_valid_digits(operand2, &hex))
        fprintf(output, "4%c%s\n", operand1[1], hex);
    else
        ret = 1;
    
    free(hex);
    return ret;
}

int skp(char* operands, node_t* labels){
    char* operand1 = strtok(operands, "\t ");
    if (is_valid_register(operand1))
        fprintf(output, "E%c9E\n", operand1[1]);
    else
        return 1;
    
    return 0;
}

int sknp(char* operands, node_t* labels){
    char* operand1 = strtok(operands, "\t ");
    if (is_valid_register(operand1))
        fprintf(output, "E%cA1\n", operand1[1]);
    else
        return 1;
    
    return 0;
}

int xor(char* operands, node_t* labels){
    char* operand1 = strtok(operands, "\t, ");
    char* operand2 = strtok(NULL, "\t ");

    if (is_valid_register(operand1) && is_valid_register(operand2))
        fprintf(output, "8%c%c3\n", operand1[1], operand2[1]);
    else
        return 1;
    
    return 0;
}

hex_gen functions[] = {add, and, call, cls, db, drw, jmp, load, or, ret, rnd, se,
    shift_left, shift_right, sknp, skp, sne, sub, subn, xor};

const char* commands[] = {"ADD","AND","CALL","CLS","DB","DRW","JP","LD","OR","RET","RND","SE",
    "SHL","SHR","SKNP","SKP","SNE","SUB","SUBN","XOR"};

static int compare(const void* a, const void* b){
    return strcmp((char*)a, *(char**)b);
}

int write_assembly(char* command, char* operands, node_t* labels){
    const char** function = (const char**) bsearch(
        command,
        commands,
        sizeof(commands) / sizeof(*commands),
        sizeof(*commands),
        compare);
    
    if(!function)
        return 3;
    return (*functions[function - commands])(operands, labels);
}

void generate_hex(node_t* labels){
    unsigned address = 0x202;
    char* line;
    size_t len = 0;
    int line_num = 0;
    // read our file under we reach the end
    while (1){
        if (feof(input))
            return;

        line = ufgets(input, &len);
        char* to_free = line;
        if (len == -1)
            return;
        
        else if (len == 0)
            goto line_done;

        char* semicolon = strchr(line, ';');
        if (semicolon) *semicolon = '\0';

        // if we have a label remove it as we are checking for opcodes
        char* colon = strchr(line, ':');
        if (colon) strtok(line, "\t :");

        char* command = strtok(colon? NULL : line, "\t ");
        char* operands = strtok(NULL, "");

        if (!command)
            goto line_done;

        int error = write_assembly(command, operands, labels);

        if(error){
            const char* types[] = {
                "no error",
                "operand formatting",
                "label not found",
                "unsupported chip8 mnemonic"
            };
            line_num++;
            failure = true;
            free(line);
            fprintf(stderr, "error: line %d: %s\n", line_num, types[error]);
            exit(1);
        }
    line_done:
        free(to_free);
        line_num++;
    }
}

int main(int argc, char** argv){
    if (argc != 3){
        fprintf(stderr, "[x] Expected input and output arguments\n");
        exit(1);
    }

    open_files(argv[1], argv[2]);
    head = create_tree();

    scan_for_labels(&head);
    free_tree = true;
    rewind(input);
    rewind(output);

    gen_entry(head, "main");
    generate_hex(head);

    destroy_tree(head);
    free_tree = false;
    exit(0);
}