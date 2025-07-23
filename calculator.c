#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_SIZE 16
#define MAX_BUFFER_SIZE 256
#define VALID_TOKENS                                                         \
    {                                                                        \
        "+", "-", "*", "/", "(", ")", "u", "tan", "ctg", "sin", "cos", "ln", \
            "sqrt"                                                           \
    }

typedef enum {
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_FUNCTION,
    TOKEN_END
} TokenType;

typedef enum { SUCCESS, ERROR, MATH_ERROR } Status;

typedef struct Node {
    char token[MAX_TOKEN_SIZE];
    struct Node *next;
} Node;

typedef struct {
    Node *top;
} Stack;

Status process_expression(const char *input, double *result);
Status tokenize_expression(char *str);
Status convert_to_postfix(char *str);
Status evaluate_postfix(char *str, double *result);
Status validate_expression(const char *str);

TokenType get_next_token(char **str, char token[], TokenType last_token);
int get_operator_priority(const char token[]);

void stack_push(Stack *stack, const char token[]);
Status stack_pop(Stack *stack, char token[]);
Status stack_peek(const Stack *stack, char token[]);
void stack_clear(Stack *stack);
bool stack_is_empty(const Stack *stack);

char get_next_char(char **str);
void unget_char(char **str);

void handle_error(Status status);
void trim_whitespace(char *str);

int main() {
    char input[MAX_BUFFER_SIZE];

    printf("Expression Calculator (enter 'q' to quit)\n");
    while (true) {
        printf("> ");
        if (!fgets(input, MAX_BUFFER_SIZE, stdin)) {
            break;
        }

        trim_whitespace(input);
        if (strcmp(input, "q") == 0) {
            break;
        }

        double result;
        Status status = process_expression(input, &result);

        if (status == SUCCESS) {
            printf("Result: %.6f\n", result);
        } else {
            handle_error(status);
        }
    }

    return 0;
}

Status process_expression(const char *input, double *result) {
    char buffer[MAX_BUFFER_SIZE];
    strncpy(buffer, input, MAX_BUFFER_SIZE);
    buffer[MAX_BUFFER_SIZE - 1] = '\0';

    Status status = tokenize_expression(buffer);
    if (status != SUCCESS) return status;

    status = validate_expression(buffer);
    if (status != SUCCESS) return status;

    status = convert_to_postfix(buffer);
    if (status != SUCCESS) return status;

    return evaluate_postfix(buffer, result);
}

Status tokenize_expression(char *str) {
    char buffer[MAX_BUFFER_SIZE] = {0};
    char token[MAX_TOKEN_SIZE];
    char *ptr = str;
    int index = 0;
    Status status = SUCCESS;
    TokenType last_type = TOKEN_END;
    TokenType type = TOKEN_END;

    char *newline = strchr(str, '\n');
    if (newline) *newline = '\0';

    while (status == SUCCESS &&
           (type = get_next_token(&ptr, token, last_type)) != TOKEN_END) {
        if (index + strlen(token) >= MAX_BUFFER_SIZE - 2) {
            status = ERROR;
            break;
        }

        strcpy(buffer + index, token);
        index += strlen(token);
        buffer[index++] = ' ';
        last_type = type;
    }

    if (index > 0) buffer[--index] = '\0';
    strcpy(str, buffer);
    return status;
}

