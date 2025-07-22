#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_SIZE 10
#define MAX_BUFFER_SIZE 100
#define VALID_TOKENS \
    { "+", "-", "*", "/", "(", ")" }

typedef enum { TOKEN_NUMBER, TOKEN_OPERATOR, TOKEN_END } TokenType;

typedef enum { SUCCESS, ERROR } Status;

typedef struct Node {
    char token[MAX_TOKEN_SIZE];
    struct Node *next;
} Node;

typedef struct {
    Node *top;
} Stack;

void tokenization(char str[]);
Status infix_to_postfix(char str[]);
Status calc_rpn(char str[], double *result);
Status validate(char str[]);
bool valid(char token[]);
int get_priority(const char token[]);
TokenType get_token(char **str, char token[]);
char getch(char **str);
void ungetch(char **str);
int is_empty(const Stack *stack_p);
void push(Stack *stack_p, const char token[]);
Status pop(Stack *stack_p, char token[]);
Status peek(Stack *stack_p, char token[]);
void clean(Stack *stack_p);

int main() {
    while (true) {
        char str[MAX_BUFFER_SIZE];
        int index = 0;
        printf("Exit(q)\nExpression: ");
        while ((str[index++] = getchar()) != '\n');
        if (index == 2 && str[0] == 'q') break;
        str[index] = '\0';
        tokenization(str);
        double result;
        if (validate(str) == SUCCESS && infix_to_postfix(str) == SUCCESS &&
            calc_rpn(str, &result) == SUCCESS)
            printf("Result: %lf\n", result);
        else
            printf("n/a\n");
    }

    return 0;
}

void tokenization(char str[]) {
    char buffer[MAX_BUFFER_SIZE];
    char token[MAX_TOKEN_SIZE];
    char *tmp = str;
    int index = 0;
    *strchr(str, '\n') = '\0';
    while (get_token(&tmp, token) != TOKEN_END) {
        int i = 0;
        while (token[i] != '\0') buffer[index++] = token[i++];
        buffer[index++] = ' ';
    }
    buffer[--index] = '\0';
    memcpy(str, buffer, MAX_BUFFER_SIZE);
}

Status infix_to_postfix(char str[]) {
    TokenType type;
    TokenType last_type = TOKEN_END;
    char buffer[MAX_BUFFER_SIZE];
    char token[MAX_TOKEN_SIZE], tmp_token[MAX_TOKEN_SIZE];
    char *tmp = str;
    int index = 0;
    Stack stack;
    stack.top = NULL;
    Status status = SUCCESS;
    while ((type = get_token(&tmp, token)) != TOKEN_END) {
        if (type == TOKEN_NUMBER) {
            int i = 0;
            while (token[i] != '\0') buffer[index++] = token[i++];
            buffer[index++] = ' ';
        } else if (strcmp(token, "(") == 0) {
            push(&stack, token);
        } else if (strcmp(token, ")") == 0) {
            if (pop(&stack, tmp_token) == ERROR) status = ERROR;
            while (!is_empty(&stack) && strcmp(tmp_token, "(") != 0) {
                int i = 0;
                while (tmp_token[i] != '\0') buffer[index++] = tmp_token[i++];
                buffer[index++] = ' ';
                if (pop(&stack, tmp_token) == ERROR) status = ERROR;
            }
        } else {
            if (!is_empty) peek(&stack, tmp_token);
            while (!is_empty(&stack) &&
                   get_priority(tmp_token) >= get_priority(token)) {
                if (pop(&stack, tmp_token) == ERROR) status = ERROR;
                int i = 0;
                while (tmp_token[i] != '\0') buffer[index++] = tmp_token[i];
                buffer[index++] = ' ';
            }
            push(&stack, token);
        }
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
    TokenType type;
    char token[MAX_TOKEN_SIZE];
    char tmp_token1[MAX_TOKEN_SIZE], tmp_token2[MAX_TOKEN_SIZE];
    char tmp[MAX_TOKEN_SIZE];
    Stack stack;
    stack.top = NULL;
    Status status = SUCCESS;

    while ((type = get_token(&str, token)) != TOKEN_END) {
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
        } else
            status = ERROR;
    }
    pop(&stack, tmp_token1);
    clean(&stack);
    *result = atof(tmp_token1);
    return status;
}

Status validate(char str[]) {
    TokenType type;
    int brackets_count = 0;
    char token[MAX_TOKEN_SIZE];
    char valid_tokens[][MAX_TOKEN_SIZE] = VALID_TOKENS;
    Status status = SUCCESS;
    while ((type = get_token(&str, token)) != TOKEN_END) {
        if (type == TOKEN_OPERATOR) {
            bool is_valid = false;
            for (int i = 0;
                 i < sizeof(valid_tokens) / (sizeof(char) * MAX_TOKEN_SIZE);
                 i++) {
                if (strcmp(token, valid_tokens[i]) == 0) is_valid = true;
            }
            if (!is_valid) status = ERROR;
        }
        if (strcmp(token, "(") == 0)
            brackets_count++;
        else if (strcmp(token, ")") == 0)
            brackets_count--;
        if (brackets_count < 0) status = ERROR;
    }
    return status;
}

bool valid(char token[]) {
    int tokens_count = 4;
    char suitable_tokens[][MAX_TOKEN_SIZE] = {"+", "-", "*", "/"};
    for (int i = 0; i < tokens_count; i++)
        if (strcmp(token, suitable_tokens[i]) == 0) return true;
    return false;
}

int get_priority(const char token[]) {
    if (strcmp(token, "+") == 0 || strcmp(token, "-") == 0)
        return 1;
    else if (strcmp(token, "*") == 0 || strcmp(token, "/") == 0)
        return 2;
    else
        return 0;
}

TokenType get_token(char **str, char token[]) {
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
    } else {
        while (isalpha(token[++i] = getch(str)));
        token[i] = '\0';
        ungetch(str);
        return TOKEN_OPERATOR;
    }
}

char getch(char **str) { return *(*str)++; }

void ungetch(char **str) { (*str)--; }

int is_empty(const Stack *stack_p) { return (stack_p->top == NULL); }

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

Status peak(Stack *stack_p, char token[]) {
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