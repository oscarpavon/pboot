#include "pboot.h"

#include "menu.h"

static Handle* bootloader_handle;
static SystemTable* system_table;

static FileProtocol* opened_kernel_file;

static LoadedImageProtocol* bootloader_image;

static uint16_t* selected_kernel_name;
static uint16_t* selected_kernel_parameters;

Handle get_bootloader_handle(){
  return bootloader_handle;
}

SystemTable* get_system_table(){
  return system_table;
}

LoadedImageProtocol* get_bootloader_image(){
	return bootloader_image;
}

void chainload_linux_efi_stub() {
  Status status;

  Handle kernel_image_handle;

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

void load_kernel_file(){
	open_file(&opened_kernel_file,selected_kernel_name);
}

void boot(){

  selected_kernel_name = get_selected_kernel();
	selected_kernel_parameters = get_selected_parameters();
		
	load_kernel_file();
	chainload_linux_efi_stub();

}

void main(Handle in_bootloader_handle, SystemTable *in_system_table) {

  system_table = in_system_table;

  bootloader_handle = in_bootloader_handle;

  setup_file_system();

  load_configuration();

  can_show_menu();
  
  boot();

  // we never got here
}
