#include "graphics.h"
#include "efi.h"
#include "pboot.h"
#include <stdint.h>

static EfiGraphicsOutputProtocol* graphics_output_protocol;
static FrameBuffer frame_buffer;
static GraphicsOutputProtocol* graphics;

static void* framebuffer_in_memory;

void* get_framebuffer(){
	return framebuffer_in_memory;
}

void plot_pixel(int x, int y, uint32_t pixel){
	FrameBuffer* frame = (FrameBuffer*)framebuffer_in_memory;
*((uint32_t*)(frame->frame_buffer
			+ 4 * frame->pixel_per_scan_line
			* y + 4 * x)) = pixel;
}

void get_graphics_output_protocol(){

  SystemTable* system_table = get_system_table();

	GUID guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	
	Status status;

	status = system_table->boot_table->locate_protocol(&guid,
			(void*)0, (void**)&graphics_output_protocol);

	if(status != EFI_SUCCESS){
	  log(u"Can't get Graphics Output Protocol with locate_protocol");
	}

	graphics = graphics_output_protocol;
	

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

	//get best mode
	uint64_t best_horizontal = 0;
	uint64_t best_mode = 0;
	for(int i = 0; i<graphics_output_protocol->mode->max_mode;i++){
		graphics->query_mode(graphics,i,&size_of_info,&info);
		if(info->horizontal_resolution > best_horizontal){
			best_mode = i;
			best_horizontal = info->horizontal_resolution;
			if(best_horizontal == 1920){
				break;//other resolution are too high
			}
		}
	}
	
	status = graphics->set_mode(graphics,best_mode)	;
	if(status != EFI_SUCCESS){
		log(u"Can't set best graphics mode");
	}


	//setup framebuffer
	frame_buffer.frame_buffer = 
		graphics_output_protocol->mode->frame_buffer_base_address;
	frame_buffer.pixel_per_scan_line = 
		graphics_output_protocol->mode->mode_info->pixel_per_scan_line;
	
	frame_buffer.horizontal_resolution =
		graphics_output_protocol->mode->mode_info->horizontal_resolution;

	frame_buffer.vertical_resolution =
		graphics_output_protocol->mode->mode_info->vertical_resolution;


  allocate_memory(sizeof(FrameBuffer), &framebuffer_in_memory);
  copy_memory(framebuffer_in_memory, &frame_buffer, sizeof(FrameBuffer));
	
}

