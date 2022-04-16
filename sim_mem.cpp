#include "sim_mem.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;


sim_mem::sim_mem(char exe_file_name[], char swap_file_name[], int text_size, int data_size, int bss_size, int heap_stack_size, int num_of_pages, int page_size){
    //assigning values to variables
    this->text_size = text_size;
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->heap_stack_size = heap_stack_size;
    this->num_of_pages = num_of_pages;
    this->page_size = page_size;
    main_memory[MEMORY_SIZE];//initiating physical memory!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    int i =0;
    for(i=0; i<MEMORY_SIZE;i++){
        main_memory[i] ='0';
    }
    page_descriptor *page_table1 = new  page_descriptor [num_of_pages];//initiating pointer to descriptor structs
    page_table = &page_table1[0];//directs the pointer to point to first struct of type file_descriptor
    descriptorFiller(num_of_pages, page_size, page_table);//creating structs * number of pages inserted
    page_table = &page_table1[0];//after creating all structs pointer is pointing to last struct, redirecting it to point to first struct of type file_descriptor
    mode_t mode1 = O_RDONLY;
    program_fd = open(exe_file_name,O_RDONLY,mode1);//opening virtual memory file for read only
    if( program_fd == -1) { // file couldn't be opened
        perror("unable to open the exe file\n");
        exit(1);
    }
    swapfile_fd = open(swap_file_name, O_RDWR| O_CREAT | O_TRUNC, 0644);//opening swap memory file for read, write and creating
    if(swapfile_fd == -1) { // file couldn't be opened
        perror("unable to open or create the swap file\n");
        exit(1);
    }
    int ret;
    off_t fsize = lseek(program_fd, 0, SEEK_END);//pointing the start to bit 0
    char swap_memory[fsize];
    memset(swap_memory, '0', fsize);//filling swap file with zeros
    if (ret = write(swapfile_fd, swap_memory,fsize)==-1)
    perror("unable to fill the file\n");
    }
sim_mem::~sim_mem(){
    if (close(swapfile_fd) < 0)
    {
        perror("unable to close swap file\n");
        exit(1);
    }
    printf("closed the swap file \n");

    if (close(program_fd) < 0)
    {
        perror("unable to close exe file\n");
        exit(1);
    }
    printf("closed the exe file \n");

    delete(page_table);
}

void sim_mem::descriptorFiller(int num_of_pages, int page_size,page_descriptor *page_table){
    int i = 0;
    for(i=0; i<num_of_pages; i++){
        struct page_descriptor address ;
        address.V= 0;//validity in physical memory
        address.D= 0;//whether the bits saved in swap file or not (dirty or not)
        address.frame= -1;//location of page in physical memory if it is valid
        address.swap_index= -1;//location of page in swap memory if it is dirty
        if(i<text_size/page_size)//permission of text bits - read only
        address.P= 0;//read=0, write=1
        if(i>=text_size/page_size)//permission of the rest of the bits - read and write
            address.P= 1;//read=0, write=1
        page_table[i] = address;

    }
}

int sim_mem::get_page(int address) {//returns page number

        int page = (address / page_size);
        return page;
}

int sim_mem::get_offset(int address){//returns location of address inside the page
    int offset = address%page_size;
    return offset;
}

