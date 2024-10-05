#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum{
    INSERT,
    FIND,
    SELECT,
    UPDATE,
    DELETE
}statement_type;

typedef enum{
    PARSE_SUCCESSFUL,
    NEGATIVE_ID_ERROR,
    INVALID_INPUT,
    UNRECOGNIZED,
}parse_res;

typedef struct{
    char type[100];
    int id;
    char username[255];
    char password[255];
}tokens;

typedef struct{
    int id;
    char username[255];
    char password[255];
} row;

typedef struct{
    statement_type type;
    row row_to_insert;
}statement;

void print_prompt(){
    printf("table > ");
}

tokens* tokenize(char* input){
    char* type = strtok(input, " ");
    char* id_str = strtok(NULL, " ");
    char* username = strtok(NULL, " ");
    char* password = strtok(NULL, " ");

    tokens* token = (tokens*)malloc(sizeof(tokens));
    if (type != NULL) strcpy(token->type, type);
    token->id = id_str != NULL ? atoi(id_str) : -1;
    username != NULL ? strcpy(token->username, username) : strcpy(token->username, "");
    password != NULL ? strcpy(token->password, password):  strcpy(token->password, "") ;

    return token;
}

void print_tokens(tokens* token){
    printf("tokens\n%s %d %s %s\n", token->type, token->id, token->username, token->password);
}

parse_res parse(tokens* token, statement** s){
    char* type = token->type;
    if(strcmp(type, "insert")==0){
        (*s)->type = INSERT;
        if(token->id < 0){
            // error
            return NEGATIVE_ID_ERROR;
        }
        if(strlen(token->username)==0){
            // error
            return INVALID_INPUT;
        }
        if(strlen(token->password)==0){
            // error
            return INVALID_INPUT;
        }
    }else if(strcmp(type, "select")==0){
        (*s)->type = SELECT;
        if(token->id!=-1){
            // error
            return INVALID_INPUT;
        }
        if(strlen(token->username)>0){
            // error
            return INVALID_INPUT;
        }
        if(strlen(token->password)>0){
            // error
            return INVALID_INPUT;
        }
    }else if(strcmp(type, "update")==0){
        (*s)->type = UPDATE;
        if(token->id<0){
            // error
            return NEGATIVE_ID_ERROR;
        }
        if(strlen(token->username)==0){
            // error
            return INVALID_INPUT;
        }
        if(strlen(token->password)==0){
            // error
            return INVALID_INPUT;
        }
    }else if(strcmp(type, "delete")==0){
        (*s)->type = DELETE;
        if(token->id<0){
            // error
            return NEGATIVE_ID_ERROR;
        }
        if(strlen(token->username)!=0){
            // error
            return INVALID_INPUT;
        }
        if(strlen(token->password)!=0){
            // error
            return INVALID_INPUT;
        }
    }else if(strcmp(type, "find")==0){
        (*s)->type = FIND;
        if(token->id<0){
            // error
            return NEGATIVE_ID_ERROR;
        }
        if(strlen(token->username)!=0){
            // error
            return INVALID_INPUT;
        }
        if(strlen(token->password)!=0){
            // error
            return INVALID_INPUT;
        }
    }else{
        return UNRECOGNIZED;
    }

    (*s)->row_to_insert.id = token->id;
    strcpy((*s)->row_to_insert.username, token->username);
    strcpy((*s)->row_to_insert.password, token->password);

    return PARSE_SUCCESSFUL;
}

void execute(statement* s){
    printf("parse statement is type: %d\nID:%d\nUsername:%s\nPassword:%s\n", s->type, s->row_to_insert.id, s->row_to_insert.username, s->row_to_insert.password);
    return;
}

int main() {
    char input[100];
    while (true) {
        print_prompt();
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;  

        tokens* tokenizedStatement = tokenize(input);
        statement* s;
        s = (statement*)malloc(sizeof(statement));

        switch(parse(tokenizedStatement, &s)){
            case PARSE_SUCCESSFUL:
                execute(s);
                break;
            case INVALID_INPUT:
                printf("Input is invalid\n");
                break;
            case NEGATIVE_ID_ERROR:
                printf("Input ID cannot be negative\n");
                break;
            case UNRECOGNIZED:
                printf("Unrecognized query\n");
                break;
        }

        free(tokenizedStatement);
        free(s);
    }
}
