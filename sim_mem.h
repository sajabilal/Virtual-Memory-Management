#ifndef SIM_MEM_H_INCLUDED
#define SIM_MEM_H_INCLUDED

#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <queue>

#define MEMORY_SIZE 25
extern char main_memory[MEMORY_SIZE];

using namespace std;
typedef struct page_descriptor//a given struct which describes properties of each page
{
    int V; // valid
    int D; // dirty
    int P; // permission
    int frame; //the number of a frame if in case it is page-mapped
    int swap_index; // where the page is located in the swap file.
} page_descriptor;

class sim_mem {
    int swapfile_fd;//swap file fd
    int program_fd;//executable file fd - virtual mem
    int text_size;//text bits number
    int data_size;//data bits number
    int bss_size;//bss bits number
    int heap_stack_size;//heap and stack bits number
    int num_of_pages;
    int page_size;
    page_descriptor *page_table;       //pointer to page table
    queue<int> memory_page_queue;//pages queue as inserted into the physical memory
public:


    sim_mem(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_size, int heap_stack_size, int num_of_pages, int page_size);//constructor
    ~sim_mem();//closes files !!!!!!!!!

//declarations of functions

    void descriptorFiller(int num_of_pages, int page_size, page_descriptor *page_table );//creates structs of kind"page_descriptore" and arranges them in a table
    char load(int address);//returns the bit which sits in the address inserted to do so it checks whether the page is valid or invalid in physical memory - for more info check README file
    void print_memory();//prints out the physical memory
    void print_swap ();//prints out the bits saved in swap file
    void print_page_table();//prints out the table of structs created by descriptorFiller
    int newpage(int address, int value);//creates new page in case of load() or store() an invalid and not dirty bss, heap or stack pages
    int get_page(int address);//returns page number
    int get_offset(int address);//returns location of address inside the page
    char get_from_virtual(int address);//gets pages from virtual memory to save it in physical memory in order to load it after wise.
    char get_from_swap(int address);//gets pages from swap memory, and replaces it with zeros to save it in physical memory in order to load it after wise.
    int old_page_toswap();//returns the address of the oldest page in the physical memory after saving it in swap file
    void store(int address, char value);//inserts the input value inside the inserted address after checking if page is valid in physical memory - for more info check README file
    void store_from_virtual(int address, char value);//get page from virtual memory, add it to physical memory and returns the address
    void store_from_swap(int address, char value);//get page from swapfile, add to physical memory and stores address
};
#endif
