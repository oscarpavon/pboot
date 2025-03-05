#include "efi.h"
#include "config.h"
#include "types.h"
#include <stdint.h>

static Handle* bootloader_handle;
static SystemTable* system_table;

static struct FileProtocol* opened_kernel_file;
//static struct FileProtocol opened_kernel_file;
static struct FileProtocol kernel_file;

static struct FileSystemProtocol* root_file_system;
static struct FileProtocol* root_directory;
	
static struct LoadedImageProtocol* bootloader_image;
static Handle kernel_image_handle;

static Handle main_device;

static uint64_t kernel_image_entry;

static uint8_t entry_selected = 0;

static uint16_t* selected_kernel_name;
static uint16_t* selected_kernel_parameters;

static uint8_t number_of_entries = 0;


void log(uint16_t* text){
	
	system_table->out->output_string(system_table->out,text);
	system_table->out->output_string(system_table->out,u"\n\r");

}


size_t u16strlen(const uint16_t *str)
{
	const uint16_t *pos = str;

	while (*pos++)
		;
	return pos - str - 1;
}

void *set_memory(void *pointer, int value, size_t size)
{
	char *to = pointer;

	for (size_t i = 0; i < size; ++i)
		*to++ = value;
	return pointer;
}

void *copy_memory(void *destination, const void *source, size_t size)
{
	const char *from = source;
	char *to = destination;

	for (size_t i = 0; i < size; ++i)
		*to++ = *from++;
	return destination;
}


 Status read_file_to_memory(
	struct FileProtocol *file,
	size_t size,
	void *destination)
{
	Status status = EFI_SUCCESS;
	unsigned char *buf = destination;
	size_t read = 0;

	while (read < size) {
		efi_uint_t remains = size - read;

		status = file->read(file, &remains, (void *)(buf + read));
		if (status != EFI_SUCCESS) {
	
			return status;
		}

		read += remains;
	}

	return status;
}

void print_entries(){
	for(uint8_t i = 0; i < number_of_entries; i++){
		system_table->out->output_string(system_table->out, entries[i].entry_name);
		if(i == entry_selected){
			system_table->out->output_string(system_table->out, u"*");
		}
		system_table->out->output_string(system_table->out, u"\n\r");
	}
}

uint64_t get_file_size(FileProtocol* file){
	Status status;
	status = file->set_position(file, 0xFFFFFFFFFFFFFFFF)	;
	uint64_t file_size;
	status = file->get_position(file, &file_size);
	status = file->set_position(file, 0);
	return file_size;
}

void allocate_memory(uint64_t size, void** memory){

	system_table->boot_table->allocate_pool(
			EFI_LOADER_DATA, size, memory);
}

void* read_file(FileProtocol* file){
	void* memory;
  uint64_t file_size = get_file_size(file);
  allocate_memory(file_size , &memory);

  read_file_to_memory(file, file_size, memory);

	return memory;	
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

  kernel_image->device = main_device;
  kernel_image->load_options = arguments_memory;
  kernel_image->load_options_size = arguments_size;

  status = system_table->boot_table->start_image(kernel_image_handle, 0, 0);

  if (status != EFI_SUCCESS) {
    log(u"Can't start kernel image");
  }
}

void setup_file_system(){

	//get loaded image to get device path
	struct GUID loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	
	system_table->boot_table->open_protocol(bootloader_handle,
			&loaded_image_guid,
			(void **)&bootloader_image,
			bootloader_handle,
			0,
			EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL)	;

	main_device = bootloader_image->device;

	struct GUID file_system_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

	system_table->boot_table->open_protocol(main_device,
			&file_system_guid,
			(void**)&root_file_system,
			bootloader_handle,
			0,
			EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL)	;


	Status open_volumen_status = 
		root_file_system->open_volume(root_file_system, &root_directory);

	if(open_volumen_status != EFI_SUCCESS){
		log(u"Open volume error");
	}

}

void hang(){
	while (1) {};
}

void open_file(FileProtocol** file, uint16_t* name){

	Status status = root_directory->open(
			root_directory,
			file,
			name,
			EFI_FILE_MODE_READ,
			EFI_FILE_READ_ONLY
			);	

	if(status != EFI_SUCCESS){
		log(u"Can't open file");
		log(u"File name:");
		log(name);
		hang();
	}
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

		if(key_pressed.scan_code == KEY_CODE_UP){
			if(entry_selected > 0){
				entry_selected--;
			}
		}

		if(key_pressed.scan_code == KEY_CODE_DOWN){

			if(entry_selected < number_of_entries-1){//-1 because start at 0
				entry_selected++;
			}	
		}
	
		system_table->out->clear_screen(system_table->out);
		print_entries();
	
		if(key_pressed.scan_code == KEY_CODE_RIGHT){
			system_table->out->clear_screen(system_table->out);
			boot_entry();		
		}
	}

	}
}

void load_configuration(){
	FileProtocol* config_file;
	open_file(&config_file, u"pboot.conf");
	char* config = read_file(config_file);

	uint64_t config_file_size = get_file_size(config_file);
	log(u"config file size");
	char size_char = config_file_size+'0';
	uint16_t size_unicode;
	size_unicode = (short)size_char;
	log(&size_unicode);
	

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
	log(u"configuration");
	hang();

}

void main(Handle in_bootloader_handle, SystemTable *in_system_table)
{

	system_table = in_system_table;

	bootloader_handle = in_bootloader_handle;

	setup_file_system();

	load_configuration();

	number_of_entries = sizeof(entries)/sizeof(entries[0]);

	entry_selected = default_entry;
	//selected_kernel_name = entries[entry_selected].kernel_name;
	selected_kernel_name = u"pLinux";
	//selected_kernel_parameters = entries[entry_selected].kernel_parameters;
	selected_kernel_parameters = u"";

	InputKey key_pressed;
	
	system_table->input->read_key_stroke(system_table->input, &key_pressed);
	
	if(key_pressed.scan_code == KEY_CODE_LEFT || show_menu == true){
		enter_in_menu_loop();
	}

	load_kernel_file();
	chainload_linux_efi_stub();

	//we never got here
}


