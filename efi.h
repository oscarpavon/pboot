#include "types.h"

static const uint32_t EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL = 0x00000001;

struct GUID{
	uint32_t data1;
	uint16_t data2;
	uint16_t data3;
	uint8_t data4[8];
};

struct TableHeader{
	uint64_t signature;
	uint32_t revision;
	uint32_t header_size;
	uint32_t crc32;
	uint32_t reserved;
};

typedef uint64_t efi_status_t;
typedef efi_status_t efi_status;


typedef struct{
	uint8_t type;
	uint8_t sub_type;
	uint8_t length[2];
}DevicePathProtocol;




#define KEY_CODE_UP 0x01
#define KEY_CODE_DOWN 0x02

typedef struct{
	uint16_t scan_code;
	uint16_t unicode_char;
}InputKey;

struct InputProtocol{
	void (*reset)();
	efi_status (*read_key_stroke)(struct InputProtocol *self, 
			InputKey* key);
	void (*wait_for_key)();//boot_services.wait_for_event()
};

struct SystemTable{
	struct TableHeader header;
	uint16_t *unused1;//firmware vendor
	uint32_t unused2;//firmware revision
	void *unused3;//console in handle
	struct InputProtocol *input; //ConIn
	void *unused5;//console out handle
	struct efi_simple_text_output_protocol *out;//console out
	void *unused6;//standard error handle
	void *unused7;//standard error
	void *unused8;//runtime services
	struct BootTable* boot_table;//boot services
	uint64_t unused10;//number of table entries
	void *unused11;//configuration table
};

typedef void * Handle;

typedef uint64_t efi_uint_t;

struct MemoryDescriptor{
	uint32_t type;
	uint64_t physical_start;
	uint64_t virtual_start;
	uint64_t pages;
	uint64_t attributes;
};

enum efi_allocate_type {
	EFI_ALLOCATE_ANY_PAGES,
	EFI_ALLOCATE_MAX_ADDRESS,
	EFI_ALLOCATE_ADDRESS,
	EFI_MAX_ALLOCATE_TYPE,
};

enum MemoryType{
	EFI_RESERVED_MEMORY_TYPE,
	EFI_LOADER_CODE,
	EFI_LOADER_DATA,
	EFI_BOOT_SERVICES_CODE,
	EFI_BOOT_SERVICES_DATA,
	EFI_RUNTIME_SERVICES_CODE,
	EFI_RUNTIME_SERVICES_DATA,
	EFI_CONVENTIAL_MEMORY,
	EFI_UNUSABLE_MEMORY,
	EFI_ACPI_RECLAIM_MEMORY,
	EFI_ACPI_MEMORY_NVS,
	EFI_MEMORY_MAPPED_IO,
	EFI_MEMORY_MAPPED_IO_PORT_SPACE,
	EFI_PAL_CODE,
	EFI_PERSISTENT_MEMORY,
	EFI_MAX_MEMORY_TYPE,
};

// Open modes
static const uint64_t EFI_FILE_MODE_READ = 0x0000000000000001;
static const uint64_t EFI_FILE_MODE_WRITE = 0x0000000000000002;
static const uint64_t EFI_FILE_MODE_CREATE = 0x8000000000000000;

// File attributes
static const uint64_t EFI_FILE_READ_ONLY = 0x1;
static const uint64_t EFI_FILE_HIDDEN = 0x2;
static const uint64_t EFI_FILE_SYSTEM = 0x4;
static const uint64_t EFI_FILE_RESERVED = 0x8;
static const uint64_t EFI_FILE_DIRECTORY = 0x10;
static const uint64_t EFI_FILE_ARCHIVE = 0x20;

static const uint64_t MAX_BIT = 0x8000000000000000ULL;

#define ERROR_CODE(status) (MAX_BIT | (status))

static const efi_status_t EFI_SUCCESS = 0;
static const efi_status_t EFI_LOAD_ERROR = ERROR_CODE(1);
static const efi_status_t EFI_INVALID_PARAMETER = ERROR_CODE(2);
static const efi_status_t EFI_UNSUPPORTED = ERROR_CODE(3);
static const efi_status_t EFI_BUFFER_TOO_SMALL = ERROR_CODE(5);

struct FileProtocol{
    uint64_t revision;
    efi_status_t (*open)(
        struct FileProtocol* self,
        struct FileProtocol** new_handle,
        uint16_t * file_name,
        uint64_t open_mode,
        uint64_t attributes);

    efi_status_t (*close)(struct FileProtocol*);

    void (*unused1)();

    efi_status_t (*read)(struct FileProtocol*, efi_uint_t *, void *);

    void (*unused2)();

    efi_status_t (*get_position)(struct FileProtocol*, uint64_t *);
    efi_status_t (*set_position)(struct FileProtocol*, uint64_t);

