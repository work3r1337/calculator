#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_SIZE 10
#define MAX_BUFFER_SIZE 100
#define VALID_TOKENS                                                         \
    {                                                                        \
        "+", "-", "*", "/", "(", ")", "u", "tan", "ctg", "sin", "cos", "ln", \
            "sqrt"                                                           \
    }
#define VALID_TOKENS_SIZE \
    (int)(sizeof(valid_tokens) / (sizeof(char) * MAX_TOKEN_SIZE))

typedef enum {
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_FUNCTION,
    TOKEN_END
} TokenType;

typedef enum { SUCCESS, ERROR } Status;

typedef struct Node {
    char token[MAX_TOKEN_SIZE];
    struct Node *next;
} Node;

typedef struct {
    Node *top;
} Stack;

Status tokenization(char str[]);
Status infix_to_postfix(char str[]);
Status calc_rpn(char str[], double *result);
Status validate(char str[]);
TokenType get_token(char **str, char token[], TokenType last_token);
int get_priority(const char token[]);
void push(Stack *stack_p, const char token[]);
Status pop(Stack *stack_p, char token[]);
Status peek(const Stack *stack_p, char token[]);
void clean(Stack *stack_p);
int is_empty(const Stack *stack_p);
char getch(char **str);
void ungetch(char **str);

int main() {
    while (true) {
        char str[MAX_BUFFER_SIZE];
        int index = 0;
        printf("Exit(q)\nExpression: ");
        while ((str[index++] = getchar()) != '\n') {
            if (index > MAX_BUFFER_SIZE - 1) {
                printf("n/a");
                while (getchar() != '\n');
                continue;
            }
        }
        if (index == 2 && str[0] == 'q') break;
        str[index] = '\0';
        double result;
        if (tokenization(str) == SUCCESS && validate(str) == SUCCESS &&
            infix_to_postfix(str) == SUCCESS &&
            calc_rpn(str, &result) == SUCCESS)
            printf("Result: %lf\n", result);
        else
            printf("n/a\n");
    }

    return 0;
}

Status tokenization(char str[]) {
    char buffer[MAX_BUFFER_SIZE];
    char token[MAX_TOKEN_SIZE];
    char *tmp = str;
    int index = 0;
    Status status = SUCCESS;
    TokenType last_type = TOKEN_END;
    TokenType type = TOKEN_END;
    *strchr(str, '\n') = '\0';
    while (status == SUCCESS &&
           (type = get_token(&tmp, token, last_type)) != TOKEN_END) {
        int i = 0;
        while (status == SUCCESS && token[i] != '\0') {
            buffer[index++] = token[i++];
            if (index >= MAX_BUFFER_SIZE - 1) status = ERROR;
        }
        buffer[index++] = ' ';
        last_type = type;
    }
    buffer[--index] = '\0';
    memcpy(str, buffer, MAX_BUFFER_SIZE);
    return status;
}

Status infix_to_postfix(char str[]) {
    TokenType last_type = TOKEN_END;
    TokenType type = TOKEN_END;
    char buffer[MAX_BUFFER_SIZE];
    char token[MAX_TOKEN_SIZE], tmp_token[MAX_TOKEN_SIZE];
    char *tmp = str;
    int index = 0;
    Stack stack;
    stack.top = NULL;
    Status status = SUCCESS;
    while ((type = get_token(&tmp, token, last_type)) != TOKEN_END) {
        if (type == TOKEN_NUMBER) {
            if (last_type == TOKEN_NUMBER) status = ERROR;
            int i = 0;
            while (token[i] != '\0') buffer[index++] = token[i++];
            buffer[index++] = ' ';
        } else if (type == TOKEN_LBRACKET) {
            push(&stack, token);
        } else if (type == TOKEN_RBRACKET) {
            peek(&stack, tmp_token);
            while (!is_empty(&stack) && strcmp(tmp_token, "(") != 0) {
                int i = 0;
                if (pop(&stack, tmp_token) == ERROR) status = ERROR;
                while (tmp_token[i] != '\0') buffer[index++] = tmp_token[i++];
                buffer[index++] = ' ';
                peek(&stack, tmp_token);
            }
            pop(&stack, tmp_token);
        } else if (type == TOKEN_OPERATOR) {
            if (last_type == TOKEN_OPERATOR) status = ERROR;
            peek(&stack, tmp_token);
            while (!is_empty(&stack) &&
                   get_priority(tmp_token) >= get_priority(token)) {
                if (pop(&stack, tmp_token) == ERROR) status = ERROR;
                int i = 0;
                while (tmp_token[i] != '\0') buffer[index++] = tmp_token[i++];
                buffer[index++] = ' ';
                peek(&stack, tmp_token);
            }
            push(&stack, token);
        } else if (type == TOKEN_FUNCTION) {
            if (last_type == TOKEN_FUNCTION) status = ERROR;
            peek(&stack, tmp_token);
            while (!is_empty(&stack) &&
                   get_priority(tmp_token) >= get_priority(token)) {
                if (pop(&stack, tmp_token) == ERROR) status = ERROR;
                int i = 0;
                while (tmp_token[i] != '\0') buffer[index++] = tmp_token[i++];
                buffer[index++] = ' ';
                peek(&stack, tmp_token);
            }
            push(&stack, token);
        } else
            status = ERROR;
        last_type = type;
    }
    while (!is_empty(&stack)) {
        if (pop(&stack, tmp_token) == ERROR) status = ERROR;
        int i = 0;
        while (tmp_token[i] != '\0') buffer[index++] = tmp_token[i++];
        buffer[index++] = ' ';
    }
    buffer[--index] = '\0';
    strcpy(str, buffer);
    clean(&stack);
    return status;
}

