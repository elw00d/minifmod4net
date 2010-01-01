
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "..\MiniFmodFacade\fmod_facade.h"

int main()
{
	if (!XM_LoadModuleFromFile(1, "test.xm")) {
		printf("Fuck");
		return 0;
	}

	XM_PlayModule(1);
	XM_FreeModule(1);
	//XM_LoadModuleFromFile(1, "test.xm");

	int len = 0;
	char* data = NULL;
		FILE* fp;
		fp = fopen("test.xm", "rb");
		if (fp) {
			fseek(fp, 0, SEEK_END);
			len = ftell(fp);
			data = (char*) calloc(len, 1);

			fseek(fp, 0, SEEK_SET);
			fread(data, 1, len, fp);
			fclose(fp);
		}
		XM_LoadModuleFromMemory(1, data, len, true);

	XM_PlayModule(1);

	printf("Press any key to quit\n");
	printf("=========================================================================\n");
	printf("Playing song...\n");

	{
		char key = 0;
		do
		{
			int		ord = 0, row = 0;
			float	mytime = 0;
			if (kbhit())
			{
				key = getch();
			}

			ord = XM_GetCurrentOrder(1);
			row = XM_GetCurrentRow(1);
			mytime = (float)XM_GetCurrentRow(1) / 1000.0f;
			printf("ord %2d row %2d seconds %5.02f %s      \r", ord, row, mytime, (row % 8 ? "    " : "TICK"));

		} while (key != 27);
	}

	XM_FreeModule(1);
}