int sim_mem::old_page_toswap() {
    //searched for a zero in swap file starting at 0 bit
    int i = 0;//loops over swap file
    char buf[1];
    if (lseek(swapfile_fd, 0, SEEK_SET) < 0){
        printf("lseek() failed3 \n");
        return -1;
    }
    if (read(swapfile_fd, buf, 1) != 1){
        perror("unable to read the swap file\n");
     return -1;
    }
    while (buf[0] != '0') {
        read(swapfile_fd, buf, 1);
        i++;
    }
    //checks whether memory_page_queue is empty: meaning no swappable page present in the physical memory
    if (!memory_page_queue.empty()) {
        int page = memory_page_queue.front();//get old page num
        memory_page_queue.pop();//getting page num out f the queue
        //passing the oldest page into buffer

        int k = page_table[page].frame * page_size;
        if(page_table[page].P ==1) {//if not text pass to swap file
            char buffer[page_size];
            int j = 0;
            for (j = 0; j < page_size; j++) {
                buffer[j] = main_memory[k];
                k++;
            }
            //started writing the oldest page(buffer) into swap page
            if (lseek(swapfile_fd, i, SEEK_SET) < 0) {
                printf("lseek() failed 4\n");
                return -1;//started where the swap file has the first zero
            }
            if (write(swapfile_fd, buffer, page_size) != page_size) {
                perror("unable to write in the swap file\n");
                return -1;//added buffer to swap file
            }
            //update file descriptor page
            page_table[page].D = 1;
            page_table[page].V = 0;
            page_table[page].frame = -1;
            page_table[page].swap_index = i;
        }
        else{
            k= k+page_size;
            //update file descriptor page
            page_table[page].D = 0;
            page_table[page].V = 0;
            page_table[page].frame = -1;
            page_table[page].swap_index = -1;
        }
    return k - page_size;//address of oldest page in physical memory
}
    else {//meaning no swappable page present in the physical memory - physical memory is small
        std::cerr << "no enough physical memory to add this page memory is full with text entries which may not be swapped" <<std::endl;
        return 1;
    }
}

char sim_mem::get_from_virtual(int address) {//in case the needed to load bits are 1/invalid and text 2/invalid data and not dirty
    int page = get_page(address);
    int i = 0;
    char result[1] = {'n'};
    //loops over the physical memory to find an empty frame
    for (i = 0; i < sizeof(main_memory); i++) {
        if (main_memory[i] == '0') {//in case found
            //copy the page from virtual memory to buffer  then to physical memory frame
            char buffer[page_size];
            if (lseek(program_fd, page_size * (page), SEEK_SET) < 0){
                printf("lseek() failed 5\n");
                return -1;
            }
            if (read(program_fd, buffer, page_size) != page_size){
                printf("unable to read exe file \n");
                return -1;
            }
            int j = 0;
            for (j = 0; j < page_size; j++) {
                main_memory[i] = buffer[j];
                i++;
            }
            //updating file_descriptor related struct
            page_table[page].V = 1;
            page_table[page].frame = (i - j) / page_size;
            int frame = page_table[page].frame;
            int offset = get_offset(address);
            memory_page_queue.push(page);
            result[0] = 'y';//yes an empty frame has been found
            return main_memory[frame * page_size + offset];
        }
    }
    if (result[0] == 'n'){//in case no empty frame has been found a swap is needed
       int index = old_page_toswap();//index of physical memory where oldest page sit
        //add value to physical memory
        char buffer [page_size];
        if (lseek(program_fd, page_size * (page), SEEK_SET) < 0) {
            printf("lseek() failed 6\n");
            return -1;
        }//start reading where the page starts in virtual file
        if (read(program_fd, buffer, page_size) != page_size){
            printf("unable to read exe file\n");
            return -1;
        } //reads the page into buffer
        int j = 0;
        for (j = 0; j < page_size; j++) {
            main_memory[index] = buffer[j];
           index++;
        }
        int offset = get_offset(address);
        //update file descriptor for page
        page_table[page].V = 1;
        page_table[page].frame = (index-j) / page_size;
        memory_page_queue.push(page);
        return main_memory[page_table[page].frame * page_size + offset];
    }
    else
    return '\0';

}

