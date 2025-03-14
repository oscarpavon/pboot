#include "files.h"
#include "efi.h"
#include "utils.h"

#include "pboot.h"

static FileProtocol* root_directory;
static FileSystemProtocol* root_file_system;

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

uint64_t get_file_size(FileProtocol* file){
	Status status;
	status = file->set_position(file, 0xFFFFFFFFFFFFFFFF)	;
	uint64_t file_size;
	status = file->get_position(file, &file_size);
	status = file->set_position(file, 0);
	return file_size;
}

void* read_file(FileProtocol* file){
	void* memory;
  uint64_t file_size = get_file_size(file);
  allocate_memory(file_size , &memory);

  read_file_to_memory(file, file_size, memory);

	return memory;	
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


void setup_file_system(){

	//get loaded image to get device path
	get_loaded_image();

	struct GUID file_system_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
	
	LoadedImageProtocol* bootloader = get_bootloader_image();
	SystemTable* system_table = get_system_table();
	Handle bootloader_handle = get_bootloader_handle();

	system_table->boot_table->open_protocol(bootloader->device,
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

