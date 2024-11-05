#include "header.h"
#include <string.h>

void execute_command(char* command, Repository* repo, UndoStack* undo_stack) {
    char *token = strtok(command, " ");
    if (token == NULL) return;

    if (strcmp(token, "create") == 0) {
        token = strtok(NULL, " ");
        if (token) {
            create_new_file(token);
        } else {
            printf("Usage: create <filename>\n");
        }
    } else if (strcmp(token, "add") == 0) {
        token = strtok(NULL, " ");
        if (token) {
            add_file(repo, token);
        } else {
            printf("Usage: add <filename>\n");
        }
    } else if (strcmp(token, "add_all") == 0) {
        add_all_files(repo);
    } else if (strcmp(token, "commit") == 0) {
        char* message = strtok(NULL, "\"");
        if (message) {
            commit(repo, message, undo_stack);
        } else {
            printf("Usage: commit \"<message>\"\n");
        }
    } else if (strcmp(token, "undo") == 0) {
        undo_commit(repo, undo_stack);
    } else if (strcmp(token, "status") == 0) {
        show_status(repo);
    } else if (strcmp(token, "list") == 0) {
        list_commits(repo);
    } else if (strcmp(token, "push") == 0) {
        push_origin_master(repo);
    } else if (strcmp(token, "checkout") == 0) {
        token = strtok(NULL, " ");
        if (token) {
            int commit_id = atoi(token);
            checkout_commit(repo, commit_id);
        } else {
            printf("Usage: checkout <commit_id>\n");
        }
    } else if (strcmp(token, "exit") == 0) {
        printf("Exiting...\n");
        exit(0);
    } else {
        printf("Unknown command: %s\n", token);
        printf("Available commands: create, add, add_all, commit, undo, status, list, push, checkout, exit\n");
    }
}

int main() {
    Repository repo;
    UndoStack undo_stack;
    init_repository(&repo);
    init_undo_stack(&undo_stack);

    char command[256];
    
    printf("Welcome to Mini-VCS. Type 'exit' to quit.\n");
    printf("Available commands: create <filename>, add <filename>, add_all, commit \"<message>\", undo, status, list, push, checkout <commit_id>\n");

    while (1) {
        printf("\n> ");
        fgets(command, sizeof(command), stdin);
        
        command[strcspn(command, "\n")] = '\0';  // Remove trailing newline
        execute_command(command, &repo, &undo_stack);
    }

    return 0;
}
