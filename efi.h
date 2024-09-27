#include <stdint.h>
#include <stdbool.h>

struct efi_table_header {
	uint64_t signature;
	uint32_t revision;
	uint32_t header_size;
	uint32_t crc32;
	uint32_t reserved;
};


struct efi_system_table {
	struct efi_table_header header;
	uint16_t *unused1;//firmware vendor
	uint32_t unused2;//firmware revision
	void *unused3;//console in handle
	void *unused4;//console in
	void *unused5;//console out handle
	struct efi_simple_text_output_protocol *out;//console out
	void *unused6;//standard error handle
	void *unused7;//standard error
	void *unused8;//runtime services
	void *unused9;//boot services
	uint64_t unused10;//number of table entries
	void *unused11;//configuration table
};

typedef void *efi_handle_t;

typedef uint64_t efi_status_t;
typedef efi_status_t efi_status;
typedef uint64_t efi_uint_t;

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


