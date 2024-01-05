/*
 * CS 261 PA2: Mini-ELF loader
 *
 * Name: marcus tran
 */

#include "p2-load.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

bool read_phdr (FILE *file, uint16_t offset, elf_phdr_t *phdr)
{
    if (file == NULL || phdr == NULL) { //null file
        return false;
    }

    if (fseek(file, offset, SEEK_SET) != 0) {
        return false; //wrong offset
    }
    size_t bytes_read = fread(phdr, 1, sizeof(elf_phdr_t), file);
    if (bytes_read != sizeof(elf_phdr_t)) {
        return false; //reading failed
    }
    if (phdr->magic != 0xDEADBEEF) {
        return false; //wrong magic
    }



    return true;
}

bool load_segment (FILE *file, byte_t *memory, elf_phdr_t *phdr)
{
    if (file == NULL || memory == NULL || phdr == NULL) {
        return false;
    }

    //zero byte
    if(phdr->p_size > 0) {
        if (fseek(file, phdr->p_offset, SEEK_SET) != 0) {
            return false; //wrong offset
        }
    }
    if (phdr->p_vaddr + phdr->p_size > MEMSIZE) {
        return false; // Writing past the end of virtual memory
    }


    size_t bytes_read = fread(memory + phdr->p_vaddr, 1, phdr->p_size, file);
    if (bytes_read != phdr->p_size) {
        return false; // Reading failed
    }

    return true;
}


/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p2 (char **argv)
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
}

bool parse_command_line_p2 (int argc, char **argv,
                            bool *print_header, bool *print_phdrs,
                            bool *print_membrief, bool *print_memfull,
                            char **filename)
{
    if (argv == NULL || print_header == NULL || print_phdrs == NULL || print_membrief == NULL
            || print_memfull == NULL || filename == NULL) {
        return false;
    }
    int c;
    bool membrief = false;
    bool memfull = false;
    while ((c = getopt(argc, argv, "hHafsmM")) != -1) {
        switch (c) {
            case 'h':
                usage_p2(argv);
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
                    usage_p2(argv);
                    return false;  // (both -m and -M)
                }
                *print_membrief = true;
                membrief = true;
                break;
            case 'M':
                if (membrief) {
                    usage_p2(argv);
                    return false;  //(both -m and -M)
                }
                *print_memfull = true;
                memfull = true;
                break;
            default:
                usage_p2(argv);
                return false;
        }

        if (memfull && membrief) {
            usage_p2(argv);
            return false;
        }

    }
    //no filename (or extra output)
    if (optind != argc - 1) {
        usage_p2(argv);
        return false;
    }

    if (argc <= 1) { //no flags
        return false;
    }

    *filename = argv[optind];  // Save the filename

    return true;

}


void dump_phdrs(uint16_t numphdrs, elf_phdr_t *phdrs)
{
    printf(" Segment   Offset    Size      VirtAddr  Type      Flags\n");
    for (uint16_t i = 0; i < numphdrs; i++) {
        elf_phdr_t *phdr = &phdrs[i];

        //  segment type
        char *segment_type = "DATA";
        if (phdr->p_type == 0x0001) {
            segment_type = "CODE";
        }
        if (phdr->p_type == 0x0002) {
            segment_type = "STACK";
        }
        if (phdr->p_type == 0x0004) {
            segment_type = "HEAP";
        }
        if (phdr->p_type == 0x0005) {
            segment_type = "UNKNOWN";
        }
        char flags[4] = "   "; //
        if (phdr->p_flags & 0x0004) {
            flags[0] = 'R';    // Read
        }
        if (phdr->p_flags & 0x0002) {
            flags[1] = 'W';    // Write
        }
        if (phdr->p_flags & 0x0001) {
            flags[2] = 'X';    // Execute
        }




        printf("  %02d       0x%04lx    0x%04lx    0x%04lx    %-5s     %s\n",
               i, (unsigned long)phdr->p_offset, (unsigned long)phdr->p_size, (unsigned long)phdr->p_vaddr,
               segment_type, flags);

    }
}



void dump_memory(byte_t *memory, uint16_t start, uint16_t end)
{
    printf("Contents of memory from %04lx to %04lx:\n", (unsigned long)start, (unsigned long)end);
    if (start >= end) {
        // No bytes to print.

        return;
    }

    // align the addresses
    uint16_t aligned_start = start & 0xFFF0;
    uint16_t address = aligned_start;
    while (address < end) {
        printf("  %04lx ", (unsigned long)address);
        for (int i = 0; i < 16; i++) {
            if (address < end) {
                if (i == 8) {
                    printf("  "); // Double space in the middle
                } else  {
                    printf(" "); // Space between bytes
                }
                printf("%02lx", (unsigned long)memory[address]);
                address++;
            }
        }

        printf("\n"); // Move to the next line
    }
}




