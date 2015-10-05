// UPPSALA UNIVERSITY
// AUTHOR: ERIC FALHEIM
// PROJECT: LAGERHANTERING 1.0

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include "declarations.h"
#define Clear_stdin while (getchar() != '\n');

//Globals
int is_shelf_occupied[26][100];
char buffer[256];

// MAIN LOOP ----------------------------------------------------

int main(int argc, char *argv[])
{
  bool should_quit = false;
  
  for(int i = 0; i < 26; i++)
    {
      for(int j = 0; j < 100; j++)
	{
	  is_shelf_occupied[i][j] = 0;
	}
    }
  
  Good nulgood;
  nulgood.name = "Nulgood";
  nulgood.descr = "Need to make so DB can be NULL";
  nulgood.price = 0;
  nulgood.row = '0';
  nulgood.column = 0;
  nulgood.amount = 0;
  
  DB_t *db = malloc(sizeof(DB_t)); // pga problem med head i linked list
  db->g = nulgood;
  db->next = NULL;
  
  action_t *action = malloc(sizeof(action_t));
  action->type = 0;
  
  create_init_db(db); //Add some goods for testing

  clear_screen();
  
  while (!should_quit)
    {
      print_main_menu();
      
      char input = ask_question_char("Choose from options: ", "AaRrEeLlUuQq");
      
      switch(toupper(input))
	{
	case 'A':
	  clear_screen();
	  add_action(db, action);
	  break;
	  
	case 'R':
	  clear_screen();
	  remove_action(db, action);
	  break;
	  
	case 'E':
	  clear_screen();
	  edit_action(db, action);
	  break;
	  
	case 'L':
	  clear_screen();
	  listing_action(db, 0, action);
	  break;
	  
	case 'U':
	  clear_screen();
	  undo_action(db, action);
	  break;
	  
	case 'Q':
	  clear_screen();
	  should_quit = quit_action();
	  break;
	  
	default:
	  puts("Something went wrong. err: default case in main");
	}
    }

  free_memory(db, action); // skaffa valgrind för testning
  
  return 0;
}

// FUNCTIONS -----------------------------------------------------------
// FRAMEWORK
void add_action(DB_t *db, action_t *action)
{
  Good g;
  
  printf("\nName: ");
  g.name = get_string_input();
  
  printf("Description: ");
  g.descr = get_string_input();
  
  printf("Price: ");
  g.price = get_int_input();
  
  printf("Row [A-Z]: ");
  g.row = get_char_input();

  int row = row_to_int(g.row);
  int col = 0;
  do{
    printf("Column [1-100]: ");
    col = get_int_input();
  } while(is_shelf_occupied[row][col] || invalid_col(col));
  
  g.column = col;
  
  printf("Amount: ");
  g.amount = get_int_input();
  puts("");
  
  print_good(g);

  char ans;
  
  do
    {
      ans = ask_question_char("Do you want to add this good? [Y]es / [N]o / [E]dit","YyNnEe");
      
      if (toupper(ans) == 'E'){
	edit_good(&g);
      }
    } while(toupper(ans) != 'Y' && toupper(ans) != 'N');
  
  if (toupper(ans) == 'Y'){
    add_to_db(db, g);

    is_shelf_occupied[row_to_int(g.row)][g.column] = 1;
    
    action->s_index = len_db(db);
    action->type = 1;
    action->copy = g;
    
  }
  clear_screen();
}



