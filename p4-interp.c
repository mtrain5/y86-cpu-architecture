/*
 * CS 261 PA4: Mini-ELF interpreter
 *
 * Name: marcus tran
 */

#include "p4-interp.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

bool x(flag_t f1, flag_t f2)
{
    return((f1 && !f2) || (!f1 && f2));
}

bool getJump(y86_jump_t jum, y86_t *cpu)
{
    bool jmp = false;
    switch(jum) {
        case JMP:
            jmp = true;
            break;
        case JLE:
            if(cpu->zf || x(cpu->sf, cpu->of)) {
                jmp = true;
            }
            break;
        case JL:
            if(x(cpu->sf, cpu->of)) {
                jmp = true;
            }
            break;
        case JE:
            if(cpu->zf) {
                jmp = true;
            }
            break;
        case JNE:
            if(!cpu->zf) {
                jmp = true;
            }
            break;
        case JGE:
            if(cpu->sf == cpu->of) {
                jmp = true;
            }
            break;
        case JG:
            if(!cpu->zf && cpu->sf == cpu->of) {
                jmp = true;
            }
            break;
        case BADJUMP:
            cpu->stat = INS;
            break;
    }
    return jmp;
}

bool getCnd(y86_cmov_t mov, y86_t *cpu)
{
    //only for cmov and rrmovq
    bool cnd = false;
    switch(mov) {
        case RRMOVQ:
            cnd = true;
            break;
        case CMOVLE:
            if(cpu->zf || x(cpu->sf, cpu->of)) {
                cnd = true;
            }
            break;
        case CMOVL:
            if(x(cpu->sf, cpu->of)) {
                cnd = true;
            }
            break;
        case CMOVE:
            if(cpu->zf) {
                cnd = true;
            }
            break;
        case CMOVNE:
            if(!cpu->zf) {
                cnd = true;
            }
            break;
        case CMOVGE:
            if(cpu->sf == cpu->of) {
                cnd = true;
            }
            break;
        case CMOVG:
            if(!cpu->zf && cpu->sf == cpu->of) {
                cnd = true;
            }
            break;
        case BADCMOV:
            cpu->stat = INS;
            break;

    }
    return cnd;
}

void handle_iotrap(y86_t *cpu, y86_inst_t inst, byte_t *memory, char *buffer)
{
    int64_t value;

    switch (inst.ifun.trap) {
        case 0: // charout
            snprintf(buffer + strlen(buffer), 2, "%c", (char)memory[cpu->reg[RSI]]);
            break;
        case 1: // charin
            if (scanf("%c", (char*)&memory[cpu->reg[RDI]]) != 1) {
                printf("I/O Error\n");
                cpu->stat = HLT;
            }
            break;
        case 2: // decout
            value = *((int64_t*)&memory[cpu->reg[RSI]]);
            snprintf(buffer + strlen(buffer), 100, "%ld", value);
            break;
        case 3: // decin
            if (scanf("%ld", &value) != 1) {
                printf("I/O Error\n");
                cpu->stat = HLT;
            } else {
                *((int64_t*)&memory[cpu->reg[RDI]]) = value;
            }
            break;
        case 4: // strout
            snprintf(buffer + strlen(buffer), 100, "%s", (char*)&memory[cpu->reg[RSI]]);
            break;
        case 5: // flush
            printf("%s", buffer);
            fflush(stdout);
            break;
        default: // badtrap
            printf("I/O Error\n");
            cpu->stat = HLT;
            break;
    }
}


