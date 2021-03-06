#include <simplebitmap.h>
#include <string.h>

int main(int argc, char *argv[])
{
	SimpleBitmap bmp;
	int x = 0, y = 0;
	if (2 <= argc && true == bmp.LoadPng(argv[1]))
	{
		auto numPix = bmp.GetWidth() * bmp.GetHeight();
		{
			for (int i = 0; i < numPix; i++) 
			{
				char fn[255];
				sprintf(fn, "./ver/%d.png", i);
				FILE *fp = fopen(fn, "wb");
				if (nullptr != fp)
				{
					SimpleBitmap newCutOut = bmp.CutOut(x, y, 40, 40);
					newCutOut.SavePng(fp);
					fclose(fp);

					if (x < bmp.GetWidth())
					{
						x = x + 40;
					}
					else
					{
						x = 0;
						y = y + 40;
					}
					if (y > bmp.GetHeight() || i >= 199)
					{
						break;
					}
				}
				else
				{
					printf("Error: Failed to read a .PNG file.\n");
				}
			}
			
		}
		
	}
	else
	{
		printf("Usage: ps3_1 <pngFileName.png>\n");
	}
	return 0;
}
