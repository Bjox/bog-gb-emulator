#include "Global.h"
#include "BogCPU.h"
#include <intrin.h>





int main()
{
	BogCPU cpu;

	cpu.run();
	cpu.printRegisters();


	system("pause");
	return 0;
}