int listing_action(DB_t *db, int page, action_t *action)
{
  int len = len_db(db);
  int is_next = len - (20*(page+1));
  int entries_last_page = len % 20;
  char ans = 0;
  int entry = 0;
  
  list_db(db, page);
  
  if(len == 0)
    {
      puts("\nDB is empty. Please add more goods.\n\n");
      return -1; //cancel;
    }
  
  if(is_next <= 0)
    {
      ans = ask_question_char("\nChoose [G]ood / [C]ancel", "GgCc");
    }
  
  else
    {
      ans =  ask_question_char("\nChoose [G]ood / [C]ancel / [N]ext page ", "GgCcNn");
      
      if (toupper(ans) == 'N')
	{
	  listing_action(db, page+1, action);
	  return 0;
	}
    }
  
  if (toupper(ans) == 'G')
    {
      if(is_next <= 0)
	{
	  do
	    {
	      printf("\nChoose good by number: ");
	      entry = get_int_input();
	    }while(entry>entries_last_page);
	}
      
      else
	{
	  do
	    {
	      printf("\nChoose good by number: ");
	      entry = get_int_input();
	    }while(entry>20);
	}
      
      int index = entry + 20*page;
      
      clear_screen();
      
      Good good = get_good_db_n(db, index);
      
      print_good(good);
    
      action->index = index;
      
      return 0;
    }
  
  else
    {
      clear_screen();
      return -1; //cancel
    }
}



void remove_action(DB_t *db, action_t *action)
{
  listing_action(db, 0, action);
  remove_n_from_db(db, action);
}



void edit_action(DB_t *db, action_t *action)
{
  int is_cancel = listing_action(db, 0, action);
  
  if(is_cancel == 0)
    {  
      char ans = ask_question_char("Do you want do edit this good? [Y]es / [N]o ", "YyNn");
      
      if(toupper(ans) == 'Y')
	{
	  Good edit_g = get_good_db_n(db, action->index);
	  Good original = edit_g;
	  int was_edited = edit_good(&edit_g);
	  
	  if (was_edited)
	    {
	      action->copy = original;
	      action->s_index = action->index;
	      action->type = 3;
	      replace_n_db(db, edit_g, action->index);
	    }
	}
    }
}



// stdbool.h
int edit_good(Good *g) // return 1 if edited. 0 if Quit.
{
  char ans = ask_question_char("\nWhat do you want to edit?\n[N]ame\n[D]escription\n[P]rice\n[R]ow\n[C]olumn\n[A]mount\n\n[Q]uit or choose from options", "NnDdPpRrCcAaQq");

  switch(toupper(ans))
    {
    case 'N':
      printf("\nCurrent name: %s\nn", g->name);
      puts("-------------------");
      printf("New input: ");
      g->name = "edited name";
      break;
      
    case 'D':
      printf("\nCurrent description: %s\n", g->descr);
      puts("-------------------");
      printf("New input: ");
      g->descr = "edited description";
      break;
      
    case 'P':
      printf("\nCurrent price: %d\n", g->price);
      puts("-------------------");
      printf("New input: ");
      g->price = get_int_input();
      break;
      
    case 'R':
      printf("\nCurrent row: %c\n", g->row);
      puts("-------------------");

      int orig_row = row_to_int(g->row);
      int row = 0;
      int col = g->column;
      char c;
      
      do
	{
	  printf("New input: ");
	  
	  c = get_char_input();
	  row = row_to_int(c);
	  
	  Clear_stdin;
	} while(is_shelf_occupied[row][col]);
      
      g->row = c;
      
      is_shelf_occupied[orig_row][col] = 0;
      is_shelf_occupied[row][col] = 1;

      break;
      
    case 'C':
      printf("\nCurrent column: %d\n", g->column);
      puts("-------------------");

      row = row_to_int(g->row);
      int orig_col = g->column;
      col = 0;
      
      do
	{
	  printf("New input: ");
	  col = get_int_input();	
	} while(is_shelf_occupied[row][col] || invalid_col(col));
      
      Clear_stdin;
      
      g->column = col;

      is_shelf_occupied[row][orig_col] = 0;
      is_shelf_occupied[row][col] = 1;
      
      break;
      
    case 'A':
      printf("\nCurrent amount: %d\n", g->amount);
      puts("-------------------");
      printf("New input: ");
      g->amount = get_int_input();
      break;
      
    case 'Q':
      return 0;
      
    default:
      puts("something happened in edit_good");
    }
  return 1;
} 



