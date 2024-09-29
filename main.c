#include "efi.h"
#include "config.h"
#include "elf.h"
#include <stdint.h>


struct SystemTable* system_table;

uint8_t entry_selected = 0;



static void get_image_size(
	struct ElfHeader* kernel_header,
	struct ElfProgramHeader* program_headers,
	uint64_t alignment,
	uint64_t *out_begin,
	uint64_t *out_end)
{
	*out_begin = UINT64_MAX;
	*out_end = 0;

	for (size_t i = 0; i < kernel_header->program_header_number_of_entries; ++i) {
		struct ElfProgramHeader *phdr = &program_headers[i];
		uint64_t phdr_begin, phdr_end;
		uint64_t align = alignment;

		if (phdr->p_type != PT_LOAD)
			continue;

		if (phdr->p_align > align)
			align = phdr->p_align;

		phdr_begin = phdr->p_vaddr;
		phdr_begin &= ~(align - 1);
		if (*out_begin > phdr_begin)
			*out_begin = phdr_begin;

		phdr_end = phdr->p_vaddr + phdr->p_memsz + align - 1;
		phdr_end &= ~(align - 1);
		if (*out_end < phdr_end)
			*out_end = phdr_end;
	}
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

	struct FileProtocol* opened_kernel_file;

	efi_status_t open_kernel_status = root_directory->open(
			root_directory,
			&opened_kernel_file,
			u"vmlinux",
			EFI_FILE_MODE_READ,
			EFI_FILE_READ_ONLY
			);	

	struct ElfHeader kernel_elf_header;	
	read_fixed(system_table, opened_kernel_file, 0, 
			sizeof(struct ElfHeader), &kernel_elf_header);

	struct ElfProgramHeader* kernel_program_headers;

	system_table->boot_table->allocate_pool(EFI_LOADER_DATA,
			kernel_elf_header.program_header_number_of_entries * 
			kernel_elf_header.program_header_entry_size,
			(void**)kernel_program_headers)	;

	read_fixed(system_table, opened_kernel_file,
			kernel_elf_header.program_header_offset, 
			kernel_elf_header.program_header_number_of_entries *
			kernel_elf_header.program_header_entry_size,
			(void*)&kernel_program_headers);

	
	uint64_t page_size = 4096;
	uint64_t image_begin;
	uint64_t image_end;
	uint64_t image_size;
	uint64_t image_address;

	
	get_image_size(&kernel_elf_header,
			kernel_program_headers, 
			page_size, &image_begin, &image_end);

	image_size = image_end - image_begin;

	system_table->boot_table->allocate_pages(EFI_ALLOCATE_ANY_PAGES,
			EFI_LOADER_DATA, image_size / page_size,
			&image_address);




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