    efi_status_t (*get_info)(
        struct FileProtocol*, struct GUID*, efi_uint_t *, void *);

    void (*unused6)();
    void (*unused7)();
    void (*unused8)();
    void (*unused9)();
    void (*unused10)();
    void (*unused11)();
};

#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
    { 0x0964e5b22, 0x6459, 0x11d2, \
      { 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b } }

struct FileSystemProtocol{
    uint64_t revision;
    efi_status_t (*open_volume)(
        struct FileSystemProtocol* self, struct FileProtocol** root);
};

#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
	{ 0x5b1b31a1, 0x9562, 0x11d2, \
	  { 0x8e, 0x3f, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b } }

struct LoadedImageProtocol{
	uint32_t revision;
	Handle parent;
	struct SystemTable *system;

	// Source location of the image
	Handle device;
	DevicePathProtocol *file_path;
	void *reserved;

	// Image's load options
	uint32_t load_options_size;
	void *load_options;

	// Location of the image in memory
	void *image_base;
	uint64_t image_size;
	enum MemoryType image_code_type;
	enum MemoryType image_data_type;
	void (*unused)();
};


struct BootTable 
{
	struct TableHeader header;

	// Task Priority Services
	void (*unused1)();
	void (*unused2)();

	// Memory Services
	efi_status_t (*allocate_pages)(
		enum efi_allocate_type,
		enum MemoryType,
		efi_uint_t,
		uint64_t *);
	efi_status_t (*free_pages)(uint64_t, efi_uint_t);
	efi_status_t (*get_memory_map)(
		efi_uint_t *,
		struct MemoryDescriptor*,
		efi_uint_t *,
		efi_uint_t *,
		uint32_t *);
	efi_status_t (*allocate_pool)(
		enum MemoryType, efi_uint_t, void **);
	efi_status_t (*free_pool)(void *);

	// Event & Timer Services
	void (*unused7)();
	void (*unused8)();
	void (*unused9)();
	void (*unused10)();
	void (*unused11)();
	void (*unused12)();

	// Protocol Handler Services
	void (*unused13)();
	void (*unused14)();
	void (*unused15)();
	void (*unused16)();
	void *reserved;
	void (*unused17)();
	void (*unused18)();
	void (*unused19)();
	void (*unused20)();

	// Image Services
	efi_status (*image_load)(bool boot_policy,
			Handle parent_image_handle,
			DevicePathProtocol *device_path,
			void* source_buffer,
			efi_uint_t source_size,
			Handle* image_handle);

	efi_status_t (*start_image)(Handle image_handle,
			uint64_t * exit_data_size, 
			uint16_t **exit_data);

	void (*unused23)();
	void (*unused24)();

	efi_status_t (*exit_boot_services)(Handle, efi_uint_t);

	// Miscellaneius Services
	void (*unused26)();
	void (*unused27)();
	void (*unused28)();

	// DriverSupport Services
	void (*unused29)();
	void (*unused30)();

	// Open and Close Protocol Services
	efi_status_t (*open_protocol)(
		Handle handle,
		struct GUID * protocol,
		void ** interface,//optional
		Handle agent_handle,
		Handle controller_handle,
		uint32_t attributes);

	efi_status_t (*close_protocol)(
		Handle,
		struct GUID *,
		Handle,
		Handle);
	void (*unused33)();

	// Library Services
	efi_status_t (*protocols_per_handle)(
		Handle, struct GUID ***, efi_uint_t *);
	void (*unused35)();
	void (*unused36)();
	void (*unused37)();
	void (*unused38)();

	// 32-bit CRC Services
	void (*unused39)();

	// Miscellaneius Services (cont)
	void (*unused40)();
	void (*unused41)();
	void (*unused42)();
};


struct efi_simple_text_output_protocol {
	efi_status (*unused1)(struct efi_simple_text_output_protocol *, bool);

	efi_status (*output_string)(
		struct efi_simple_text_output_protocol *self,
		uint16_t *string);

	efi_status (*unused2)(
		struct efi_simple_text_output_protocol *,
		uint16_t *);
	efi_status (*unused3)(
		struct efi_simple_text_output_protocol *,
		efi_uint_t, efi_uint_t *, efi_uint_t *);
	efi_status (*unused4)(
		struct efi_simple_text_output_protocol *,
		efi_uint_t);
	efi_status (*unused5)(
		struct efi_simple_text_output_protocol *,
		efi_uint_t);

	efi_status (*clear_screen)(
		struct efi_simple_text_output_protocol *self);

	efi_status (*unused6)(
		struct efi_simple_text_output_protocol *,
		efi_uint_t, efi_uint_t);
	efi_status (*unused7)(
		struct efi_simple_text_output_protocol *,
		bool);

	void *unused8;
};