char sim_mem::get_from_swap(int address){//in case invalid and dirty
    int page = get_page(address);
    int i =0;
    char result[1] = {'n'};
    for(i=0; i<sizeof(main_memory); i++){//search of empty frame in physical memory
        if(main_memory[i]=='0'){//in case found
            //read the page from swap file and add it to physical memory
            char buffer[page_size];
            if(lseek(swapfile_fd,page_table[page].swap_index,SEEK_SET)<0){
                printf("unable to lseak() swap file \n");
                return -1;
            }
            if(read(swapfile_fd,buffer,page_size)!= page_size) {
                printf("unable to read swap file\n");
                return -1;
            }
            int j=0;
            for(j=0;j<page_size;j++){
                main_memory[i]= buffer[j];
                buffer[j] = 0;
                i++;
            }
            //fill zeros instead of the page read from swap file
            if(write(swapfile_fd,buffer,page_size)!= page_size) {
                printf("unable to write to swap file\n");
                return -1;
            }
            //update related struct
            page_table[page].V=1;
            page_table[page].swap_index =-1;
            page_table[page].D =0;
            page_table[page].frame=(i-j)/page_size;
            int frame = page_table[page].frame;
            int offset= get_offset(address);
            memory_page_queue.push(page);//add it to page queue
            result[0] = 'y';//yes empty memory frame found
            return main_memory[frame*page_size+offset];
        }
    }
    if (result[0] == 'n') {//no empty frame found, a swaP IS needed
        int index = old_page_toswap();//get the physical address of the oldest page which is not text
        //read the page from swap into buffer and add it into the frame
        char buffer[page_size];
        if(lseek(swapfile_fd,page_table[page].swap_index,SEEK_SET)<0){
            printf("unable to lseak swap file \n");
            return -1;
        }
        if(read(swapfile_fd,buffer,page_size)!= page_size) {
            printf("unable to read swap file\n");
            return -1;
        }
        int j=0;
        for(j=0;j<page_size;j++){
            main_memory[index]= buffer[j];
            buffer[j] = 0;
            index++;
        }
        if(write(swapfile_fd,buffer,page_size)!= page_size) {
            printf("unable to write to swap file\n");
            return -1;
        }//replace the page in swap file with zeros
        //update related file descriptor
        page_table[page].V=1;
        page_table[page].D = 0;
        page_table[page].swap_index =-1;
        page_table[page].frame=(index-j)/page_size;
        int frame = page_table[page].frame;
        int offset= get_offset(address);
         memory_page_queue.push(page);
        return main_memory[frame*page_size+offset];
    }
    return 1;
}

char sim_mem::load(int address){
    int page = get_page(address);
    if(address > text_size+bss_size+data_size+heap_stack_size) std::cerr << "page number "<<page<< " out of range" <<std::endl;//if address is out of range
    if(page_table[page].V==1){//valid pages, simply loaded from physical memory
        int frame = page_table[page].frame;
        int offset= get_offset(address);
        return main_memory[frame*page_size+offset];
    }
    else {//not valid
        if (page_table[page].P == 0 )//loading invalid text pages from virtual memory
            return get_from_virtual(address);

        else if (page_table[page].P == 1 && page_table[page].D == 1) //loading invalid non text dirty pages from swap memory
        return get_from_swap(address);

        else if (page_table[page].P ==1 && page_table[page].D ==0) {//loading invalid non dirty pages

            if (address > text_size && address <=text_size + data_size)//if data  from virtual memory
                return get_from_virtual(address);

            else if (address > text_size + data_size && address <= text_size + data_size+ bss_size){//bss adding zero to oldest filled frame
                char result[1] = {'n'};
                int i =0;
                for(i=0; i<sizeof(main_memory); i++) {
                    if (main_memory[i] == '0') {
                        int j=0;
                        for(j=0;j<page_size;j++){
                            main_memory[i]= '0';
                            i++;
                        }
                        memory_page_queue.push(page);//add it to page queue
                        result[0] = 'y';
                        //update related struct
                        page_table[page].V=1;
                        page_table[page].swap_index =-1;
                        page_table[page].D =0;
                        page_table[page].frame=(i-j)/page_size;
                        break;
                    }
                }
                if (result[0] == 'n') {
                    int index = old_page_toswap();//get the physical address of the oldest page which is not text
                    int j=0;
                    for(j=0;j<page_size;j++){
                        main_memory[index]= '0';
                        index++;
                    }
                    //update related file descriptor
                    page_table[page].V=1;
                    page_table[page].D = 0;
                    page_table[page].swap_index =-1;
                    page_table[page].frame=(index-j)/page_size;
                    memory_page_queue.push(page);//add it to page queue
                }

            }
            else if (address > text_size + data_size+bss_size &&
                address <=text_size + heap_stack_size+ data_size+ bss_size) {//if heap or stack or bss, unable to read since invalid
                std::cerr << "error, cant read null memory for page "<< page <<std::endl;
            return '\0';
           }
        }
    }
    return '\0';
}

