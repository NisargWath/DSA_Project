#include "header.h"

void init_repository(Repository* repo) {
    repo->head = NULL;
    repo->tail = NULL;
    repo->staged_count = 0;
    printf("Initialized an empty repository.\n");
}

void init_undo_stack(UndoStack* stack) {
    stack->top = -1;
}

void push_undo_stack(UndoStack* stack, Commit* commit) {
    if (stack->top < MAX_FILES - 1) {
        stack->commits[++(stack->top)] = commit;
    } else {
        printf("Undo stack overflow!\n");
    }
}

Commit* pop_undo_stack(UndoStack* stack) {
    if (stack->top >= 0) {
        return stack->commits[(stack->top)--];
    } else {
        printf("Undo stack is empty!\n");
        return NULL;
    }
}

void create_new_file(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Could not create file '%s'.\n", filename);
        return;
    }
    printf("Enter content for the file (Press Enter when done):\n");
    char content[1024];
    fgets(content, sizeof(content), stdin);  
    fprintf(file, "%s", content); 
    fclose(file);
    printf("File '%s' created successfully.\n", filename);
}

void add_file(Repository* repo, const char* filename) {
    for (int i = 0; i < repo->staged_count; i++) {
        if (strcmp(repo->staging_area[i].filename, filename) == 0) {
            printf("File '%s' is already in the staging area.\n", filename);
            return;
        }
    }
    if (repo->staged_count < MAX_FILES) {
        strcpy(repo->staging_area[repo->staged_count].filename, filename);
        repo->staged_count++;
        printf("File '%s' added to staging area.\n", filename);
    } else {
        printf("Staging area is full. Cannot add more files.\n");
    }
}

void add_all_files(Repository* repo) {
    struct dirent *entry;
    DIR *dir = opendir(".");
    if (dir == NULL) {
        printf("Could not open current directory.\n");
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && repo->staged_count < MAX_FILES) {
            add_file(repo, entry->d_name);
        }
    }
    closedir(dir);
}

void commit(Repository* repo, const char* message, UndoStack* undo_stack) {
    if (repo->staged_count == 0) {
        printf("No files to commit.\n");
        return;
    }
    Commit* new_commit = (Commit*)malloc(sizeof(Commit));
    new_commit->id = (repo->head == NULL) ? 1 : repo->tail->id + 1;
    strcpy(new_commit->message, message);
    new_commit->timestamp = time(NULL);
    new_commit->file_count = repo->staged_count;
    for (int i = 0; i < repo->staged_count; i++) {
        new_commit->files[i] = repo->staging_area[i];
        char commit_filename[200];
        sprintf(commit_filename, "commit_%d_%s", new_commit->id, new_commit->files[i].filename);
        FILE* src_file = fopen(new_commit->files[i].filename, "r");
        FILE* dest_file = fopen(commit_filename, "w");
        if (src_file == NULL || dest_file == NULL) {
            printf("Error: Cannot copy file '%s'.\n", new_commit->files[i].filename);
            if (src_file) fclose(src_file);
            if (dest_file) fclose(dest_file);
            return;
        }
        char ch;
        while ((ch = fgetc(src_file)) != EOF) {
            fputc(ch, dest_file);
        }
        fclose(src_file);
        fclose(dest_file);
        printf("File '%s' committed as '%s'.\n", new_commit->files[i].filename, commit_filename);
    }
    repo->staged_count = 0;
    new_commit->next = NULL;
    new_commit->prev = repo->tail;
    if (repo->tail) {
        repo->tail->next = new_commit;
    }
    repo->tail = new_commit;
    if (repo->head == NULL) {
        repo->head = new_commit;
    }
    push_undo_stack(undo_stack, new_commit);
    printf("Committed with message: '%s'\n", message);
}

void undo_commit(Repository* repo, UndoStack* undo_stack) {
    Commit* commit = pop_undo_stack(undo_stack);
    if (commit == NULL) {
        printf("No commits to undo.\n");
        return;
    }
    if (commit->prev) {
        commit->prev->next = NULL;
        repo->tail = commit->prev;
    } else {
        repo->head = NULL;
        repo->tail = NULL;
    }
    free(commit);
    printf("Last commit has been undone.\n");
}

void show_status(Repository* repo) {
    if (repo->staged_count == 0) {
        printf("No files in staging area.\n");
    } else {
        printf("Staged files:\n");
        for (int i = 0; i < repo->staged_count; i++) {
            printf("  - %s\n", repo->staging_area[i].filename);
        }
    }
    printf("\nCommit log:\n");
    Commit* current = repo->head;
    if (current == NULL) {
        printf("No commits yet.\n");
    }
    while (current) {
        printf("Commit %d: %s\n", current->id, current->message);
        current = current->next;
    }
}

void list_commits(Repository* repo) {
    Commit* current = repo->head;
    if (!current) {
        printf("No commits yet.\n");
        return;
    }
    while (current) {
        printf("Commit %d: %s (Time: %s)\n", current->id, current->message, ctime(&current->timestamp));
        current = current->next;
    }
}

void push_origin_master(Repository* repo) {
    struct stat st = {0};
    if (stat("origin_master", &st) == -1) {
        mkdir("origin_master", 0700);
    }
    Commit* current = repo->head;
    while (current) {
        char commit_dir[100];
        sprintf(commit_dir, "origin_master/commit_%d", current->id);
        mkdir(commit_dir, 0700);
        for (int i = 0; i < current->file_count; i++) {
            char commit_filename[200], destination[300];
            sprintf(commit_filename, "commit_%d_%s", current->id, current->files[i].filename);
            sprintf(destination, "%s/%s", commit_dir, commit_filename);
            rename(commit_filename, destination);
        }
        current = current->next;
    }
    printf("Pushed all commits to origin_master.\n");
}

void checkout_commit(Repository* repo, int commit_id) {
    Commit* current = repo->head;
    while (current && current->id != commit_id) {
        current = current->next;
    }
    if (current == NULL) {
        printf("Commit ID %d not found.\n", commit_id);
        return;
    }
    printf("Switched to commit %d: %s\n", current->id, current->message);
    for (int i = 0; i < current->file_count; i++) {
        printf("File: %s\n", current->files[i].filename);
    }
}
