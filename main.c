#include "efi.h"
#include "config.h"
#include "types.h"
#include "files.h"
#include "utils.h"

static Handle* bootloader_handle;
static SystemTable* system_table;

static FileProtocol* opened_kernel_file;

static FileProtocol kernel_file;
	
static LoadedImageProtocol* bootloader_image;
static Handle kernel_image_handle;

static uint64_t kernel_image_entry;

static uint8_t entry_selected = 0;

static uint16_t* selected_kernel_name;
static uint16_t* selected_kernel_parameters;

static uint8_t number_of_entries = 0;


void print_entries(){
	for(uint8_t i = 0; i < number_of_entries; i++){
		system_table->out->output_string(system_table->out, entries[i].entry_name);
		if(i == entry_selected){
			system_table->out->output_string(system_table->out, u"*");
		}
		system_table->out->output_string(system_table->out, u"\n\r");
	}
}


void chainload_linux_efi_stub() {
  Status status;

  void *kernel_memory_allocated = read_file(opened_kernel_file);

	uint64_t kernel_file_size = get_file_size(opened_kernel_file);

  status = system_table->boot_table->image_load(
      false, bootloader_handle, bootloader_image->file_path,
      kernel_memory_allocated, kernel_file_size, &kernel_image_handle);
  if (status != EFI_SUCCESS) {
    log(u"Can't load kernel image");
  }

  uint16_t *arguments = selected_kernel_parameters;
  size_t arguments_size = u16strlen(arguments);
  arguments_size = arguments_size * sizeof(uint16_t);

  uint16_t *arguments_memory;
  system_table->boot_table->allocate_pool(EFI_LOADER_DATA, arguments_size,
                                          (void **)&arguments_memory);

  copy_memory(arguments_memory, arguments, arguments_size);

  // passing arguments
  struct GUID loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;

  struct LoadedImageProtocol *kernel_image;
  status = system_table->boot_table->open_protocol(
      kernel_image_handle, &loaded_image_guid, (void **)&kernel_image,
      kernel_image_handle, 0, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if (status != EFI_SUCCESS) {
    log(u"Can't get image");
  }

  kernel_image->device = bootloader_image->device;
  kernel_image->load_options = arguments_memory;
  kernel_image->load_options_size = arguments_size;

  status = system_table->boot_table->start_image(kernel_image_handle, 0, 0);

  if (status != EFI_SUCCESS) {
    log(u"Can't start kernel image");
  }
}

void get_loaded_image(){
	struct GUID loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	
	system_table->boot_table->open_protocol(bootloader_handle,
			&loaded_image_guid,
			(void **)&bootloader_image,
			bootloader_handle,
			0,
			EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL)	;
}

SystemTable* get_system_table(){
  return system_table;
}

LoadedImageProtocol* get_bootloader_image(){
	return bootloader_image;
}



void load_kernel_file(){
	open_file(&opened_kernel_file,selected_kernel_name);
}

void boot_entry(){
	
	selected_kernel_name = entries[entry_selected].kernel_name;
	selected_kernel_parameters = entries[entry_selected].kernel_parameters;
		
	load_kernel_file();
	chainload_linux_efi_stub();

}

void enter_in_menu_loop(){
	
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
			boot_entry();		
		}
	}

	}
}

void parse_configuration(uint64_t config_file_size, char* config){

	void* unicode_config;
	allocate_memory(config_file_size, &unicode_config);


	//fill memory by null character
	for(int i = 0; i < config_file_size; i++){
		short* data = (short*)unicode_config;
		char zero = '\0';
		data[i] = (short)zero;
	}
	
	//convert file to unicode 16
	for(int i = 0; i < config_file_size-1; i++){
		short* data = (short*)unicode_config;
		data[i] = (short)config[i];
	}


	uint16_t* new_config = (uint16_t*)unicode_config;

	//log(unicode_config);

	system_table->out->output_string(system_table->out,unicode_config);
	log(u"configuration loaded");
	//hang();
}

void load_configuration(){
	FileProtocol* config_file;
	open_file(&config_file, u"pboot.conf");
	char* config = read_file(config_file);

	uint64_t config_file_size = get_file_size(config_file);
	uint8_t entry = *config - '0';
	default_entry = entry;

}

Handle get_bootloader_handle(){
  return bootloader_handle;
}

void main(Handle in_bootloader_handle, SystemTable *in_system_table) {

  system_table = in_system_table;

  bootloader_handle = in_bootloader_handle;

  setup_file_system();

  load_configuration();

  number_of_entries = sizeof(entries) / sizeof(entries[0]);

  entry_selected = default_entry;
  selected_kernel_name = entries[entry_selected].kernel_name;
  selected_kernel_parameters = entries[entry_selected].kernel_parameters;

  InputKey key_pressed;

  system_table->input->read_key_stroke(system_table->input, &key_pressed);

  if (key_pressed.scan_code == KEY_CODE_LEFT ||
      key_pressed.unicode_char == u'a' || show_menu == true) {
    enter_in_menu_loop();
  }

  load_kernel_file();
  chainload_linux_efi_stub();

  // we never got here
}
