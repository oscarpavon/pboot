#include "graphics.h"
#include "efi.h"
#include "pboot.h"

EfiGraphicsOutputProtocol* graphics_output_protocol;
FrameBuffer frame_buffer;

void get_graphics_output_protocol(){

  SystemTable* system_table = get_system_table();

	GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	
	Status status;

	status = system_table->boot_table->locate_protocol(&gop_guid,
			(void*)0, (void**)&graphics_output_protocol);

	if(status != EFI_SUCCESS){
	  log(u"Can't get Graphics Output Protocol with locate_protocol");
	}
	

	//get the current mode

	EfiGraphicsOutputModeInformation *info;
	uint64_t size_of_info, number_of_modes, native_mode;
	if(graphics_output_protocol->mode == NULL){
		log(u"Graphics Output Protocol Mode is NULL");
		status = graphics_output_protocol->query_mode(graphics_output_protocol,
				0 , &size_of_info, &info);
	}else{
		status = graphics_output_protocol->query_mode(graphics_output_protocol,
				graphics_output_protocol->mode->mode , &size_of_info, &info);
	}

	if(status != EFI_SUCCESS){
		log(u"Can't query mode");
	}
	
	native_mode = graphics_output_protocol->mode->mode;
	number_of_modes = graphics_output_protocol->mode->max_mode;


	//setup framebuffer
	frame_buffer.frame_buffer = 
		graphics_output_protocol->mode->frame_buffer_base_address;
	frame_buffer.pixel_per_scan_line = 
		graphics_output_protocol->mode->mode_info->pixel_per_scan_line;
	
	frame_buffer.horizontal_resolution =
		graphics_output_protocol->mode->mode_info->horizontal_resolution;

	frame_buffer.vertical_resolution =
		graphics_output_protocol->mode->mode_info->vertical_resolution;


}