Status calc_rpn(char str[], double *result) {
    TokenType last_type = TOKEN_END;
    TokenType type = TOKEN_END;
    char token[MAX_TOKEN_SIZE];
    char tmp_token1[MAX_TOKEN_SIZE], tmp_token2[MAX_TOKEN_SIZE];
    char tmp[MAX_TOKEN_SIZE];
    Stack stack;
    stack.top = NULL;
    Status status = SUCCESS;

    while ((type = get_token(&str, token, last_type)) != TOKEN_END) {
        if (type == TOKEN_NUMBER)
            push(&stack, token);
        else if (strcmp(token, "+") == 0) {
            if (pop(&stack, tmp_token1) == ERROR) status = ERROR;
            if (pop(&stack, tmp_token2) == ERROR) status = ERROR;
            sprintf(tmp, "%lf", atof(tmp_token1) + atof(tmp_token2));
            push(&stack, tmp);
        } else if (strcmp(token, "*") == 0) {
            if (pop(&stack, tmp_token1) == ERROR) status = ERROR;
            if (pop(&stack, tmp_token2) == ERROR) status = ERROR;
            sprintf(tmp, "%lf", atof(tmp_token1) * atof(tmp_token2));
            push(&stack, tmp);
        } else if (strcmp(token, "-") == 0) {
            if (pop(&stack, tmp_token1) == ERROR) status = ERROR;
            if (pop(&stack, tmp_token2) == ERROR) status = ERROR;
            sprintf(tmp, "%lf", atof(tmp_token2) - atof(tmp_token1));
            push(&stack, tmp);
        } else if (strcmp(token, "/") == 0) {
            if (pop(&stack, tmp_token1) == ERROR) status = ERROR;
            if (pop(&stack, tmp_token2) == ERROR) status = ERROR;
            if (atof(tmp_token1) != 0.0) {
                sprintf(tmp, "%lf", atof(tmp_token2) / atof(tmp_token1));
                push(&stack, tmp);
            } else
                status = ERROR;
        } else if (strcmp(token, "u") == 0) {
            if (pop(&stack, tmp_token1) == ERROR) status = ERROR;
            sprintf(tmp, "%lf", -atof(tmp_token1));
            push(&stack, tmp);
        } else if (strcmp(token, "tan") == 0) {
            if (pop(&stack, tmp_token1) == ERROR) status = ERROR;
            sprintf(tmp, "%lf", tan(atof(tmp_token1)));
            push(&stack, tmp);
        } else if (strcmp(token, "ctg") == 0) {
            if (pop(&stack, tmp_token1) == ERROR) status = ERROR;
            sprintf(tmp, "%lf", 1.0 / tan(atof(tmp_token1)));
            push(&stack, tmp);
        } else if (strcmp(token, "sin") == 0) {
            if (pop(&stack, tmp_token1) == ERROR) status = ERROR;
            sprintf(tmp, "%lf", sin(atof(tmp_token1)));
            push(&stack, tmp);
        } else if (strcmp(token, "cos") == 0) {
            if (pop(&stack, tmp_token1) == ERROR) status = ERROR;
            sprintf(tmp, "%lf", cos(atof(tmp_token1)));
            push(&stack, tmp);
        } else if (strcmp(token, "ln") == 0) {
            if (pop(&stack, tmp_token1) == ERROR) status = ERROR;
            sprintf(tmp, "%lf", log(atof(tmp_token1)));
            push(&stack, tmp);
        } else if (strcmp(token, "sqrt") == 0) {
            if (pop(&stack, tmp_token1) == ERROR) status = ERROR;
            sprintf(tmp, "%lf", sqrt(atof(tmp_token1)));
            push(&stack, tmp);
        } else
            status = ERROR;
    }
    if (pop(&stack, tmp_token1) == ERROR) status = ERROR;
    clean(&stack);
    *result = atof(tmp_token1);
    return status;
}

