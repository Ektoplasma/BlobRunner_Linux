/*

	Linux fork - ekt0

 */

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

// Define bool
typedef int bool;
#define true 1
#define false 0

const char *_version = "0.0.3-linux";

const char *_banner = " __________.__        ___.  __________\n"
    			   " \\______   \\  |   ____\\_ |__\\______   \\__ __  ____   ____   ___________        \n"
    			   "  |    |  _/  |  /  _ \\| __ \\|       _/  |  \\/    \\ /    \\_/ __ \\_  __ \\     \n"
                   "  |    |   \\  |_(  <_> ) \\_\\ \\    |   \\  |  /   |  \\   |  \\  ___/|  | \\/    \n"
                   "  |______  /____/\\____/|___  /____|_  /____/|___|  /___|  /\\___  >__|             \n"
                   "         \\/                \\/       \\/           \\/     \\/     \\/           \n\n"
                   "                                                               \033[92m %s\033[0m \n\n";

void banner(){
    system("clear");
    printf(_banner, _version);
    return;
}

void* process_file(char* inputfile_name, bool autobreak, int offset, bool debug){
	void* lpvBase;
	FILE *file;
	unsigned long fileLen;
	char *buffer;

	file = fopen(inputfile_name, "rb");

	if (!file){
		printf(" [!] Error: Unable to open %s\n", inputfile_name);

		return (void*)NULL;
	}

	printf (" [*] Reading file...\n");	
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file); //Get Length

	printf (" [*] File Size: 0x%04x\n", fileLen);
	fseek(file, 0, SEEK_SET); //Reset

	if(autobreak)
		fileLen+=2;
	else
		fileLen+=1;
	
	buffer=(char *)malloc(fileLen); //Create Buffer

	if(autobreak){
		if(offset == 0){
			buffer[0] = 0xCC;
			fread(buffer+1, fileLen, 1, file);
		}
		else{
			buffer[offset] = 0xCC;
			fread(buffer, offset, 1, file);
			fread(buffer+offset+1, fileLen, 1, file);
		}
	}
	else{
		fread(buffer, fileLen, 1, file);	
	}
	
	fclose(file);                     
	
	printf (" [*] Allocating Memory...");	

	lpvBase = mmap(NULL, 2048, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_ANON|MAP_PRIVATE, 0, 0);

	if(lpvBase == MAP_FAILED){
		printf("Error MMAP\n");
		exit(1);
	}

	printf(".Allocated!\n");
	printf(" [*]   |-Base: %p\n",lpvBase);
	printf(" [*] Copying input data...\n");	

	memcpy(lpvBase, buffer, fileLen);
	return lpvBase;
}

void execute(void* base, int offset, bool nopause, bool autobreak, bool debug)
{
	void* entry;

	entry = (void*)((long long)base + offset);


    printf(" [*] Entry: %p\n",entry);
    if(nopause == false){
	    printf(" [*] Navigate to the EP and set a breakpoint. Then press any key to jump to the shellcode.\n");
	    getchar();
	}
	else{
	    printf(" [*] Jumping to shellcode\n");
	}
    (*(void(*)()) entry)();

}

int main(int argc, char* argv[])
{
    void* base;
	int i;
	int offset = 0;
	bool nopause = false;
	bool debug = false;
	bool autobreak = false;
	char *nptr;

	banner();

	if(argc < 2){
		printf(" \033[31m [!] Error: \033[0m No file!\n\n");
		printf("     Required args: <inputfile>\n\n");
		printf("     Optional Args:\n");
		printf("                     --offset <offset> The offset to jump into.\n");
		printf("                     --nopause         Don't pause before jumping to shellcode. Danger!!! \n");
		printf("                     --autobreak       Insert a breakpoint at the offset. (Default: 0)\n");
		printf("                     --debug           Verbose logging.\n");
		printf("                     --version         Print version and exit.\n\n");
		return -1;
	}

	printf(" [*] Using file: %s \n", argv[1]);

	for(i=2; i < argc; i++){
		if(strcmp(argv[i], "--offset") == 0){
			printf(" [*] Parsing offset...\n");
			i=i+1;
			offset = strtol(argv[i], &nptr, 16);
		}
		else if(strcmp(argv[i], "--nopause") == 0){
			nopause = true;
		}
		else if(strcmp(argv[i], "--autobreak") == 0){
			autobreak = true;
			nopause = true; 
		}
		else if(strcmp(argv[i],"--debug") == 0){
			debug = true;
		}
		else if(strcmp(argv[i],"--version") == 0){
			printf("Version: %s", _version);
		}
		else{
			printf("\033[93m [!] Warning: \033[0mUnknown arg: %s\n",argv[i]);
		}
	}

	base = process_file(argv[1], autobreak, offset, debug);
	if (base == NULL){
		printf(" [!] Exiting...");
		return -1;
	}
	printf(" [*] Using offset: 0x%08x\n", offset);
	execute(base, offset, nopause, autobreak, debug);
	printf ("Pausing - Press any key to quit.\n");
	getchar();
	return 0;
}