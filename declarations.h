// DECLARATIONS ------------------------------------------

typedef struct Good
{
  char *name;
  char *descr; 
  int price; 
  char row;
  int column;
  int amount;
}Good;

typedef struct DB{
  Good g;
  struct DB *next;
}DB_t;

typedef struct action
{
  int type; // NOTHING = 0, ADD = 1, REMOVE = 2, EDIT = 3
  int index; // index for listing goods etc. in DB
  int s_index; // used for storing index when edit/ remove, even thou user may use list.
  Good copy; // copy of good 
}action_t;

//FRAMEWORK
void print_main_menu(void);
void add_action(DB_t *d, action_t *a);
void remove_action(DB_t *d, action_t *a);
void edit_action(DB_t *d, action_t *a);
int edit_good(Good *g);
int listing_action(DB_t *d, int page, action_t *a);
void undo_action(DB_t *d, action_t *a);
bool quit_action(void);
void list_db(DB_t *p, int n);
void print_good(Good g);


//INPUT
char *get_string_input(void);
int get_int_input(void);
char get_char_input(void);
char ask_question_char(char *c, char *d);


//DB
void add_to_db(DB_t *head, Good g);
int len_db(DB_t *head);
Good get_good_db_n(DB_t *h, int n);
void replace_n_db(DB_t *h, Good g, int i);
void remove_n_from_db(DB_t *h, action_t *a);
void insert_at_n(DB_t *h, Good g, int i);


// MISC
void create_init_db(DB_t *head);
void clear_screen(void);
int row_to_int(char c);
int invalid_col(int n);
void free_memory(DB_t *db, action_t *a);
