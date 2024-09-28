#include "efi.h"
#include "config.h"
#include <stdint.h>


struct efi_system_table* system_table;

uint8_t entry_selected = 0;

void print_entries(){
	uint8_t number_of_entries = sizeof(entries)/sizeof(entries[0]);
	for(uint8_t i = 0; i < number_of_entries; i++){
		system_table->out->output_string(system_table->out, entries[i].name);
		if(i == entry_selected){
			system_table->out->output_string(system_table->out, u"*");
		}
		system_table->out->output_string(system_table->out, u"\n\r");
	}
}

void print_selection_counter(){
	system_table->out->output_string(system_table->out, u"10s");
}

efi_status_t efi_main(
	efi_handle_t handle, struct efi_system_table *in_system_table)
{

	system_table = in_system_table;

	efi_status_t status;

	status = system_table->out->clear_screen(system_table->out);
	if (status != 0)
		return status;


	if(show_bootloader){
		print_entries();
		print_selection_counter();
	}else{
		system_table->out->output_string(system_table->out, u"Booting ");
		system_table->out->output_string(system_table->out, entries[default_entry].name);
		system_table->out->output_string(system_table->out, u"\n\r");
	}

	InputKey key_pressed;
	while (1) {
	
	system_table->input->read_key_stroke(system_table->input, &key_pressed);

	if(key_pressed.scan_code == KEY_CODE_UP){
		
		system_table->out->clear_screen(system_table->out);
		entry_selected--;
		print_entries();
		print_selection_counter();
	}
	if(key_pressed.scan_code == KEY_CODE_DOWN){
		
		system_table->out->clear_screen(system_table->out);
		entry_selected++;
		print_entries();
		print_selection_counter();
	}

	}
	return 0;
}