void sim_mem::store(int address, char value){//only for p=0 addresses
    int page = get_page(address);
    if(address > text_size+bss_size+data_size+heap_stack_size) std::cerr << "page number "<<page<<" out of range"<<std::endl;//pages out of range
    else if(page_table[page].V==1 && page_table[page].P==1){//valid && writable
        int frame = page_table[page].frame;
        int offset= get_offset(address);
        main_memory[frame*page_size+offset] = value;
    }
    else if (page_table[page].V==0 && page_table[page].P==1 && page_table[page].D==0 && address > text_size && address <= text_size+data_size){//invalid && writable && not dirty store from virtual (data)
        store_from_virtual(address, value);
    }
    else if (page_table[page].V==0 && page_table[page].P==1 && page_table[page].D==0 && address > text_size+data_size){//invalid && writable && not dirty store new page (bss/stack/heap)

        int i = newpage(address,value);//creating new page with added value into it
        //updating file descriptor related struct
        page_table[page].V = 1;
        page_table[page].frame = (i - page_size) / page_size;
         memory_page_queue.push(page);//add to the queue of added pages to the physical memory

    }
    else if (page_table[page].V==0 && page_table[page].P==1 && page_table[page].D==1) {//invalid && writable && dirty  from swap file
        store_from_swap(address, value);
    }
    else std::cerr << "page "<<page<<"  is not available for writing"<<std::endl;//if text page is inserted
}

int sim_mem::newpage(int address , int value){//creates new page for heap, stack and bss and adds value to it

    int i =0;
    char result[1] = {'n'};
    int offset = get_offset(address);
    int j = 0;
    for (i = 0; i < sizeof(main_memory); i++) {//search of empty memory frame
        if (main_memory[i] == '0') {//in case found adding the page to the memory
            for (j = 0; j < page_size; j++) {
                if(offset == j) main_memory[i]=value;
                else main_memory[i] = '0';
                i++;
            }
            result[0] = 'y';//yes found
        }
        break;
    }
    if (result[0] == 'n'){//no empty frame found a swap is needed
        int index = old_page_toswap();//index of physical memory where oldest page has been
        //add value to physical memory
        int j = 0;
        for (j = 0; j < page_size; j++) {
            if(offset == j) main_memory[index]=value;
            else main_memory[index] = '0';
            index++;
        }
        return index;
    }
    return i;
}

void sim_mem::store_from_swap(int address, char value){//in case storing dirty & invalid (not text)
    int page = get_page(address);
    int i =0;
    char result[1] = {'n'};
    for(i=0; i<sizeof(main_memory); i++){//search for empty frame
        if(main_memory[i]=='0'){//found
            //read the page from swp and add to memory
            char buffer[page_size];
            if(lseek(swapfile_fd,page_table[page].swap_index,SEEK_SET)<0){
                printf("unable to lseak swap file \n");
                exit(1);
            }
            if(read(swapfile_fd,buffer,page_size)!= page_size){
                printf("unable to read swap file\n");
                exit(1);
            }
            int j=0;
            for(j=0;j<page_size;j++){
                main_memory[i]= buffer[j];
                buffer[j] = 0;
                i++;
            }
            //replace page with zeros in swap file
            if(write(swapfile_fd,buffer,page_size)!= page_size) {
                printf("unable to write to swap file \n");
                exit(1);
            }
            //file descriptor update
            page_table[page].V=1;
            page_table[page].D=0;
            page_table[page].swap_index =-1;
            page_table[page].frame=(i-j)/page_size;
            int frame = page_table[page].frame;
            int offset= get_offset(address);
            memory_page_queue.push(page);
            result[0] = 'y';//a sign empty frame found
            main_memory[frame*page_size+offset] = value;
            break;
        }
    }
    if (result[0] == 'n') {//no empty frame found swap needed
        int index = old_page_toswap();//physical address of oldest page
        //needed page read from swap file and stored into physical memory
        char buffer[page_size];
        if(lseek(swapfile_fd,page_table[page].swap_index,SEEK_SET)<0){
            printf("unable to lseak swap file \n");
            exit(1);
        }
        if(read(swapfile_fd,buffer,page_size)!= page_size) {
            printf("unable to read swap file\n");
            exit(1);
        }
        int j=0;
        for(j=0;j<page_size;j++){
            main_memory[index]= buffer[j];
            index++;
        }
        //page replaced with zeros in swap file
        char buf [page_size];
        int i =0;
        for(i=0; i<page_size;i++){
            buf[i] = '0';
        }
        if(lseek(swapfile_fd,page_table[page].swap_index,SEEK_SET)<0){
            printf("unable to lseak swap file \n");
            exit(1);
        }
        if(write(swapfile_fd,buf,page_size)!= page_size) {
            printf("unable to write to swap file\n");
            exit(1);
        }
        //file descriptor update
        page_table[page].V=1;
        page_table[page].D=0;
        page_table[page].swap_index =-1;
        page_table[page].frame=(index-j)/page_size;
        int frame = page_table[page].frame;
        int offset= get_offset(address);
        memory_page_queue.push(page);//pushed to queue of added pages to physical memory
        main_memory[frame*page_size+offset]  = value;
    }
}

