#ifndef __MENU_H__
#define __MENU_H__

#include "types.h"

Unicode* get_selected_parameters();

BootLoaderEntry* get_entries();

Unicode* get_selected_kernel();

void can_show_menu();

void set_number_of_entries(uint8_t number);

void set_show_menu(bool can_show);

#endif
