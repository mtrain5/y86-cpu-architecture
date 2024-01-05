/*
 * CS 261 PA3: Mini-ELF disassembler
 *
 * Name: marcus tran
 */

#include "p3-disas.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

y86_inst_t fetch (y86_t *cpu, byte_t *memory)
{
    if (cpu->pc >= MEMSIZE || cpu->pc < 0 || memory == NULL) {
        cpu->stat = ADR;
        y86_inst_t ins;
        ins.icode = INVALID;
        return ins;
    }

    y86_inst_t ins;
    ins.icode = memory[cpu->pc] >> 4; // opcode
    ins.ifun.b = memory[cpu->pc] & 0x0F;


    switch (ins.icode) {
        case HALT:
            if (ins.ifun.b == 0) {
                ins.valP = cpu->pc + 1;
            } else {
                ins.icode = INVALID;
                cpu->stat = INS;
            }
            break;
        case NOP:
            if (ins.ifun.b == 0) {
                ins.valP = cpu->pc + 1;
            } else {
                ins.icode = INVALID;
                cpu->stat = INS;

            }
            break;
        case CMOV:
            if (cpu->pc + 2 <= MEMSIZE) {
                if (ins.ifun.cmov >= 0 && ins.ifun.cmov <= 6) {

                    ins.icode = CMOV;
                    ins.ra = (memory[cpu->pc + 1] >> 4) & 0x0F; // ra
                    ins.rb = memory[cpu->pc + 1] & 0x0F;         // rb
                    ins.valP = cpu->pc + 2;
                    if (ins.ra < NUMREGS && ins.rb < NUMREGS) {
                        //  ins.valP = cpu->pc + 2;
                    } else {
                        ins.icode = INVALID;
                        cpu->stat = INS;
                    }
                } else {
                    ins.icode = INVALID;
                    cpu->stat = INS;
                }
            } else {
                ins.icode = INVALID;
                cpu->stat = ADR;
            }
            break;

        case IRMOVQ:
            if (ins.ifun.b == 0) {

                ins.icode = IRMOVQ;
                ins.ra = (memory[cpu->pc + 1] >> 4) & 0x0F; //ra
                ins.rb = memory[cpu->pc + 1] & 0x0F;         //rb
                ins.valP = cpu->pc + 10;
                ins.valC.v = *((int64_t*)(memory + cpu->pc + 2));
                if (((memory[cpu->pc +1] & 0XF0) >> 4) != 0X0F) { //f register
                    ins.icode = INVALID;
                    cpu->stat = INS;
                }
            } else {
                ins.icode = INVALID;
                cpu->stat = INS;
            }

            break;
        case RMMOVQ:
            if (cpu->pc + 10 <= MEMSIZE) {
                if (ins.ifun.b == 0) {
                    ins.icode = RMMOVQ;
                    ins.ra = (memory[cpu->pc + 1] >> 4) & 0x0F; //ra
                    ins.rb = memory[cpu->pc + 1] & 0x0F;         //rb
                    ins.valP = cpu->pc + 10;
                    ins.valC.v = *((int64_t*)(memory + cpu->pc + 2));
                } else {
                    ins.icode = INVALID;
                    cpu->stat = INS;
                }
            } else {
                ins.icode = INVALID;
                cpu->stat = ADR;
            }
            break;

        case MRMOVQ:
            if (ins.ifun.b == 0) {
                ins.icode = MRMOVQ;
                ins.ra = (memory[cpu->pc + 1] >> 4) & 0x0F; //ra
                ins.rb = memory[cpu->pc + 1] & 0x0F;         //rb
                ins.valP = cpu->pc + 10;
                ins.valC.v = *((int64_t*)(memory + cpu->pc + 2));
            } else {
                ins.icode = INVALID;
                cpu->stat = INS;
            }

            break;
        case OPQ:
            if (ins.ifun.op >= 0 && ins.ifun.op <= 3) {
                ins.icode = OPQ;
                ins.ra = (memory[cpu->pc + 1] >> 4) %0x0F;
                ins.rb = memory[cpu->pc + 1] & 0x0F;
                ins.valP = cpu->pc + 2;
            } else {
                ins.icode = INVALID;
                cpu->stat = INS;
            }


            break;
        case JUMP:
            if (ins.ifun.jump >= 0 && ins.ifun.jump <= 6) {
                ins.icode = JUMP;
                ins.valP = cpu->pc + 9;
                ins.valC.dest = *((int64_t*)(memory + cpu->pc + 1));
            } else {
                ins.icode = INVALID;
                cpu->stat = INS;
            }

            break;
        case CALL:
            if (cpu->pc + 9 <= MEMSIZE) {
                if (ins.ifun.b == 0) {
                    ins.icode = CALL;
                    ins.valP = cpu->pc + 9;
                    ins.valC.dest = *((int64_t*)(memory + cpu->pc + 1));
                } else {
                    ins.icode = INVALID;
                    cpu->stat = INS;
                }
            } else {
                ins.icode = INVALID;
                cpu->stat = ADR;
            }

            break;
        case RET:
            if (ins.ifun.b == 0) {
                ins.icode = RET;
                ins.valP = cpu->pc + 1;
            } else {
                ins.icode = INVALID;
                cpu->stat = INS;
            }
            break;
        case PUSHQ:
            if (ins.ifun.b == 0) {
                ins.icode = PUSHQ;
                ins.ra = (memory[cpu->pc + 1] >> 4) &0x0F;
                ins.rb = (memory[cpu->pc + 1]) & 0x0F;
                ins.valP = cpu->pc + 2;
                if (ins.ra > 0x07 || ins.rb != 0X0F) { //rb = f
                    ins.icode = INVALID;
                    cpu->stat = INS;
                }
            } else {
                ins.icode = INVALID;
                cpu->stat = INS;
            }
            break;
            break;
        case POPQ:
            if (ins.ifun.b == 0) {
                ins.icode = POPQ;
                ins.ra = (memory[cpu->pc + 1] >> 4) &0x0F;
                ins.rb = (memory[cpu->pc + 1]) & 0x0F;
                ins.valP = cpu->pc + 2;
                if (ins.ra > 0x07 || ins.rb != 0X0F) { //rb = f
                    ins.icode = INVALID;
                    cpu->stat = INS;
                }
            } else {
                ins.icode = INVALID;
                cpu->stat = INS;
            }
            break;
        case IOTRAP:
            if (ins.ifun.trap >= 0 && ins.ifun.trap <= 5) {
                ins.icode = IOTRAP;
                ins.valP = cpu->pc + 1;
            } else {
                ins.icode = INVALID;
                cpu->stat = INS;
            }
            break;
        default:
            ins.icode = INVALID;
            cpu->stat = INS;
            return ins;
    }


    return ins;
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p3 (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
    printf("  -s      Show the program headers\n");
    printf("  -m      Show the memory contents (brief)\n");
    printf("  -M      Show the memory contents (full)\n");
    printf("  -d      Disassemble code contents\n");
    printf("  -D      Disassemble data contents\n");
}

bool parse_command_line_p3 (int argc, char **argv,
                            bool *print_header, bool *print_phdrs,
                            bool *print_membrief, bool *print_memfull,
                            bool *disas_code, bool *disas_data, char **filename)
{
    if (argc <= 1 || argv == NULL || print_header == NULL || print_phdrs == NULL
            || print_membrief == NULL
            || print_memfull == NULL || filename == NULL) {
        return false;
    }
    int c;
    bool membrief = false;
    bool memfull = false;
    while ((c = getopt(argc, argv, "hHafsmMDd")) != -1) {
        switch (c) {
            case 'h':
                usage_p3(argv);
                return false;
            case 'H':
                *print_header = true;
                break;
            case 'a':
                *print_header = true;
                *print_phdrs = true;
                *print_membrief = true;
                membrief = true;
                break;
            case 'f':
                *print_header = true;
                *print_phdrs = true;
                *print_memfull = true;
                memfull = true;
                break;
            case 's':
                *print_phdrs = true;
                break;
            case 'm':
                if (memfull) {
                    usage_p3(argv);
                    return false;  // (both -m and -M)
                }
                *print_membrief = true;
                membrief = true;
                break;
            case 'M':
                if (membrief) {
                    usage_p3(argv);
                    return false;  //(both -m and -M)
                }
                *print_memfull = true;
                memfull = true;
                break;
            case 'D':
                *disas_data = true;
                break;
            case 'd':
                *disas_code = true;
                break;
            default:
                usage_p3(argv);
                return false;
        }

        if (memfull && membrief) {
            usage_p3(argv);
            return false;
        }

    }
    //no filename (or extra output)
    if (optind != argc - 1) {
        usage_p3(argv);
        return false;
    } else {
        *filename = argv[optind];  // Save the filename
        if (*filename == NULL) {
            usage_p3(argv);
            return false;
        }

        return true;

    }
}

void printReg(uint32_t b)
{
    switch (b) {
        case RAX:
            printf("%%rax");
            break;
        case RCX:
            printf("%%rcx");
            break;
        case RDX:
            printf("%%rdx");
            break;
        case RBX:
            printf("%%rbx");
            break;
        case RSP:
            printf("%%rsp");
            break;
        case RBP:
            printf("%%rbp");
            break;
        case RSI:
            printf("%%rsi");
            break;
        case RDI:
            printf("%%rdi");
            break;
        case R8:
            printf("%%r8");
            break;
        case R9:
            printf("%%r9");
            break;
        case R10:
            printf("%%r10");
            break;
        case R11:
            printf("%%r11");
            break;
        case R12:
            printf("%%r12");
            break;
        case R13:
            printf("%%r13");
            break;
        case R14:
            printf("%%r14");
            break;
        case NOREG:
            break;
    }

}

void disassemble (y86_inst_t *inst)
{
    switch(inst->icode) {
        case HALT:
            printf("halt");
            break;
        case NOP:
            printf("nop");
            break;
        case CMOV:
            switch (inst->ifun.cmov) {
                case RRMOVQ:
                    printf("rrmovq ");
                    break;
                case CMOVLE:
                    printf("cmovle ");
                    break;
                case CMOVL:
                    printf("cmovl ");
                    break;
                case CMOVE:
                    printf("cmove ");
                    break;
                case CMOVNE:
                    printf("cmovne ");
                    break;
                case CMOVGE:
                    printf("cmovge ");
                    break;
                case CMOVG:
                    printf("cmovg ");
                    break;
                case BADCMOV:
                    return;

            }
            printReg(inst->ra);
            printf(", ");
            printReg(inst->rb);
            break;
        case IRMOVQ:
            printf("irmovq ");
            printf("%#lx, ", (uint64_t) inst->valC.v);
            printReg(inst->rb);
            break;
        case RMMOVQ:
            printf("rmmovq ");
            printReg(inst->ra);
            if (inst->rb >= 0 && inst-> rb <= 14) {
                printf(", %#lx(", (uint64_t)inst->valC.d);
                printReg(inst->rb);
                printf(")");
            } else {
                printf(", %#lx", (uint64_t)inst->valC.d);
            }
            break;
        case MRMOVQ:
            printf("mrmovq ");
            printf("%#lx", (uint64_t)inst->valC.d);

            if (inst->rb >= 0 && inst->rb <= 14) { //make sure its in registers
                printf("(");
                printReg(inst->rb);
                printf("), ");
            } else {
                printf(", ");
            }

            printReg(inst->ra);
            break;
        case OPQ:
            switch(inst->ifun.op) {
                case ADD:
                    printf("addq ");
                    break;
                case SUB:
                    printf("subq ");
                    break;
                case AND:
                    printf("andq ");
                    break;
                case XOR:
                    printf("xorq ");
                    break;
                case BADOP:
                    return;
            }
            printReg(inst->ra);
            printf(", ");
            printReg(inst->rb);
            break;
        case JUMP:
            switch(inst->ifun.jump) {
                case JMP:
                    printf("jmp ");
                    break;
                case JLE:
                    printf("jle ");
                    break;
                case JL:
                    printf("jl ");
                    break;
                case JE:
                    printf("je ");
                    break;
                case JNE:
                    printf("jne ");
                    break;
                case JGE:
                    printf("jge ");
                    break;
                case JG:
                    printf("jg ");
                    break;
                case BADJUMP:
                    return;

            }
            printf("%#lx", (uint64_t) inst->valC.dest);
            break;
        case CALL:
            printf("call ");
            printf("%#lx", (uint64_t) inst->valC.dest);
            break;
        case RET:
            printf("ret");
            break;
        case PUSHQ:
            printf("pushq ");
            printReg(inst->ra);
            break;
        case POPQ:
            printf("popq ");
            printReg(inst->ra);
            break;
        case IOTRAP:
            switch(inst->ifun.b) {
                case CHAROUT:
                    printf("iotrap 0");
                    break;
                case CHARIN:
                    printf("iotrap 1");
                    break;
                case DECOUT:
                    printf("iotrap 2");
                    break;
                case DECIN:
                    printf("iotrap 3");
                    break;
                case STROUT:
                    printf("iotrap 4");
                    break;
                case FLUSH:
                    printf("iotrap 5");
                    break;
                case BADTRAP:
                    printf("iotrap 1");
                    return;
            }
            break;
        case INVALID:
            break;
    }

}

void disassemble_code (byte_t *memory, elf_phdr_t *phdr, elf_hdr_t *hdr)
{
    if (memory == NULL || phdr == NULL || hdr == NULL) {
        return;
    }
    y86_t cpu;          // CPU struct to store "fake" PC
    y86_inst_t ins;     // struct to hold fetched instruction

    // start at beginning of the segment
    cpu.pc = phdr->p_vaddr;
    int end_of_segment = phdr->p_vaddr +phdr->p_size;
    // int spaces = 31;
    printf("  0x%03lx:%40s%03lx code", (uint64_t) cpu.pc, "| .pos 0x", (uint64_t) cpu.pc);
    printf("\n");
    // iterate through the segment one instruction at a time
    while (cpu.pc < end_of_segment) {
        if(cpu.pc == hdr->e_entry) {
            printf("  0x%03lx:%40s", (uint64_t) cpu.pc, "| _start:");
            printf("\n");
        }

        ins = fetch (&cpu, memory);         // stage 1: fetch instruction

        // TODO: abort with error if instruction was invalid
        if (ins.icode == INVALID) {
            printf("Invalid opcode: %#02x\n", memory[cpu.pc]);
            cpu.pc = ins.valP;
            break;
        }
        // TODO: print current address and raw bytes of instruction
        printf("  0x%03lx: ", (uint64_t) cpu.pc);
        for (int i = cpu.pc; i < ins.valP; i++) {
            printf("%02x ", memory[i]);
        }
        if (ins.icode == HALT || ins.icode == NOP || ins.icode == RET
                || ins.icode == IOTRAP) { //space formatting
            for (int m = 0; m < 27; m++) {
                printf(" ");
            }
        } else if (ins.icode == CMOV || ins.icode == OPQ || ins.icode == PUSHQ || ins.icode == POPQ) {
            for (int m = 0; m < 24; m++) {
                printf(" ");
            }
        } else if (ins.icode == MRMOVQ || ins.icode == IRMOVQ || ins.icode == RMMOVQ) {

        }


        else {
            for (int m = 0; m < 3; m++) {
                printf(" ");

            }

        }
        printf("|   ");
        disassemble (&ins);                 // stage 2: print disassembly
        cpu.pc = ins.valP;
        printf("\n");               // stage 3: update PC (go to next instruction)
    }
    printf("\n");
}

void disassemble_data (byte_t *memory, elf_phdr_t *phdr)
{
    if (memory == NULL || phdr == NULL) {
        return;
    }
    y86_t cpu;
    cpu.pc = phdr->p_vaddr;
    printf("  0x%03lx:", (uint64_t) cpu.pc);
    printf("%40s%03lx data\n","| .pos 0x", (uint64_t) cpu.pc);
    int end_of_segment = phdr->p_vaddr +phdr->p_size;
    while (cpu.pc < end_of_segment) {
        printf("  0x%03lx: ", (uint64_t) cpu.pc); //print address
        for(int i = cpu.pc; i < cpu.pc + 8; i++) { //read  raw byetes
            printf("%02x ", memory[i]);
        }
        printf("%7s","|");
        printf("   .quad ");
        uint64_t *p;
        p = (uint64_t *) &memory[cpu.pc];
        printf("%p", (void *) *p);
        cpu.pc += 8;
        printf("\n");


    }
    printf("\n");
}


void disassemble_rodata (byte_t *memory, elf_phdr_t *phdr)
{
    if (memory == NULL || phdr == NULL) {
        return;

    }
    y86_t cpu;
    cpu.pc = phdr->p_vaddr;
    bool done = false;
    printf("  0x%03lx:", (uint64_t) cpu.pc);
    printf("%40s%03lx rodata\n","| .pos 0x", (uint64_t) cpu.pc); //first line
    int end_of_segment = phdr->p_vaddr + phdr->p_size;
    while (cpu.pc < end_of_segment) {
        done = false;
        printf("  0x%03lx: ", (uint64_t) cpu.pc); //address
        for(int i = cpu.pc; i < cpu.pc + 10; i++) { //first 10 bytes
            printf("%02x ", memory[i]);
            if (memory[i] == 0x00) {
                done = true;
                int spaces = 10 - (i - (cpu.pc - 1));
                for (int m = 0; m < spaces; m++) {
                    printf("   ");
                    // break;
                }
                break;

            }
        }
        int i = cpu.pc;
        printf("|");
        printf("   .string \"");
        while (memory[i] != 0x00) {
            printf("%c", memory[i++]); //string part (hello)
        }

        int num = (i - cpu.pc) + 1;
        printf("\"");
        if (!done) { //Theres still more
            printf("\n");
            i = cpu.pc + 10;
            int j = 0;
            printf("  0x%03lx: ", (uint64_t) i);

            while(memory[i] != 0x00) {
                printf("%02x ", memory[i++]);
                //i++;

                if(++j % 10 == 0) { //formatting (make sure its not over)
                    printf("| \n  0x%03lx: ", (uint64_t) i);
                }
            }
            printf("%02x ", memory[i++]);
            int spaces = 10 - ((j % 10) + 1); //spaces
            for (int x = 0; x < spaces; x++) {
                printf("   ");
            }
            printf("| ");
        }
        cpu.pc += num; //goto next address
        printf("\n");


    }
    printf("\n");

}
