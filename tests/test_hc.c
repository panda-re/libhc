#include <stdio.h>
#include "hypercall.h"


int main() {
    int magic = 6767;
    printf("[*] Attempting hypercall %d...\n", magic);
    printf("[!] NOTE: This will FAIL if run outside of PANDA/QEMU!\n");

    // In PANDA, the handler will set the return value to 6767 if successful
    int result = (int) igloo_hypercall4(magic, 42, 43, 44, 45);

    printf("[+] Hypercall finished. Result: %d\n", result);

    if (result == magic) {
        printf("[+] Success!\n");
        return 0;
    } else {
        printf("[-] Failure: Expected %d, got %d\n", magic, result);
        return 1;
    }
}