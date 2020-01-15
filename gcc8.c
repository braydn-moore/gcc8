#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stdbool.h>

#define LINE_SIZE_INITIAL 512
#define NUM_VARIABLES_INITIAL 16
#define NUM_LABELS_INITIAL 16

// Initialize the input and output files
static FILE* input, *output;
static bool failed = false;

// Line reading variables
static int index, line_size, line_num;
static char* line;

// Variable tracking
static int num_variables;
static char** variables;

// function / array tracking
typedef struct __label__{
    char* name;
    int num_args;
    int height;
} label_t;

static label_t** labels;
static num_labels;

void close_on_exit(){
    fclose(input);
    fclose(output);

    reset_variables();
    if (line != NULL)
        free(line);

    if (variables != NULL){
        reset_variables();
        free(variables);
    }
    
    if (labels != NULL){
        reset_labels();
        free(labels);
    }
}

int initialize_input(char** argv){
    input = fopen(argv[1], "r");
    if (input == NULL){
        fprintf(stderr, "Error opening the input file\n");
        return -1;
    }

    output = fopen(argv[2], "r");
    if (output == NULL){
        fprintf(stderr, "Error opening the output file\n");
        return -1;
    }

    atexit(close_on_exit);  

    line = malloc(LINE_SIZE_INITIAL);
    line_size = LINE_SIZE_INITIAL;

    variables = malloc(NUM_VARIABLES_INITIAL * sizeof(char*));
    num_variables = NUM_VARIABLES_INITIAL;

    labels = (label_t**)malloc(sizeof(label_t*) * NUM_LABELS_INITIAL);
    num_labels = NUM_LABELS_INITIAL;
}

void error(char* msg, ...){
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "Failed at line %d\n", line_num);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);
    failed = true;
    exit(1);
}

void write_to_file(char* msg, ...){
    va_list args;
    va_start(args, msg);
    vfprintf(output, msg, args);
    fprintf(output, "\n");
    va_end(args);
}

char buffer_character(){
    char next = fgetc(input);
    if (index == line_size - 1){
        char* new_line = realloc(line, line_size * 2);
        if (new_line == NULL){
            fprintf(stderr,"Failed to realloc, line too long\n");
            exit(1);
        }
        else{
            free(line);
            line = new_line;
        }
    }

    line[index++] = next == '\n' ? '\0':next;
    if (next == '\n'){
        line_num++;
        index = 0;
        write_to_file(";%s", line);
    }

    return next;
}

void reset_variables(){
    for (int i = 0; i < num_variables; i++){
        if (variables[i] != NULL)
            free(variables[i]);
        
        variables[i] = NULL;
    }

    num_variables = 0;
}

void reset_labels(){
    for (int i = 0; i < num_labels; i++){
        if (labels[i] != NULL)
            free(labels[i]);
        
        labels[i] = NULL;
    }
    num_labels = 0;
}

char read_character(){
    char next = buffer_character();
    if (next == '/'){
        next = buffer_character();
        if (next != '/')
            error("Expected \"/\" at line");
        // ignore all commented lines
        while (next != '\n')
            next = buffer_character();
    }

    return next;
}

char read_ignore_whitespace(){
    char next = buffer_character();
    while (isspace(next)) next = buffer_character();
    return next;
}

void init_compiler(){
    label_t resvered[] = {
        {strdup("draw"), 3, 0},
        {strdup("putchar"), 3, 0},
        {strdup("rand"), 0, 0},
        {strdup("getchar"), 0, 0},
        {strdup("cls"), 0, 0},
        {strdup("sizeof"), 1, 0}
    };

    for (int i = 0; i < sizeof(resvered)/sizeof(label_t); i++){
        label_t* label = malloc(sizeof(label_t));
        label->name = resvered[i].name;
        label->height = resvered[i].height;
        label->num_args = resvered[i].num_args;
        labels[i] = label;
    }
}

static inline bool is_end_of_expression(char c){
    return c == ';' || c == ',' || c == ']' || c == ')';
}

static inline bool is_end_of_operator(char c){
    return isalnum(c) || is_end_of_expression(c) || isspace(c);
}

static inline char match(const char x, const char y){
    if (x != y)
        error("Expected %c", x);
    
    return buffer_character();
}

char* read_name(){
    char input = read_ignore_whitespace();
    if (input != '_' && !isalpha(input))
        error("Names must start with a letter or underscore");
    
    char* ret = malloc(128);
    int name_index = 0;
    while (input == '_' || isalnum(input)){
        if (name_index == 127)
            error("Name too long");
        
        ret[name_index++] = input;
        input = buffer_character();
    }
    ret[name_index] = '\0';
    return ret;
}

char* read_digit(){
    char next = read_ignore_whitespace();
    
}