# Virtual-Memory-Management
==Description==

==Description of Virtual-Memory-Management.c==
the program is a simulation of virtual memory management, it deals with requests of storing and loading of bits (text, data, swap, heap and bss).
and bit stored or loaded shall be present in physical memory though different functions depending on its status in file descriptor. 
the program prints the physical memory, swap memory and file descriptor after load and stor requests are done.


==functions== 
    void descriptorFiller(int num_of_pages, int page_size, page_descriptor *page_table );//creates structs of kind"page_descriptore" and arranges them in a table
    char load(int address);//returns the bit which sits in the address inserted to do so it checks whether the page is valid or invalid in physical memory dependind on the status of the page the function gets the bits from swap file, physical memory or exe file stores it in physical memory, if not there, then returns the needed content of the address
    void print_memory();//prints out the physical memory
    void print_swap ();//prints out the bits saved in swap file
    void print_page_table();//prints out the table of structs created by descriptorFiller
    int newpage(int address, int value);//creates new page in case of load() or store() an invalid and not dirty bss, heap or stack pages
    int get_page(int address);//returns page number
    int get_offset(int address);//returns location of address inside the page
    char get_from_virtual(int address);//gets pages from virtual memory to save it in physical memory in order to load it after wise.
    char get_from_swap(int address);//gets pages from swap memory, and replaces it with zeros to save it in physical memory in order to load it after wise.
    int old_page_toswap();//returns the address of the oldest page in the physical memory after saving it in swap file
    void store(int address, char value);//inserts the input value inside the inserted address after checking if page is valid in physical memory, the function gets the bits from swap file or exe file depending on its status, adds the value to the address and adds the page to the physical memory.
    void store_from_virtual(int address, char value);//get page from virtual memory, add it to physical memory and returns the address
    void store_from_swap(int address, char value);//get page from swapfile, add to physical memory and stores address

==files used== 
exec_file: the file which contains bits of text, data, bss, swap and heap 

==how to compile== 
 g++ -o Virtual-Memory-Management  main.cpp sim_mem.cpp
 
to run: ./Virtual-Memory-Management

==input== 
exe_file_name, swap_file_name, int text_size, int data_size, int bss_size, int heap_stack_size, int num_of_pages, int page_size for constructor

int address for load 

int address and value for store 
==output=== 

a print out of : 

physical memory 

swap memory 

file descriptor 