Status convert_to_postfix(char *str) {
    TokenType last_type = TOKEN_END;
    TokenType type = TOKEN_END;
    char buffer[MAX_BUFFER_SIZE] = {0};
    char token[MAX_TOKEN_SIZE], top_token[MAX_TOKEN_SIZE];
    char *ptr = str;
    int index = 0;
    Stack stack = {NULL};
    Status status = SUCCESS;

    while ((type = get_next_token(&ptr, token, last_type)) != TOKEN_END) {
        switch (type) {
            case TOKEN_NUMBER:
                if (last_type == TOKEN_NUMBER) {
                    status = ERROR;
                } else {
                    strcat(buffer, token);
                    strcat(buffer, " ");
                    index += strlen(token) + 1;
                }
                break;

            case TOKEN_LBRACKET:
                stack_push(&stack, token);
                break;

            case TOKEN_RBRACKET:
                while (stack_peek(&stack, top_token) == SUCCESS &&
                       strcmp(top_token, "(") != 0) {
                    stack_pop(&stack, top_token);
                    strcat(buffer, top_token);
                    strcat(buffer, " ");
                    index += strlen(top_token) + 1;
                }

                if (stack_is_empty(&stack)) {
                    status = ERROR;
                } else {
                    stack_pop(&stack, top_token);
                }
                break;

            case TOKEN_OPERATOR:
            case TOKEN_FUNCTION:
                if (last_type == type) {
                    status = ERROR;
                } else {
                    while (stack_peek(&stack, top_token) == SUCCESS &&
                           get_operator_priority(top_token) >=
                               get_operator_priority(token)) {
                        stack_pop(&stack, top_token);
                        strcat(buffer, top_token);
                        strcat(buffer, " ");
                        index += strlen(top_token) + 1;
                    }
                    stack_push(&stack, token);
                }
                break;

            default:
                status = ERROR;
        }

        if (status != SUCCESS) break;
        last_type = type;
    }

    while (status == SUCCESS && !stack_is_empty(&stack)) {
        stack_pop(&stack, top_token);
        strcat(buffer, top_token);
        strcat(buffer, " ");
    }

    if (index > 0) buffer[strlen(buffer) - 1] = '\0';
    strcpy(str, buffer);
    stack_clear(&stack);
    return status;
}

Status evaluate_postfix(char *str, double *result) {
    char token[MAX_TOKEN_SIZE];
    char operand1[MAX_TOKEN_SIZE], operand2[MAX_TOKEN_SIZE];
    char tmp_result[MAX_TOKEN_SIZE];
    TokenType last_type = TOKEN_END;
    TokenType type = TOKEN_END;
    Stack stack = {NULL};
    Status status = SUCCESS;

    while ((type = get_next_token(&str, token, last_type)) != TOKEN_END) {
        if (type == TOKEN_NUMBER) {
            stack_push(&stack, token);
        } else {
            double a, b, res;

            if (strcmp(token, "u") == 0) {
                if (stack_pop(&stack, operand1) != SUCCESS) {
                    status = ERROR;
                    break;
                }
                a = atof(operand1);
                res = -a;
            } else if (strcmp(token, "+") == 0 || strcmp(token, "-") == 0 ||
                       strcmp(token, "*") == 0 || strcmp(token, "/") == 0) {
                if (stack_pop(&stack, operand1) != SUCCESS ||
                    stack_pop(&stack, operand2) != SUCCESS) {
                    status = ERROR;
                    break;
                }

                a = atof(operand1);
                b = atof(operand2);

                if (strcmp(token, "+") == 0)
                    res = b + a;
                else if (strcmp(token, "-") == 0)
                    res = b - a;
                else if (strcmp(token, "*") == 0)
                    res = b * a;
                else if (a == 0.0) {
                    status = MATH_ERROR;
                    break;
                } else {
                    res = b / a;
                }
            } else {
                if (stack_pop(&stack, operand1) != SUCCESS) {
                    status = ERROR;
                    break;
                }
                a = atof(operand1);

                if (strcmp(token, "sin") == 0)
                    res = sin(a);
                else if (strcmp(token, "cos") == 0)
                    res = cos(a);
                else if (strcmp(token, "tan") == 0)
                    res = tan(a);
                else if (strcmp(token, "ctg") == 0) {
                    if (tan(a) == 0.0) {
                        status = MATH_ERROR;
                        break;
                    }
                    res = 1.0 / tan(a);
                } else if (strcmp(token, "ln") == 0) {
                    if (a <= 0.0) {
                        status = MATH_ERROR;
                        break;
                    }
                    res = log(a);
                } else if (strcmp(token, "sqrt") == 0) {
                    if (a < 0.0) {
                        status = MATH_ERROR;
                        break;
                    }
                    res = sqrt(a);
                } else {
                    status = ERROR;
                    break;
                }
            }

            snprintf(tmp_result, MAX_TOKEN_SIZE, "%.6f", res);
            stack_push(&stack, tmp_result);
        }

        last_type = type;
    }

    if (status == SUCCESS) {
        if (stack_pop(&stack, token) != SUCCESS || !stack_is_empty(&stack)) {
            status = ERROR;
        } else {
            *result = atof(token);
        }
    }

    stack_clear(&stack);
    return status;
}