void sim_mem::store_from_virtual(int address , char value) {//in case storing invalid , not dirty not text
    int page = get_page(address);
    int i = 0;
    char result[1] = {'n'};
    for (i = 0; i < sizeof(main_memory); i++) {//search for empty frame
        if (main_memory[i] == '0') {//empty frame found
            //read from virtual memory into physical memory
            char buffer[page_size];
            if (lseek(program_fd, page_size * (page), SEEK_SET) < 0){
                printf("lseek() failed7\n");
                exit(1);
            }
            if (read(program_fd, buffer, page_size) != page_size){
                printf("unable to read exe file\n");
                exit(1);
            }
            int j = 0;
            for (j = 0; j < page_size; j++) {
                main_memory[i] = buffer[j];
                i++;
            }
            //file descriptor update
            page_table[page].V = 1;
            page_table[page].frame = (i - j) / page_size;
            int frame = page_table[page].frame;
            int offset = get_offset(address);
            memory_page_queue.push(page);//added to queue
            result[0] = 'y';
            main_memory[frame * page_size + offset] = value;
            break;
        }
    }
    if (result[0] == 'n'){//no free frame found
        int index = old_page_toswap();

        //add value to physical memory
        char buffer [page_size];
        if (lseek(program_fd, page_size * (page), SEEK_SET) < 0) {
            printf("lseek() failed1\n");
            exit(1);//start reading where the page starts in virtual file
        }
        if (read(program_fd, buffer, page_size) != page_size){
            printf("unable to read exe file\n");
            exit(1);
        } //reads the page into buffer
        int j = 0;
        for (j = 0; j < page_size; j++) {
            main_memory[index] = buffer[j];
            index++;
        }
        int offset = get_offset(address);
        //update file descriptor for page
        page_table[page].V = 1;
        page_table[page].frame = (index-j) / page_size;
        memory_page_queue.push(page);
        main_memory[page_table[page].frame * page_size + offset] = value;
    }
}

void sim_mem::print_page_table() { //prints out the table of structs created by descriptorFiller (given function)
    int i;
    printf("\n page table \n");
    printf("Valid\t Dirty\t Permission \t Frame\t Swap index\n");
    for (i = 0; i <num_of_pages ;i++) {
        printf("[%d]\t[%d]\t[%d]\t\t[%d]\t[%d]\n",
               page_table[i].V,
               page_table[i].D,
               page_table[i].P,
               page_table[i].frame,
               page_table[i].swap_index);
    }
}

void sim_mem::print_memory() {//prints out the physical memory (given)
    int i;
    printf("\n Physical memory\n");
    for(i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\n", main_memory[i]);
    }
}

void sim_mem::print_swap() {//prints out the bits saved in swap file (given)
    char* str = (char*)malloc(this->page_size *sizeof(char));
    int i;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while(read(swapfile_fd, str, this->page_size) == this->page_size) {
        for(i = 0; i < page_size; i++) {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }
    delete(str);
}