void undo_action(DB_t *db, action_t *action)
{
  switch(action->type)
    {
    case 0:
      puts("No action to undo.");
      break;
      
    case 1: // Add
      puts("Removed last added item.");
      action->index = action->s_index;
      remove_n_from_db(db, action);
      action->type = 0;
      break;
      
    case 2: // Remove
      puts("Added last removed good.");
      insert_at_n(db, action->copy, action->s_index);
      action->type = 0;
      break;
      
    case 3: // Edit
      puts("Edited item set to original.");
      replace_n_db(db, action->copy, action->s_index);
      action->type = 0;
      break;
      
    default:
      break;
    }
}



bool quit_action(void)
{
  while(true)
    {
      char ans = ask_question_char("Do you really want to quit? [Y]es / [N]o", "YyNn");
      if (toupper(ans) == 'Y') return true;
      if (toupper(ans) == 'N') return false;
    }
  
  clear_screen();
  return false;
}



void print_main_menu(void)
{
  puts("-------------------------------------------");
  puts("Main menu:");
  puts("-------------------------------------------\n");
  puts("[A]dd good");
  puts("[R]emove good");
  puts("[E]dit good");
  puts("[L]ist goods");
  puts("[U]ndo");
  puts("[Q]uit\n");
}



void print_good(Good g)
{
  puts("------------------------------------");
  printf("Name: %s\n", g.name);
  printf("Description: %s\n", g.descr);
  printf("Price: %d\n", g.price);
  printf("Located at: %c%d\n", g.row, g.column);
  printf("Amount: %d\n", g.amount);
  puts("------------------------------------\n");
}


// INPUT
char *get_string_input(void)
{
  
  char *dest = malloc(sizeof(buffer));
  char c;
  int i = 0;
  
  do
    {
      c = getchar();
      
      if (c=='\n'){break;}
      
      buffer[i] = c;
      i++;
      
    }while(i < 255);
  
  i++;
  buffer[i] = '\0';

  strncpy(dest, buffer, 256);
  
  return dest;
}



char get_char_input(void)
{
  char input = 0;
  char *alt = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  do
    {
      if (input)
	{
	  printf("Bad input '%c', enter valid character.\n", input);
	  Clear_stdin;
	}
      input = toupper(getchar());

    } while(strchr(alt, input) == NULL);
  
  return input;
}



int get_int_input(void) // TODO: handle wrong type of input
{
  int input = 0;
  int count = 0;
  
  count = scanf("%d", &input);
  
  while (!count || input <= 0)
    {
      puts("Please enter a number greater than zero: ");
      Clear_stdin;
      count = scanf("%d", &input);
    }
  
  Clear_stdin;
  
  return input;
}



char ask_question_char(char *q, char *alt)
{
  char input = 0;
  
  printf("%s\n", q);
  
  do
    {
      if (input)
	{
	  printf("Bad input '%c', try again [%s]\n", input, alt);
	  Clear_stdin;
	}
      input = getchar();
      
    }while (strchr(alt, input) == NULL);
  
  Clear_stdin;
  return input;
}



// DB
void add_to_db(DB_t *head, Good g) // TODO: FIXA SÅ första elementet kan erasättas 
{
  DB_t *current = head;
  
  if(current != NULL)
    {
      while(current->next != NULL)
	{
	  current = current->next;
	}
      
      current->next = malloc(sizeof(DB_t));
      current->next->g = g;
      current->next->next = NULL;
    }
  
  else
    {
      head = malloc(sizeof(DB_t));
      head->g = g;
      head->next = NULL;
    }
}



int len_db(DB_t *head)
{
  DB_t *current = head;
  int i = 1;
  
  if(current == NULL)
    {
      return 0;
    }
  
  else
    {
      while(current->next != NULL)
	{
	  current = current->next;
	  i++;
	}
      
      return i;
    }
}



void list_db(DB_t *head, int page)
{
  int i = 1;
  int g_per_page = 20;
  int g_skip = g_per_page*page;
  DB_t *current = head;
  
  printf("\nPAGE %d\n--------\n", page+1);
  
  if(page>0)
    {
      for(int i = 0; i < g_skip; ++i)
	{
	  current = current->next;
	}
    }
  
  while((current != NULL) && (i <= 20))
    {
      printf("%d. %s\n", (i), current->g.name);
      current = current->next;
      i++;
    }
}



