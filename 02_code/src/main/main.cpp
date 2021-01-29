extern "C" {
#include "..\\fc\fc.h"
#include "stdio.h"
}

#include "graphics.h"
#include <conio.h>

int main(void)
{
	fc_state_t *fs;
	fs = fc_new();
	fc_dofile(fs,"nestest.nes");
	fc_del(&fs);
	return 0;
}