y86_reg_t decode_execute (y86_t *cpu, y86_inst_t inst, bool *cnd, y86_reg_t *valA)
{
    if(valA == NULL || cnd == NULL) {
        cpu->stat = INS;
    }
    if (cpu->pc >= MEMSIZE || cpu->pc < 0) {
        cpu->stat = ADR;
    }
    y86_reg_t valE = 0;
    y86_reg_t valB;
    switch(inst.icode) {
        case HALT:
            cpu->stat = HLT;
            //  cpu->pc = inst.valP;
            break;
        case NOP:
            break;
        case CMOV:
            //according to cheat sheet valE = valA, valA = register of Ra
            //cnd = cond(cc, ifun)
            //cnd ? r[rb] <- valE
            //*valA = getRegister(inst.ra, cpu);
            *valA = cpu->reg[inst.ra];
            *cnd = getCnd(inst.ifun.cmov, cpu);
            valB = cpu->reg[inst.rb];
            valE = *valA;
            break;
        case IRMOVQ:
            valE = (uint64_t) inst.valC.v;
            break;
        case RMMOVQ:
            *valA = cpu->reg[inst.ra];
            valB = cpu->reg[inst.rb];
            valE = (uint64_t) inst.valC.d + valB;
            break;
        case MRMOVQ:
            *valA = cpu->reg[inst.ra];
            valB = cpu->reg[inst.rb];
            valE = (uint64_t) inst.valC.d + valB;
            break;
        case OPQ:
            *valA = cpu->reg[inst.ra];
            valB = cpu->reg[inst.rb];
            valE = 0;
            int64_t tvalE = 0;
            int64_t tvalB = valB;
            int64_t tvalA = *valA;
            switch(inst.ifun.op) {
                case ADD:
                    tvalE = tvalB + tvalA;
                    //set flags
                    // Check for overflow
                    cpu->of = (tvalB < 0 && tvalA < 0 && tvalE > 0) || (tvalB > 0 && tvalA > 0 && tvalE < 0);
                    valE = tvalE;
                    break;
                case SUB:
                    tvalE = tvalB - tvalA;
                    //set flags
                    // Check for overflow
                    cpu->of = ((tvalB < 0 && tvalA > 0 && tvalE > 0) || (tvalB > 0 && tvalA < 0 && tvalE < 0 ));
                    valE = tvalE;
                    break;
                case AND:
                    valE = *valA & valB;
                    break;
                case XOR:
                    valE = *valA ^ valB;
                    break;
                case BADOP:
                    cpu->stat = INS;
                    valE = 0;
                    break;
                default:
                    cpu->stat = INS;
                    valE = 0;
                    break;
            }
            cpu->sf = (valE >> 63 == 1);
            cpu->zf = (valE == 0);
            break;
        case JUMP:
            *cnd = getJump(inst.ifun.jump, cpu);
            break;
        case CALL:
            valB = cpu->reg[RSP];
            valE = valB - 8;
            break;
        case RET:
            *valA = cpu->reg[RSP];
            valB = cpu->reg[RSP];
            valE = valB + 8;
            break;
        case PUSHQ:
            *valA = cpu->reg[inst.ra];
            valB = cpu->reg[RSP];
            valE = valB - 8;
            break;
        case POPQ:
            *valA = cpu->reg[RSP];
            valB = cpu->reg[RSP];
            valE = valB + 8;
            break;
        case IOTRAP:
            //  cpu->pc = inst.valP;
            break;
        case INVALID:
            cpu->stat = INS;
            break;
        default:
            cpu->stat = INS;
            break;
    }
    return valE;
}

void memory_wb_pc (y86_t *cpu, y86_inst_t inst, byte_t *memory,
                   bool cnd, y86_reg_t valA, y86_reg_t valE)
{
    y86_reg_t valM;
    uint64_t *t;
    static char buffer[100];
    if(valA < 0 || valE < 0) {
        cpu->stat = ADR;
    }
    if (cpu->pc >= MEMSIZE || cpu->pc < 0 || memory == NULL) {
        cpu->stat = ADR;
    }
    switch(inst.icode) {
        case HALT:
            cpu->pc = inst.valP;
            break;
        case NOP:
            cpu->pc = inst.valP;
            break;
        case CMOV:
            if(cnd) {
                //cnd ? R[rb] <- valE
                cpu->reg[inst.rb] = valE;
            }
            cpu->pc += 2;
            break;
        case IRMOVQ:
            // Rrb <- valE
            cpu->reg[inst.rb] = valE;
            cpu->pc = inst.valP;
            break;
        case RMMOVQ:
            if(valE >= MEMSIZE) {
                cpu->stat = ADR;
                cpu->pc = inst.valP;
                break;
            }
            cpu->pc = inst.valP;
            t = (uint64_t *) &memory[valE];
            *t = valA;
            break;
        case MRMOVQ:
            if(valE >= MEMSIZE) {
                cpu->stat = ADR;
                cpu->pc = inst.valP;
                break;
            }
            t = (uint64_t *) &memory[valE];
            valM = *t;
            //rRa <- valM
            cpu->reg[inst.ra] = valM;
            cpu->pc = inst.valP;
            break;
        case OPQ:
            //R[rb] <- valE
            cpu->reg[inst.rb] = valE;
            cpu->pc = inst.valP;
            break;
        case JUMP:
            if(cnd) {
                cpu->pc = inst.valC.dest;
            } else {
                cpu->pc = inst.valP;
            }
            break;
        case CALL:
            if(valE >= MEMSIZE) {
                cpu->stat = ADR;
                cpu->pc = inst.valC.dest;
                break;
            }
            t = (uint64_t *) &memory[valE];
            *t = inst.valP;
            cpu->reg[RSP] = valE;
            cpu->pc = inst.valC.dest;
            break;
        case RET:
            //valM <- m8[valA]
            //R[rsp] = valE
            if(valE >= MEMSIZE) {
                cpu->stat = ADR;
            }
            t = (uint64_t *) &memory[valA];
            valM = *t;
            cpu->reg[RSP] = valE;
            cpu->pc = valM;
            break;
        case PUSHQ:
            //m8[vale] = valA
            //rrsp = valE
            if(valE >= MEMSIZE) {
                cpu->stat = ADR;
                cpu->pc = inst.valP;
            }
            t = (uint64_t *) &memory[valE];
            *t = valA;
            cpu->reg[RSP] = valE;
            cpu->pc = inst.valP;
            break;
        case POPQ:
            //rRa <- valM
            //t = (uint64_t *) &memory[valE];
            // valM = *t;
            //THE STUFF ABOVE IS THE M8[REG] PART
            if(valE >= MEMSIZE) {
                cpu->stat = ADR;
                cpu->pc = inst.valP;
            }
            t = (uint64_t *) &memory[valA];
            valM = *t;
            cpu->reg[RSP] = valE;
            cpu->reg[inst.ra] = valM;
            cpu->pc = inst.valP;
            break;
        case IOTRAP:
            handle_iotrap(cpu, inst, memory, buffer);
            cpu->pc = inst.valP;
            break;
        case INVALID:
            cpu->stat = INS;
            break;
    }

}


