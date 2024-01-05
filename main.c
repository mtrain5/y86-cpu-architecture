/*
 * CS 261: Main driver
 *
 * Name: marcus tran
 */

#include "p1-check.h"
#include "p2-load.h"
#include "p3-disas.h"
#include "p4-interp.h"

int main (int argc, char **argv)
{
    bool print_header = false;
    bool print_phdrs = false;
    bool print_membrief = false;
    bool print_memfull = false;
    char *filename = NULL;
    bool disas_data = false;
    bool disas_code = false;
    bool exec_norm = false;
    bool exec_debug = false;
    bool parse = parse_command_line_p4(argc, argv, &print_header, &print_phdrs, &print_membrief,
                                       &print_memfull, &disas_code, &disas_data, &exec_norm, &exec_debug, &filename);

    if (!parse) {
        return EXIT_FAILURE;
    }
    if (filename == NULL) { //error checks
        printf("Failed to read file\n");
        return EXIT_FAILURE;
    }

    FILE *file = fopen(filename, "rb");
    if (file == NULL) { //bad file name and bad phdr test
        printf("Failed to read file\n");
        return EXIT_FAILURE;
    }
    elf_hdr_t head;
    bool read = read_header(file, &head);
    if (!read) {
        printf("Failed to read file\n");
        fclose(file);
        return EXIT_FAILURE;
    }
    elf_phdr_t phead[head.e_num_phdr]; //put it outside so that it gets the value

    for (uint16_t i = 0; i < head.e_num_phdr; i++) {
        if (!read_phdr(file, head.e_phdr_start + (i * sizeof(elf_phdr_t)), &phead[i])) {
            printf("Failed to read file\n");
            fclose(file);
            return EXIT_FAILURE;
        }
    }

    byte_t *mem = (byte_t *)calloc(MEMSIZE,
                                   sizeof(byte_t)); //fixes the unitialized value by using calloc
    if (mem == NULL) {
        fclose(file);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < head.e_num_phdr; i++) {
        if(!load_segment(file, mem, &phead[i])) {
            printf("Failed to read file\n");
            free(mem);
            return EXIT_FAILURE;

        }

    }
    if (print_header) { //-H
        dump_header(&head);
    }

    if (print_phdrs) { //-s
        dump_phdrs(head.e_num_phdr, phead);

    }
    if (print_membrief) { // - m
        for (int i = 0; i < head.e_num_phdr; i++) {
            int start = phead[i].p_vaddr;
            int end = start + phead[i].p_size;

            dump_memory(mem, start, end);
        }
    }


    if (print_memfull) { // -M
        dump_memory(mem, 0, MEMSIZE);
    }
    if (disas_code) { //-d
        printf("Disassembly of executable contents:\n");
        for (int i = 0; i < head.e_num_phdr; i++) {
            if(phead[i].p_type == CODE) {
                disassemble_code(mem, &phead[i], &head);
            }
        }

    }

    if (disas_data) { //-D
        printf("Disassembly of data contents:\n");
        for (int i = 0; i < head.e_num_phdr; i++) {
            if(phead[i].p_type == DATA ) {
                if(phead[i].p_flags == 4) {
                    disassemble_rodata(mem, &phead[i]);//read only
                } else if(phead[i].p_flags == 6) { //read and write
                    disassemble_data(mem, &phead[i]);
                }


            }

        }
    }
    //stuff to make p4-interp.c methods work
    y86_t cpu;
    int count = 0; //instruction count
    memset(&cpu, 0x00, sizeof(cpu)); //initialize cpu
    cpu.stat = AOK; //set it to ok
    y86_reg_t valA = 0;
    y86_reg_t valE = 0;
    bool cnd = false;
    cpu.pc = head.e_entry;//where to start

    if(exec_norm) { //-e
        printf("Beginning execution at 0x%04x\n", head.e_entry);
        while(cpu.stat == AOK) { //keep going as long as cpu says its okay
            y86_inst_t ins = fetch(&cpu, mem);
            if(cpu.stat == INS || cpu.stat == ADR) {
                break; //do break instead of exit_failure because then badinsn wont work.
            } else {
                cnd = false;
                valE = decode_execute(&cpu, ins, &cnd, &valA);
                memory_wb_pc(&cpu, ins, mem, cnd, valA, valE);
                count++;
            }
            if(cpu.pc >= MEMSIZE) { //error check
                cpu.stat = ADR;
            }
        }
        dump_cpu_state(&cpu);
        printf("Total execution count: %d\n", count);
    }

    if(exec_debug) { //-E
        printf("Beginning execution at 0x%04x\n", head.e_entry);
        while(cpu.stat == AOK) {
            dump_cpu_state(&cpu);//all of them

            y86_inst_t ins = fetch(&cpu, mem);
            printf("\n");
            if(cpu.stat == INS || cpu.stat == ADR) {
                printf("Invalid instruction at 0x%04lx\n", cpu.pc); //trace mode enabled so print this instead
            } else {
                printf("Executing: "); //printing each instruction
                disassemble(&ins); //the actual instruction
                printf("\n"); //formatting
                cnd = false;
                valE = decode_execute(&cpu, ins, &cnd, &valA);
                memory_wb_pc(&cpu, ins, mem, cnd, valA, valE);
                count++;
            }

            if(cpu.pc >= MEMSIZE) { //error check
                cpu.stat = ADR;
            }
        }
        dump_cpu_state(&cpu);
        printf("Total execution count: %d\n", count);
        printf("\n"); //format line
        dump_memory(mem, 0x00, 0xff0 + 16);

    }
    // cleanup
    fclose(file);
    free(mem);
    return EXIT_SUCCESS;
}