Status validate(char str[]) {
    TokenType last_type = TOKEN_END;
    TokenType type = TOKEN_END;
    int brackets_count = 0;
    char token[MAX_TOKEN_SIZE];
    char valid_tokens[][MAX_TOKEN_SIZE] = VALID_TOKENS;
    Status status = SUCCESS;
    while ((type = get_token(&str, token, last_type)) != TOKEN_END) {
        if (type == TOKEN_OPERATOR) {
            bool is_valid = false;
            for (int i = 0; i < VALID_TOKENS_SIZE; i++) {
                if (strcmp(token, valid_tokens[i]) == 0) is_valid = true;
            }
            if (!is_valid) status = ERROR;
        }
        if (strcmp(token, "(") == 0)
            brackets_count++;
        else if (strcmp(token, ")") == 0)
            brackets_count--;
        if (brackets_count < 0) status = ERROR;
        last_type = type;
    }
    return status;
}

TokenType get_token(char **str, char token[], TokenType last_type) {
    int i = 0;
    while ((token[i] = getch(str)) == ' ' || token[i] == '\t');
    token[1] = '\0';
    if (token[i] == '\0')
        return TOKEN_END;
    else if (isdigit(token[i])) {
        while (isdigit(token[++i] = getch(str)));
        if (token[i] == '.')
            while (isdigit(token[++i] = getch(str)));
        token[i] = '\0';
        ungetch(str);
        return TOKEN_NUMBER;
    } else if (token[i] == '-' && last_type != TOKEN_NUMBER) {
        token[0] = 'u';
        return TOKEN_OPERATOR;
    } else if (token[i] == '(') {
        return TOKEN_LBRACKET;
    } else if (token[i] == ')') {
        return TOKEN_RBRACKET;
    } else if (isalpha(token[i])) {
        while (isalpha(token[++i] = getch(str)));
        token[i] = '\0';
        ungetch(str);
        return TOKEN_FUNCTION;
    } else {
        token[++i] = '\0';
        return TOKEN_OPERATOR;
    }
}

int get_priority(const char token[]) {
    if (strcmp(token, "+") == 0 || strcmp(token, "-") == 0)
        return 1;
    else if (strcmp(token, "*") == 0 || strcmp(token, "/") == 0)
        return 2;
    else if (strcmp(token, "tan") == 0 || strcmp(token, "ctg") == 0 ||
             strcmp(token, "sin") == 0 || strcmp(token, "cos") == 0 ||
             strcmp(token, "ln") == 0 || strcmp(token, "sqrt") == 0)
        return 3;
    else if (strcmp(token, "u") == 0)
        return 4;
    else
        return 0;
}

void push(Stack *stack_p, const char token[]) {
    Node *tmp_node = stack_p->top;
    stack_p->top = (Node *)malloc(sizeof(Node));
    stack_p->top->next = tmp_node;
    memcpy(stack_p->top->token, token, MAX_TOKEN_SIZE);
}

Status pop(Stack *stack_p, char token[]) {
    if (is_empty(stack_p)) return ERROR;
    memcpy(token, stack_p->top->token, MAX_TOKEN_SIZE);
    Node *tmp = stack_p->top;
    stack_p->top = stack_p->top->next;
    free(tmp);
    return SUCCESS;
}

Status peek(const Stack *stack_p, char token[]) {
    if (is_empty(stack_p)) return ERROR;
    memcpy(token, stack_p->top->token, MAX_TOKEN_SIZE);
    return SUCCESS;
}

void clean(Stack *stack_p) {
    while (stack_p->top != NULL) {
        Node *tmp_p = stack_p->top;
        stack_p->top = stack_p->top->next;
        free(tmp_p);
    }
}

int is_empty(const Stack *stack_p) { return (stack_p->top == NULL); }

char getch(char **str) { return *(*str)++; }

void ungetch(char **str) { (*str)--; }
