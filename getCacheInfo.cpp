#include <iostream>
#include <stdio.h>
#include <stdlib.h>

int MASK_LSB_8 = 0xFF;

void int2bin(int val) {
	char buffer[33];
	atol(val, buffer, 2);
	printf("binary: %s\n", buffer);
}

// CPUID instruction takes no parameters as CPUID implicitly uses the EAX register.
// The EAX register should be loaded with a value specifying what information to return
void cpuinfo(int code, int *eax, int *ebx, int *ecx, int *edx) {
	__asm__ volatile(
			"cpuid;" //  call cpuid instruction
			:"=a"(*eax),"=b"(*ebx),"=c"(*ecx), "=d"(*edx)// output equal to "movl  %%eax %1"
			:"a"(code)// input equal to "movl %1, %%eax"
			//:"%eax","%ebx","%ecx","%edx"// clobbered register
	);
}

int main() {
	int eax = 0, ebx = 0, ecx = 0, edx = 0;
	//CPUID(0): Basic information
	cpuinfo(0x0, &eax, &ebx, &ecx, &edx);

	// check vendor [GenuineIntel, GenuntelineI] = ebx+ecx+edx
	char vendor[13] = { 0 };
	int identity_reg[3] = { ebx, ecx, edx };
	for (unsigned int i = 0; i < 3; i++) {
		for (unsigned int j = 0; j < 4; j++) {
			vendor[i * 4 + j] = identity_reg[i] >> (j * 8) & MASK_LSB_8;
		}
	}
	printf("\nVendor name = %s\n", vendor);

	//CPUID(0x80000000): Extended CPU information
	cpuinfo(0x80000000, &eax, &ebx, &ecx, &edx);
	int intelExtended = eax & MASK_LSB_8;
	printf("\nMax feature id = %d\n", intelExtended);
	if (intelExtended < 6) {
		printf("\nCache Extended Feature not supported");
		return 0;
	}
	// CPUID(0x800000006): Cache line information
	cpuinfo(0x80000006, &eax, &ebx, &ecx, &edx);
	// We are only interested in lower 8 bits
	printf("\nL2 Cache Size = %d\n", (ecx & MASK_LSB_8));
}