Status validate_expression(const char *str) {
    char token[MAX_TOKEN_SIZE];
    char *ptr = (char *)str;
    TokenType last_type = TOKEN_END;
    TokenType type = TOKEN_END;
    int brackets_count = 0;
    Status status = SUCCESS;

    char valid_tokens[][MAX_TOKEN_SIZE] = VALID_TOKENS;
    size_t valid_tokens_count = sizeof(valid_tokens) / sizeof(valid_tokens[0]);

    while ((type = get_next_token(&ptr, token, last_type)) != TOKEN_END) {
        if (type == TOKEN_OPERATOR) {
            bool valid = false;
            for (size_t i = 0; i < valid_tokens_count; i++) {
                if (strcmp(token, valid_tokens[i]) == 0) {
                    valid = true;
                    break;
                }
            }
            if (!valid) {
                status = ERROR;
                break;
            }
        }

        if (strcmp(token, "(") == 0) {
            brackets_count++;
        } else if (strcmp(token, ")") == 0) {
            brackets_count--;
            if (brackets_count < 0) {
                status = ERROR;
                break;
            }
        }

        last_type = type;
    }

    if (brackets_count != 0) {
        status = ERROR;
    }

    return status;
}

TokenType get_next_token(char **str, char token[], TokenType last_token) {
    int i = 0;
    char c;

    while ((c = get_next_char(str)) == ' ' || c == '\t');

    if (c == '\0') {
        return TOKEN_END;
    }

    token[i++] = c;
    token[1] = '\0';

    if (isdigit(c)) {
        while (isdigit((c = get_next_char(str)))) {
            token[i++] = c;
        }

        if (c == '.') {
            token[i++] = c;
            while (isdigit((c = get_next_char(str)))) {
                token[i++] = c;
            }
        }

        token[i] = '\0';
        unget_char(str);
        return TOKEN_NUMBER;
    } else if (c == '-' && last_token != TOKEN_NUMBER &&
               last_token != TOKEN_RBRACKET) {
        token[0] = 'u';
        return TOKEN_OPERATOR;
    } else if (c == '(') {
        return TOKEN_LBRACKET;
    } else if (c == ')') {
        return TOKEN_RBRACKET;
    } else if (isalpha(c)) {
        while (isalpha((c = get_next_char(str)))) {
            token[i++] = c;
        }
        token[i] = '\0';
        unget_char(str);
        return (i == 1) ? TOKEN_OPERATOR : TOKEN_FUNCTION;
    } else {
        return TOKEN_OPERATOR;
    }
}

int get_operator_priority(const char token[]) {
    if (strcmp(token, "+") == 0 || strcmp(token, "-") == 0) return 1;
    if (strcmp(token, "*") == 0 || strcmp(token, "/") == 0) return 2;
    if (strcmp(token, "tan") == 0 || strcmp(token, "ctg") == 0 ||
        strcmp(token, "sin") == 0 || strcmp(token, "cos") == 0 ||
        strcmp(token, "ln") == 0 || strcmp(token, "sqrt") == 0)
        return 3;
    if (strcmp(token, "u") == 0) return 4;
    return 0;
}

void stack_push(Stack *stack, const char token[]) {
    Node *node = malloc(sizeof(Node));
    if (node) {
        strncpy(node->token, token, MAX_TOKEN_SIZE);
        node->next = stack->top;
        stack->top = node;
    }
}

Status stack_pop(Stack *stack, char token[]) {
    if (stack_is_empty(stack)) return ERROR;

    Node *top = stack->top;
    strncpy(token, top->token, MAX_TOKEN_SIZE);
    stack->top = top->next;
    free(top);
    return SUCCESS;
}

Status stack_peek(const Stack *stack, char token[]) {
    if (stack_is_empty(stack)) return ERROR;
    strncpy(token, stack->top->token, MAX_TOKEN_SIZE);
    return SUCCESS;
}

void stack_clear(Stack *stack) {
    while (!stack_is_empty(stack)) {
        Node *top = stack->top;
        stack->top = top->next;
        free(top);
    }
}

bool stack_is_empty(const Stack *stack) { return stack->top == NULL; }

char get_next_char(char **str) { return *(*str)++; }

void unget_char(char **str) { (*str)--; }

void handle_error(Status status) {
    switch (status) {
        case ERROR:
            printf("Error: Invalid expression\n");
            break;
        case MATH_ERROR:
            printf("Error: Math error (division by zero, etc.)\n");
            break;
        default:
            printf("Error: Unknown error\n");
    }
}

void trim_whitespace(char *str) {
    char *end;

    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) return;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    *(end + 1) = 0;
}