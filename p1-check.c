/*
 * CS 261 PA1: Mini-ELF header verifier
 *
 * Name: marcus tran
 */

#include "p1-check.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

// Function to read a Mini-ELF header from a file
bool read_header(FILE *file, elf_hdr_t *hdr)
{
    if (file == NULL) {
        return false;
    }

    //read 16 bytes
    size_t bytes_read = fread(hdr, 1, sizeof(elf_hdr_t), file);

    if (bytes_read != sizeof(elf_hdr_t)) {
        return false;
    }

    //MATCH THE MAGIC
    if (hdr->magic != 4607045) {
        return false;
    }

    return true;

}
/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p1 (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
}

bool parse_command_line_p1 (int argc, char **argv, bool *header, char **filename)
{
    if (argv == NULL || header == NULL || filename == NULL) {
        return false;
    }
 
    // parameter parsing w/ getopt()
    int c;
    while ((c = getopt(argc, argv, "hH")) != -1) {
        switch (c) {
            case 'h':
                usage_p1(argv);
                return true;
            case 'H':
                *header = true;
                break;
            default:
                usage_p1(argv);
                return false;
        }
    }
 
    if (optind != argc-1) {
        // no filename (or extraneous input)
        usage_p1(argv);
        return false;
    }
    *filename = argv[optind];   // save filename
 
    return true;
}

//the formatting
void dump_header(elf_hdr_t *hdr)
{
    printf("Mini-ELF version %u\n", hdr->e_version);
    printf("Entry point 0x%0x\n", hdr->e_entry);
    printf("There are %u program headers, starting at offset %u (0x%0x)\n", hdr->e_num_phdr,
           hdr->e_phdr_start, hdr->e_phdr_start);

    if (hdr->e_symtab != 0) {
        printf("There is a symbol table starting at offset %u (0x%0x)\n", hdr->e_symtab, hdr->e_symtab);
    } else {
        printf("There is no symbol table present\n");
    }

    if (hdr->e_strtab != 0) {
        printf("There is a string table starting at offset %u (0x%0x)\n", hdr->e_strtab, hdr->e_strtab);
    } else {
        printf("There is no string table present\n");
    }
}




