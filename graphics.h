#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "types.h"
#include "efi.h"

//  blt mean Block Transfer o Blt

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
{0x9042a9de,0x23dc,0x4a38,\
{0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a}}

typedef struct {
	uint32_t red_mask;
	uint32_t green_mask;
	uint32_t blue_mask;
	uint32_t reserved_mask;
} EfiPixelBitMask;


typedef enum {
	PixelRedGreenBlueReserved8BitPerColor,
	PixelBlueGreenRedReserved8BitPerColor,
	PixelBitMask,
	PixelBltOnly,
	PixelFormatMax
}EfiGraphicsPixelFormat;


typedef struct {
	uint32_t version;
	uint32_t horizontal_resolution;
	uint32_t vertical_resolution;
	EfiGraphicsPixelFormat pixel_format;
	EfiPixelBitMask pixel_information;
	uint32_t pixel_per_scan_line;
}EfiGraphicsOutputModeInformation;

typedef struct{
	uint32_t max_mode;
	uint32_t mode;
	EfiGraphicsOutputModeInformation *mode_info;
	uint64_t size_of_info;
	uint64_t frame_buffer_base_address;
	uint64_t frame_buffer_size;
} EfiGraphicsOutputProtocolMode;


typedef struct{
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t reserved;
} EfiGraphicsOutputBlockTransferPixel;

typedef enum {
EfiBltVideoFill,
EfiBltVideoToBltBuffer,
EfiBltBufferToVideo,
EfiBltVideoToVideo,
EfiGraphicsOutputBltOperationMax
} EfiGraphicsOutputBlockTransferOperation;



typedef struct EfiGraphicsOutputProtocol{
	//query_mode
	Status (*query_mode)(struct EfiGraphicsOutputProtocol *self,
			uint32_t mode_number,
			uint64_t *size_of_info,
			EfiGraphicsOutputModeInformation **mode_info);
	//set_mode
	Status (*set_mode)(struct EfiGraphicsOutputProtocol *self, 
			uint32_t mode_number);
	//blt
	Status (*unused1)();

	EfiGraphicsOutputProtocolMode *mode;
} EfiGraphicsOutputProtocol;

typedef EfiGraphicsOutputProtocol GraphicsOutputProtocol;

struct FrameBuffer{
	uint64_t frame_buffer;
	uint32_t pixel_per_scan_line;
	uint32_t vertical_resolution;
	uint32_t horizontal_resolution;
};

typedef struct FrameBuffer FrameBuffer;

void get_graphics_output_protocol();
void* get_framebuffer();

#endif
