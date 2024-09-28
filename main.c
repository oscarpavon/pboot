#include "efi.h"
#include "config.h"
#include <stdint.h>


struct SystemTable* system_table;

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
	Handle bootloader_handle, struct SystemTable *in_system_table)
{

	system_table = in_system_table;

	efi_status_t status;

	status = system_table->out->clear_screen(system_table->out);
	if (status != 0)
		return status;

	
	//get loaded image to get device path
	struct LoadedImageProtocol* bootloader_image;
	struct GUID loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	
	system_table->boot_table->open_protocol(bootloader_handle,
			&loaded_image_guid,
			&bootloader_image,
			bootloader_handle,
			0,
			EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL)	;

	Handle device = bootloader_image->device;

	struct GUID file_system_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	struct FileSystemProtocol* root_file_system;

	system_table->boot_table->open_protocol(device,
			&file_system_guid,
			&root_file_system,
			bootloader_handle,
			0,
			EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL)	;

	struct FileProtocol* root_directory;

	efi_status_t open_volumen_status = 
		root_file_system->open_volume(root_file_system, &root_directory);

	if(open_volumen_status != EFI_SUCCESS){

		system_table->out->output_string(system_table->out, u"Open volume error \n\r");
	}

	struct FileProtocol* opend_kernel_file;

	efi_status_t open_kernel_status = root_directory->open(
			root_directory,
			&opend_kernel_file,
			u"vmlinuz.efi",
			EFI_FILE_MODE_READ,
			EFI_FILE_READ_ONLY
			);	
	

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


