#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE 2048
#define BOOT_BIN_SIZE 512
#define ISO_NAME "ovos.iso"

void write_empty_sector(FILE *fp) {
    char zero[SECTOR_SIZE];
    memset(zero, 0, SECTOR_SIZE);
    fwrite(zero, 1, SECTOR_SIZE, fp);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s boot.bin\n", argv[0]);
        return 1;
    }

    FILE *boot = fopen(argv[1], "rb");
    if (!boot) {
        perror("boot.bin");
        return 1;
    }

    unsigned char boot_img[SECTOR_SIZE];
    memset(boot_img, 0, SECTOR_SIZE);

    size_t read = fread(boot_img, 1, BOOT_BIN_SIZE, boot);
    fclose(boot);

    if (read != BOOT_BIN_SIZE || boot_img[510] != 0x55 || boot_img[511] != 0xAA) {
        fprintf(stderr, "boot.bin doit faire 512 octets et se terminer par 0xAA55.\n");
        return 1;
    }

    FILE *iso = fopen(ISO_NAME, "wb");
    if (!iso) {
        perror(ISO_NAME);
        return 1;
    }

    for (int i = 0; i < 16; i++) {
        write_empty_sector(iso);
    }

    char pvd[SECTOR_SIZE];
    memset(pvd, 0, SECTOR_SIZE);
    pvd[0] = 0x01;
    memcpy(pvd + 1, "CD001", 5);
    pvd[6] = 0x01;
    memcpy(pvd + 8, "BOOTABLE ISO", 13);
    pvd[80] = 0x13;
    pvd[0x47] = 19;
    fwrite(pvd, 1, SECTOR_SIZE, iso);

    char brvd[SECTOR_SIZE];
    memset(brvd, 0, SECTOR_SIZE);
    brvd[0] = 0x00;
    memcpy(brvd + 1, "CD001", 5);
    brvd[6] = 0x01;
    memcpy(brvd + 7, "EL TORITO SPECIFICATION", 23);
    brvd[0x47] = 19;
    fwrite(brvd, 1, SECTOR_SIZE, iso);

    char vdst[SECTOR_SIZE];
    memset(vdst, 0, SECTOR_SIZE);
    vdst[0] = 0xFF;
    memcpy(vdst + 1, "CD001", 5);
    vdst[6] = 0x01;
    fwrite(vdst, 1, SECTOR_SIZE, iso);

    char catalog[SECTOR_SIZE];
    memset(catalog, 0, SECTOR_SIZE);
    catalog[0x00] = 0x01;
    catalog[0x01] = 0x00;
    catalog[0x20] = 0x88;
    catalog[0x21] = 0x00;
    catalog[0x22] = 0x00;
    catalog[0x23] = 0x00;
    catalog[0x24] = 0x14;
    catalog[0x25] = 0x00;
    catalog[0x26] = 0x01;
    catalog[0x28] = 20;
    fwrite(catalog, 1, SECTOR_SIZE, iso);

    fwrite(boot_img, 1, SECTOR_SIZE, iso);

    fclose(iso);

    printf("ISO bootable créé : %s\n", ISO_NAME);
    return 0;
}