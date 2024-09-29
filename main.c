#include "efi.h"
#include "config.h"
#include "elf.h"


#define ELFABI __attribute__((sysv_abi))

struct SystemTable* system_table;
Handle* bootloader_handle;

struct ElfProgramHeader* kernel_program_headers;
struct ElfHeader kernel_elf_header;	
struct FileProtocol* opened_kernel_file;

struct FileSystemProtocol* root_file_system;
struct FileProtocol* root_directory;
	
struct LoadedImageProtocol* bootloader_image;

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
			//log(u"Can't allocate memory for memory map");
		}

		status = system_table->boot_table->get_memory_map(
			&mmap_size,
			mmap,
			&mmap_key,
			&desc_size,
			&desc_version);
		if (status == EFI_SUCCESS){
			//log(u"Got memory map");
			break;
		}


		//system_table->boot_table->free_pool(mmap);
		
		if (status == EFI_BUFFER_TOO_SMALL) {
			mmap_size *= 2;
			continue;
		}

	}

	//log(u"Closing boot services..");
	status = system_table->boot_table->exit_boot_services(bootloader_handle, 
			mmap_key);
	if(status != EFI_SUCCESS){

		//log(u"ERROR boot service not closed");
		return;
	}

	//system_table->boot_table->free_pool(mmap);

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

struct ReserveMemory {
	const char *name;
	uint64_t begin;
	uint64_t end;
};

size_t reserves_count;
size_t reserve_capacity;
struct ReserveMemory* main_reserve;

static efi_status reserve_memory(uint64_t begin, uint64_t end){
	if(reserves_count == reserve_capacity){

	size_t new_size = 2 * reserves_count;
	struct ReserveMemory* new_reserve = 0;
	struct ReserveMemory* old_reserve = main_reserve;


	if(new_size == 0)	
		new_size = 16;

	system_table->boot_table->allocate_pool(
			EFI_LOADER_DATA, 
			new_size * sizeof(struct ReserveMemory), 
			(void **)&new_reserve
			);

	copy_memory(new_reserve, old_reserve,
			reserves_count * sizeof(struct ReserveMemory));

	main_reserve = new_reserve;
	reserve_capacity = new_size;
	
	if(old_reserve != 0){
		system_table->boot_table->free_pool((void*)old_reserve);
	}
	}	
	
	set_memory(&main_reserve[reserves_count],0,sizeof(struct ReserveMemory));
	main_reserve[reserves_count].name = "Kernel";
	main_reserve[reserves_count].begin = begin;
	main_reserve[reserves_count].end = end;
	reserves_count++;

}

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

void bootloader_loop(){

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
}


void load_kernel(){

	efi_status_t status;

	//get loaded image to get device path
	struct GUID loaded_image_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	
	system_table->boot_table->open_protocol(bootloader_handle,
			&loaded_image_guid,
			&bootloader_image,
			bootloader_handle,
			0,
			EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL)	;

	Handle device = bootloader_image->device;

	struct GUID file_system_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

	system_table->boot_table->open_protocol(device,
			&file_system_guid,
			&root_file_system,
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
			u"vmlinux",
			EFI_FILE_MODE_READ,
			EFI_FILE_READ_ONLY
			);	

	read_fixed(system_table, opened_kernel_file, 0, 
			sizeof(struct ElfHeader), &kernel_elf_header);


	system_table->boot_table->allocate_pool(EFI_LOADER_DATA,
			kernel_elf_header.program_header_number_of_entries * 
			kernel_elf_header.program_header_entry_size,
			(void**)kernel_program_headers)	;

	read_fixed(system_table, opened_kernel_file,
			kernel_elf_header.program_header_offset, 
			kernel_elf_header.program_header_number_of_entries *
			kernel_elf_header.program_header_entry_size,
			(void*)&kernel_program_headers);
	

	system_table->out->output_string(system_table->out, u"Kernel ELF readed\n\r");
	
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


	system_table->out->output_string(system_table->out, u"Load kernel to memroy\n\r");
	
	for (size_t i = 0; i < kernel_elf_header.program_header_number_of_entries; ++i) {
		struct ElfProgramHeader *phdr = &kernel_program_headers[i];

		uint64_t phdr_addr;

		if (phdr->p_type != PT_LOAD)
			continue;

		phdr_addr = image_address + phdr->p_vaddr - image_begin;
		status = read_fixed(
			system_table,
			opened_kernel_file,
			phdr->p_offset,
			phdr->p_filesz,
			(void *)phdr_addr);
		if (status != EFI_SUCCESS) {

			system_table->out->output_string(system_table->out, 
					u"Failed to read kernel segment in memory\n\r");
		}
	

		reserve_memory(phdr_addr, phdr_addr + phdr->p_memsz);



	}

	uint64_t kernel_image_entry = image_address + kernel_elf_header.e_entry - image_begin;	

	log(u"Kernel loaded to memory");

}


efi_status_t efi_main(
	Handle in_bootloader_handle, struct SystemTable *in_system_table)
{

	system_table = in_system_table;

	bootloader_handle = in_bootloader_handle;

	log(u"Bootloader started")	;
	load_kernel();
	

	void(ELFABI* entry)(struct ReserveMemory*, size_t);
	//system_table->out->clear_screen(system_table->out);
	exit_boot_services();

	entry = (void (ELFABI*)(struct ReserveMemory*,size_t))kernel_image_entry;
	(*entry)(main_reserve, reserves_count);
	
	//bootloader_loop();
	while(1){};
	return 0;
}


