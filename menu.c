#include "config.h"
#include "menu.h"
#include "efi.h"
#include "pboot.h"
#include "types.h"
#include <stdint.h>

static uint8_t number_of_entries = 0;

static uint8_t entry_selected = 0;

void set_default_entry(uint8_t entry){
  default_entry = entry;
}

Unicode* get_selected_kernel() { 
	return entries[entry_selected].kernel_name; 
}

Unicode* get_selected_parameters() {

  return entries[entry_selected].kernel_parameters;
}


void print_entries() {
  SystemTable *system_table = get_system_table();

  for (uint8_t i = 0; i < number_of_entries; i++) {

    system_table->out->output_string(system_table->out, entries[i].entry_name);
    if (i == entry_selected) {
      system_table->out->output_string(system_table->out, u"*");
    }
    system_table->out->output_string(system_table->out, u"\n\r");
  }
}

void enter_in_menu_loop(){
	
	SystemTable* system_table = get_system_table();
	
	system_table->out->clear_screen(system_table->out);

	print_entries();

	InputKey key_pressed;
	while (1) {
	
	efi_status status;
	status = system_table->input->read_key_stroke(system_table->input, &key_pressed);

	if(status == EFI_SUCCESS){

		if(key_pressed.scan_code == KEY_CODE_UP || key_pressed.unicode_char == u'w'){
			if(entry_selected > 0){
				entry_selected--;
			}
		}

		if(key_pressed.scan_code == KEY_CODE_DOWN || key_pressed.unicode_char == u's'){

			if(entry_selected < number_of_entries-1){//-1 because start at 0
				entry_selected++;
			}	
		}
	
		system_table->out->clear_screen(system_table->out);
		print_entries();
	
		if(key_pressed.scan_code == KEY_CODE_RIGHT || key_pressed.unicode_char == u'd'){
			system_table->out->clear_screen(system_table->out);
			boot();		
		}
	}

	}
}

void can_show_menu(){

  number_of_entries = sizeof(entries) / sizeof(entries[0]);

  entry_selected = default_entry;

  InputKey key_pressed;

	SystemTable* system_table = get_system_table();

  system_table->input->read_key_stroke(system_table->input, &key_pressed);

  if (key_pressed.scan_code == KEY_CODE_LEFT ||
      key_pressed.unicode_char == u'a' || show_menu == true) {
    enter_in_menu_loop();
  }
}
