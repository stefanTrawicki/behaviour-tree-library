#if !defined(TREE_H)
#define TREE_H

/* --------------------------------- Imports -------------------------------- */

#include <libpointerlist.h>
#include <libpointerstack.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* --------------------------------- Callers -------------------------------- */

#define FAIL(NODE) ({NODE->state = FAIL; NODE->state_vtable->fail(NODE); return; })
#define SUCCEED(NODE) ({NODE->state = SUCCESS; NODE->state_vtable->succeed(NODE); return; })
#define RUN(NODE) ({NODE->state_vtable->run(NODE); return; })

#define START(NODE) ( {SET_FLAG(NODE->flags, IS_RUNNING);} )
#define FINISH(NODE) ( {SET_FLAG(NODE->flags, IS_FINISHED); CLEAR_FLAG(NODE->flags, IS_RUNNING);} )

/* ---------------------------------- Flags --------------------------------- */

#define CHECK_FLAG(FLAG, FIELD) ({ FLAG >> FIELD & 0x1; })
#define SET_FLAG(FLAG, FIELD) ({ FLAG = FLAG | ((uint8_t)0x01 << FIELD); })
#define CLEAR_FLAG(FLAG, FIELD) ({ FLAG = FLAG & ~((uint8_t)0x01 << FIELD); })

/* General Flags */
#define IS_INITIALISED 0x01
#define IS_RUNNING 0x02
#define IS_FINISHED 0x03

/* Node Flags */
#define IS_COMPOSITE 0x04
#define IS_DECORATOR 0x05
#define IS_LABELLED 0x06
#define IS_IN_TREE 0x07

/* Tree Flags */
#define IS_HALTED 0x04
#define IS_ROOT_SET 0x05
#define IS_BLACKBOARD_SET 0x06

/* --------------------------------- Logging -------------------------------- */

#define CLEAR_LOG() ({if(log_path){FILE *f = fopen(log_path, "w"); fprintf(f, ""); fclose(f);} })
#define LOG(...) ({if(log_path){const char* const_log_path = (const char*)log_path; FILE *f = fopen(const_log_path, "a"); fprintf(f, __VA_ARGS__); fprintf(f, "\n"); fclose(f);} })
#define LABEL_LOG(NODE, ...) ({if(log_path){const char* const_log_path = (const char*)log_path; FILE *f = fopen(const_log_path, "a"); if(CHECK_FLAG(NODE->flags, IS_LABELLED))fprintf(f, "[%s] ", NODE->label); fprintf(f, __VA_ARGS__); fprintf(f, "\n"); fclose(f);} })
#define TYPE_LABEL ((char const*[]){ "entry", "sequence", "fallback", "inverter", "leaf", "repeater"})

/* ------------------------------- Assertions ------------------------------- */

#define ASSERT_MSG(COND, MSG) ({if(COND){printf(MSG "\n"); exit(EXIT_FAILURE);} })
#define ASSERT(COND) ({if(COND){exit(EXIT_FAILURE);} })

/* --------------------------------- VTables -------------------------------- */

typedef struct Node Node_t;
typedef void (*Function_t)(Node_t *node);

typedef struct ActionVtable
{
    Function_t std_start;
    Function_t start;
    Function_t tick;
    Function_t stop;
    Function_t std_stop;
    Function_t evaluation;
} ActionVtable_t;

typedef struct StateVtable
{
    Function_t succeed;
    Function_t fail;
    Function_t run;
} StateVtable_t;

/* ------------------------------ Enumerations ------------------------------ */

typedef enum
{
    SUCCESS,
    FAIL,
    UNDETERMINED
} _NodeState_e;

typedef enum
{
    ENTRY,
    SEQUENCE,
    FALLBACK,
    INVERTER,
    LEAF,
    REPEATER,
    _COUNT
} NodeType_e;

/* --------------------------------- Logging -------------------------------- */

char *log_path;
void log_set_path(char *path);
void _print_flags(uint8_t flags);

/* ---------------------------------- Node ---------------------------------- */

typedef struct BTree BTree_t;

typedef struct Node
{
    NodeType_e type;
    _NodeState_e state;

    ActionVtable_t *action_vtable;
    StateVtable_t *state_vtable;

    uint8_t flags;
    void *subject;

    BTree_t *tree;
    Node_t *parent;
    void_list_t *children;

    int32_t start_loops;
    int32_t left_loops;

    char *label;
} Node_t;

void node_create(Node_t *node, NodeType_e t);
void node_set_label(Node_t *node, const char *label);
void node_set_actions(Node_t *node, ActionVtable_t *actions);
void node_set_subject(Node_t *node, void *subject);
void node_add_child(Node_t *node, Node_t *child);
void node_set_repetitions(Node_t *node, int32_t repetitions);
void node_reset(Node_t *node);
void node_set_tree(Node_t *node, BTree_t *tree);
void *node_get_blackboard(Node_t *node);

/* ----------------------------- Behaviour tree ----------------------------- */

typedef struct BTree
{
    uint8_t flags;

    void_list_t *nodes;

    Node_t *root_node;
    Node_t *current_node;

    void *blackboard;

    ActionVtable_t std_action_vtables[_COUNT];
    StateVtable_t *std_state_vtables;
} BTree_t;

void b_tree_create(BTree_t *tree);
uint32_t b_tree_run(BTree_t *tree);
void b_tree_set_root(BTree_t *tree, Node_t *node);
void b_tree_move(BTree_t *tree, Node_t *node);
void b_tree_reset(BTree_t *tree);
void b_tree_set_blackboard(BTree_t *tree, void *blackboard);

/* --------------------------- Standard functions --------------------------- */

void std_success(Node_t *node);
void std_failure(Node_t *node);
void std_running(Node_t *node);
void std_start(Node_t *node);
void std_stop(Node_t *node);
void std_decorator_handler(Node_t *node);
void std_composite_handler(Node_t *node);

#endif // TREE_H