Good get_good_db_n(DB_t *head, int n)
{
  DB_t *current = head;
  
  for(int i = 1; i < n; ++i)
    {
      if(current != NULL)
	{
	  current = current->next;
	}
    }
  return current->g;
}



void replace_n_db(DB_t *head, Good g, int index)
{
  DB_t *current = head;
  
  if(index == 1)
    {
      current->g = g;
    }
  else
    {  
      for(int i = 1; i < index; i++)
	{
	  current = current->next; 
	}
      
      current->g = g;
    }
}



void remove_n_from_db(DB_t *head, action_t *action)
{
  DB_t *current = head;
  
  action->type = 2;
  action->s_index = action->index;
  
  if(action->index == 1) // problem med head
    { 
      action->copy = head->g;
      head = current->next;
      free(current);
    }
  
  else
    {
      for(int i = 1; i < (action->index)-1; i++)
	{
	  current = current->next;
	}
      
      DB_t *tail = current->next;
      
      current->next = tail->next;
      action->copy = tail->g;
      
      is_shelf_occupied[row_to_int(tail->g.row)][tail->g.column] = 0;
      free(tail);
    } 
}

// MISC.

void insert_at_n(DB_t *head, Good g, int index)
{
  DB_t *current = head;
  DB_t *new = malloc(sizeof(DB_t));
  
  if(index == 1)
    {
      new->g = g;
      new->next = current;
    }
  
  else
    {
      for(int i = 1; i < index-1; i++)
	{
	  current = current->next;
	}
      new->next = current->next;
      current->next = new;
    }
}



void free_memory(DB_t *head, action_t *action)
{
  DB_t *current = head;
  DB_t *temp;
  
  free(action); // räcker detta ? måste jag gå in på alla element?
  
  while(current != NULL) 
    {
      temp = current;
      current = current->next;
      free(temp);    // räcker detta?
    }
  
  head = NULL;
}



int row_to_int(char c)
{
  return c - 65;
}



int invalid_col(int n)
{
  if(n > 100)
    {
      return 1;
    }
  
  if(n < 1)
    {
      return 1;
    }
  
  else
    {
      return 0;
    }
}



void clear_screen(void)
{
  for(int i = 0; i< 3; i++)
    {                           
      printf("\n\n\n\n\n\n\n\n");
    }
}



void create_init_db(DB_t *head)
{
  Good g;
  Good g1;
  Good g2;
  Good g3;
  
  g1.name = "Milk";
  g1.descr = "Cows produced this. This item was added at startup using add_to_db(db, g)";
  g1.price = 20;
  g1.row = 'A';
  g1.column = 100;
  g1.amount = 1000;

  g2.name = "Beef";
  g2.descr = "Got some? This item was added at startup using add_to_db(db, g)";
  g2.price = 50;
  g2.row = 'Y';
  g2.column = 50;
  g2.amount = 200;

  g3.name = "Steak";
  g3.descr = "Just another description for steak. this item was added at startup using add_to_db(db, g)";
  g3.price = 2389;
  g3.row = 'G';
  g3.column = 29;
  g3.amount = 234;

  g.name = "item";
  g.descr = "This is item, testing replace_first_db without add_to_db";
  g.price = 500;
  g.row = 'A';
  g.column = 50;
  g.amount = 10000;
  
  add_to_db(head, g1);
  add_to_db(head, g3);
  add_to_db(head, g2);
  add_to_db(head, g);
  add_to_db(head, g1);
  add_to_db(head, g3);
  add_to_db(head, g2);
  add_to_db(head, g);
  add_to_db(head, g1);
  add_to_db(head, g3);
  add_to_db(head, g2);
  add_to_db(head, g);
  add_to_db(head, g1);
  add_to_db(head, g3);
  add_to_db(head, g2);
  add_to_db(head, g);
  add_to_db(head, g2);
  add_to_db(head, g);
  add_to_db(head, g1);
  add_to_db(head, g3);
}
