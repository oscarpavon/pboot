#include "efi.h"
#include "config.h"



struct SystemTable* system_table;
Handle* bootloader_handle;

struct FileProtocol* opened_kernel_file;

struct FileSystemProtocol* root_file_system;
struct FileProtocol* root_directory;
	
struct LoadedImageProtocol* bootloader_image;
Handle kernel_image_handle;

Handle main_device;

uint64_t kernel_image_entry;

uint8_t entry_selected = 0;


void log(uint16_t* text){
	
	system_table->out->output_string(system_table->out,text);
	system_table->out->output_string(system_table->out,u"\n\r");

}

static void exit_boot_services(){

	struct MemoryDescriptor *mmap;
	efi_uint_t mmap_size = 4096;
	efi_uint_t mmap_key;
	efi_uint_t desc_size;
	uint32_t desc_version;
	efi_status_t status;

	while (1) {
		status = system_table->boot_table->allocate_pool(
			EFI_LOADER_DATA,
			mmap_size,
			(void **)&mmap);
		if(status != EFI_SUCCESS){
			log(u"Can't allocate memory for memory map");
		}

		status = system_table->boot_table->get_memory_map(
			&mmap_size,
			mmap,
			&mmap_key,
			&desc_size,
			&desc_version);
		if (status == EFI_SUCCESS){
			break;
		}


	}

	status = system_table->boot_table->exit_boot_services(bootloader_handle, 
			mmap_key);
	if(status != EFI_SUCCESS){
		log(u"ERROR boot service not closed");
		return;
	}

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


efi_status_t read_fixed(
	struct SystemTable *system,
	struct FileProtocol *file,
	uint64_t offset,
	size_t size,
	void *dst)
{
	efi_status_t status = EFI_SUCCESS;
	unsigned char *buf = dst;
	size_t read = 0;

	status = file->set_position(file, offset);
	if (status != EFI_SUCCESS) {

		return status;
	}

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
	uint8_t number_of_entries = sizeof(entries)/sizeof(entries[0]);
	for(uint8_t i = 0; i < number_of_entries; i++){
		system_table->out->output_string(system_table->out, entries[i].name);
		if(i == entry_selected){
			system_table->out->output_string(system_table->out, u"*");
		}
		system_table->out->output_string(system_table->out, u"\n\r");
	}
}


void bootloader_loop(){

	if(show_bootloader){ 
		print_entries();
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
	}
	if(key_pressed.scan_code == KEY_CODE_DOWN){
		
		system_table->out->clear_screen(system_table->out);
		entry_selected++;
		print_entries();
	}

	}
}
void chainload_linux_efi_stub(){
	efi_status_t status;
	status = opened_kernel_file->set_position(opened_kernel_file, 0xFFFFFFFFFFFFFFFF)	;
	uint64_t kernel_file_size;
	status = opened_kernel_file->get_position(opened_kernel_file, &kernel_file_size);

	uint64_t *kernel_memory_allocated;

	system_table->boot_table->allocate_pool(
			EFI_LOADER_DATA, 
			kernel_file_size, 
			(void **)&kernel_memory_allocated
			);


	read_fixed(system_table, opened_kernel_file, 0,
			kernel_file_size, kernel_memory_allocated);
	
	status = system_table->boot_table->image_load(false, bootloader_handle, bootloader_image->file_path, kernel_memory_allocated, 
			kernel_file_size, &kernel_image_handle);
	if(status != EFI_SUCCESS){
		log(u"Can't load kernel image");
	}

	uint16_t * arguments = u"root=/dev/nvme0n1p3 rw fstype=ext4";
	size_t arguments_size = u16strlen(arguments);
	arguments_size = arguments_size * sizeof(uint16_t);	

	uint16_t* arguments_memory;
	system_table->boot_table->allocate_pool(
			EFI_LOADER_DATA, 
			arguments_size, 
			(void **)&arguments_memory
			);

	copy_memory(arguments_memory, arguments,
		arguments_size);

	//passing arguments
	struct GUID loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	
	struct LoadedImageProtocol* kernel_image;
	status = system_table->boot_table->open_protocol(kernel_image_handle,
			&loaded_image_guid,
			(void**)&kernel_image,
			kernel_image_handle,
			0,
			EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL)	;
	if(status != EFI_SUCCESS){
		log(u"Can't get image");
	}

	kernel_image->device = main_device;
	kernel_image->load_options = arguments_memory;
	kernel_image->load_options_size = arguments_size;

	status = system_table->boot_table->start_image(kernel_image_handle, 0, 0);
	
	if(status != EFI_SUCCESS){
		log(u"Can't start kernel image");
	}

}

void load_kernel_file(){

	efi_status_t status;

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


	efi_status_t open_volumen_status = 
		root_file_system->open_volume(root_file_system, &root_directory);

	if(open_volumen_status != EFI_SUCCESS){

		system_table->out->output_string(system_table->out, u"Open volume error \n\r");
	}


	efi_status_t open_kernel_status = root_directory->open(
			root_directory,
			&opened_kernel_file,
			u"vmlinuz",
			EFI_FILE_MODE_READ,
			EFI_FILE_READ_ONLY
			);	

	if(open_kernel_status != EFI_SUCCESS){
		log(u"Can't open kernel file");
	}


}


efi_status_t efi_main(
	Handle in_bootloader_handle, struct SystemTable *in_system_table)
{

	system_table = in_system_table;

	bootloader_handle = in_bootloader_handle;

	load_kernel_file();
	chainload_linux_efi_stub();
	//bootloader_loop();
	while(1){};
	return 0;
}