/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p4 (char **argv)
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
    printf("  -e      Execute program\n");
    printf("  -E      Execute program (trace mode)\n");
}

bool parse_command_line_p4 (int argc, char **argv,
                            bool *print_header, bool *print_phdrs,
                            bool *print_membrief, bool *print_memfull,
                            bool *disas_code, bool *disas_data,
                            bool *exec_normal, bool *exec_debug, char **filename)
{
    if (argc <= 1 || argv == NULL || print_header == NULL || print_phdrs == NULL
            || print_membrief == NULL
            || print_memfull == NULL || filename == NULL || disas_code == NULL || disas_data == NULL
            || exec_normal == NULL || exec_debug == NULL) {
        return false;
    }
    int c;
    bool membrief = false;
    bool memfull = false;
    bool x_debug = false;
    bool x_normal = false;
    while ((c = getopt(argc, argv, "hHafsmMDdEe")) != -1) {
        switch (c) {
            case 'h':
                usage_p4(argv);
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
                    usage_p4(argv);
                    return false;  // (both -m and -M)
                }
                *print_membrief = true;
                membrief = true;
                break;
            case 'M':
                if (membrief) {
                    usage_p4(argv);
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
            case 'E':
                if(x_normal) {
                    usage_p4(argv);
                    return false;
                }
                *exec_debug = true;
                x_debug = true;
                break;
            case 'e':
                if(x_debug) {
                    usage_p4(argv);
                    return false;
                }
                *exec_normal = true;
                x_normal = true;
                break;
            default:
                usage_p4(argv);
                return false;
        }

        if (memfull && membrief) {
            usage_p4(argv);
            return false;
        }
        if(x_normal && x_debug) {
            usage_p4(argv);
            return false;
        }

    }
    //no filename (or extra output)
    if (optind != argc - 1) {
        usage_p4(argv);
        return false;
    } else {
        *filename = argv[optind];  // Save the filename
        if (*filename == NULL) {
            usage_p4(argv);
            return false;
        }

        return true;

    }
}

void dump_cpu_state (y86_t *cpu)
{
    printf("Y86 CPU state:\n");
    printf("    PC: %016lx   flags: Z%d S%d O%d     ", cpu->pc, cpu->zf, cpu->sf, cpu->of);

    switch(cpu->stat) { //the cpu status at the end for line above
        case 1:
            printf("AOK\n");
            break;
        case 2:
            printf("HLT\n");
            break;
        case 3:
            printf("ADR\n");
            break;
        case 4:
            printf("INS\n");
            break;
    }
    printf("  %%rax: %016lx    %%rcx: %016lx\n",  cpu->reg[RAX],  cpu->reg[RCX]);
    printf("  %%rdx: %016lx    %%rbx: %016lx\n", cpu->reg[RDX], cpu->reg[RBX]);
    printf("  %%rsp: %016lx    %%rbp: %016lx\n",   cpu->reg[RSP],  cpu->reg[RBP]);
    printf("  %%rsi: %016lx    %%rdi: %016lx\n",  cpu->reg[RSI], cpu->reg[RDI]);
    printf("   %%r8: %016lx     %%r9: %016lx\n",  cpu->reg[R8],  cpu->reg[R9]);
    printf("  %%r10: %016lx    %%r11: %016lx\n", cpu->reg[R10], cpu->reg[R11]);
    printf("  %%r12: %016lx    %%r13: %016lx\n",   cpu->reg[R12],  cpu->reg[R13]);
    printf("  %%r14: %016lx\n",  cpu->reg[R14]);

}
