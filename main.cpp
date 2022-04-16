
#include "sim_mem.h"
#include <stdio.h>
char main_memory[MEMORY_SIZE];

int main() {
//    char val;
//    sim_mem mem_sm ((char*)"exec_file", (char*)"swap_file" ,25, 50, 25,25, 25, 5);
//     mem_sm.store( 98,'X');
//     mem_sm.load ( 30);
// mem_sm.print_memory(); mem_sm.print_swap();
//    mem_sm.print_page_table();
//    mem_sm.print_memory();
//
//}
sim_mem mem_sm((char*) "exec_file", (char*) "swap_file" ,25, 50, 25,25, 25, 5);

mem_sm.load(5);
mem_sm.load(10);
mem_sm.load(15);
mem_sm.load(20);
mem_sm.load(25);
mem_sm.load(30);
mem_sm.store(31,'y');
mem_sm.load(1);
mem_sm.load(40);
mem_sm.load(45);
mem_sm.load(1);
mem_sm.load(50);
mem_sm.load(30);
mem_sm.load(90);
    mem_sm.store(90, 's');
    mem_sm.load(80);
mem_sm.store(80,'l');
    mem_sm.store(200, '&');//out of range page
    mem_sm.store(100,'d');
    mem_sm.store(106,'f');
    mem_sm.store(130,'t');//out of range page
    mem_sm.load(150);
    mem_sm.store(32,'y');
    mem_sm.load(30);
    mem_sm.load(10);


mem_sm.print_page_table();
mem_sm.print_swap();
mem_sm.print_memory();

return 0;
}