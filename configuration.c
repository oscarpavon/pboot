#include "efi.h"
#include "menu.h"
#include "utils.h"
#include "pboot.h"
#include "files.h"

void parse_configuration(uint64_t config_file_size, char* config){

	void* unicode_config;
	allocate_memory(config_file_size, &unicode_config);


	//fill memory by null character
	for(int i = 0; i < config_file_size; i++){
		short* data = (short*)unicode_config;
		char zero = '\0';
		data[i] = (short)zero;
	}
	
	//convert file to unicode 16
	for(int i = 0; i < config_file_size-1; i++){
		short* data = (short*)unicode_config;
		data[i] = (short)config[i];
	}


	uint16_t* new_config = (uint16_t*)unicode_config;

	//log(unicode_config);

	SystemTable* system_table = get_system_table();
	system_table->out->output_string(system_table->out,unicode_config);
	log(u"configuration loaded");
	//hang();
}

void load_configuration(){
	FileProtocol* config_file;
	open_file(&config_file, u"pboot.conf");
	char* config = read_file(config_file);

	while(*config){
		if(*config == 'm'){
			config++;
			config++;
			if(*config == '1'){
				set_show_menu(true);	
			}else if(*config == '0'){
				set_show_menu(false);
			}
		}else if(*config == 'e'){
			config++;
			config++;
			uint8_t entry = *config - '0';

			set_default_entry(entry);
		}
		config++;
	}

}
