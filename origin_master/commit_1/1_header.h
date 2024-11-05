#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_FILES 100

typedef struct File {
    char filename[100];
} File;

typedef struct Commit {
    int id;
    char message[256];
    time_t timestamp;
    File files[MAX_FILES];
    int file_count;
    struct Commit* next;
    struct Commit* prev; 
} Commit;

typedef struct Repository {
    Commit* head;
    Commit* tail;
    File staging_area[MAX_FILES];
    int staged_count;
} Repository;

typedef struct UndoStack {
    Commit* commits[MAX_FILES];
    int top;
} UndoStack;

void init_repository(Repository* repo);
void init_undo_stack(UndoStack* stack);
void push_undo_stack(UndoStack* stack, Commit* commit);
Commit* pop_undo_stack(UndoStack* stack);
void create_new_file(const char* filename);
void add_file(Repository* repo, const char* filename);
void add_all_files(Repository* repo);
void commit(Repository* repo, const char* message, UndoStack* undo_stack);
void undo_commit(Repository* repo, UndoStack* undo_stack);
void show_status(Repository* repo);
void list_commits(Repository* repo);
void push_origin_master(Repository* repo);
void checkout_commit(Repository* repo, int commit_id);

#endif
