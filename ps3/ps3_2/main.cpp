#include <fslazywindow.h>
#include <simplebitmap.h>
#include <hashtable.h>
#include <string.h>


class FsLazyWindowApplication : public FsLazyWindowApplicationBase
{
protected:
	const int a = 40;
	bool needRedraw;
	SimpleBitmap bmp, c, temp, all;
	HashTable<SimpleBitmap, int> h;
	int state, val, moveX, moveY;
	
public:
	FsLazyWindowApplication();
	virtual void BeforeEverything(int argc, char *argv[]);
	virtual void GetOpenWindowOption(FsOpenWindowOption &OPT) const;
	virtual void Initialize(int argc, char *argv[]);
	virtual void Interval(void);
	virtual void BeforeTerminate(void);
	virtual void Draw(void);
	virtual bool UserWantToCloseProgram(void);
	virtual bool MustTerminate(void) const;
	virtual long long int GetMinimumSleepPerInterval(void) const;
	virtual bool NeedRedraw(void) const;
};

FsLazyWindowApplication::FsLazyWindowApplication()
{
	needRedraw = false;
}

/* virtual */ void FsLazyWindowApplication::BeforeEverything(int argc, char *argv[])
{
}
/* virtual */ void FsLazyWindowApplication::GetOpenWindowOption(FsOpenWindowOption &opt) const
{
	opt.x0 = 0;
	opt.y0 = 0;
	opt.wid = 1200;
	opt.hei = 800;
}
/* virtual */ void FsLazyWindowApplication::Initialize(int argc, char *argv[])
{
	state = 0;
	moveX = 0;
	moveY = 0;
	val = 0;

	if (2 <= argc && true == bmp.LoadPng(argv[1]))
	{
		for (int j = 0; j < bmp.GetHeight(); j += a)
		{
			for (int i = 0; i < bmp.GetWidth(); i += a)
			{
				c = bmp.CutOut(i, j, a, a);
				if (!h.IsIncluded(c))
				{
					h.Update(c, val);
				}
				val++;
			}
		}
	}

	else
	{
		printf("Usage: ps3_2 <pngFileName.png>\n");
	}
}

/* virtual */ void FsLazyWindowApplication::Interval(void)
{
	auto key = FsInkey();
	if (FSKEY_ESC == key)
	{
		SetMustTerminate(true);
	}

	if (FSKEY_SPACE == key)
	{
		state = 1;
		//bmp.Invert();
	}
	if (FSKEY_UP == key)
	{
		moveY -= 40;
	}

	if (FSKEY_DOWN == key)
	{
		moveY += a;
	}
	if (FSKEY_LEFT == key)
	{
		moveX -= a;
	}
	if (FSKEY_RIGHT == key)
	{
		moveX += a;
	}
	needRedraw = true;
}
/* virtual */ void FsLazyWindowApplication::Draw(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int wid, hei;
	FsGetWindowSize(wid, hei);
	int xn = 0, yn = 0;
	int x = 0, y = 40;
	if (state == 0)
	{
		for (auto hd = h.First(); true == h.IsNotNull(hd); hd = h.Next(hd))
		{
			temp = h.GetKey(hd);
			temp.Invert();

			glRasterPos2i(x, y);
			glDrawPixels(temp.GetWidth(), temp.GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, temp.GetBitmapPointer());

			if (x < wid - 40)
			{
				x = x + a;
			}
			else
			{
				x = 0;
				y = y + a;
			}
		}
	}
	else
	{
		if (moveX > (bmp.GetWidth() - wid))
		{
			moveX = (bmp.GetWidth() - wid);
		}
		if (moveX < 0)
		{
			moveX = 0;
		}
		if (moveY < 0)
		{
			moveY = 0;
		}
		if (moveY >(bmp.GetHeight() - hei))
		{
			moveY = (bmp.GetHeight() - hei);
		}
		all = bmp.CutOut(xn + moveX, yn + moveY, wid, hei);
		all.Invert();
		glRasterPos2i(0, 742);
		glDrawPixels(all.GetWidth(), all.GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, all.GetBitmapPointer());
	}
	FsSwapBuffers();
	needRedraw = false;
}
/* virtual */ bool FsLazyWindowApplication::UserWantToCloseProgram(void)
{
	return true; // Returning true will just close the program.
}
/* virtual */ bool FsLazyWindowApplication::MustTerminate(void) const
{
	return FsLazyWindowApplicationBase::MustTerminate();
}
/* virtual */ long long int FsLazyWindowApplication::GetMinimumSleepPerInterval(void) const
{
	return 10;
}
/* virtual */ void FsLazyWindowApplication::BeforeTerminate(void)
{
}
/* virtual */ bool FsLazyWindowApplication::NeedRedraw(void) const
{
	return needRedraw;
}


static FsLazyWindowApplication *appPtr = nullptr;

/* static */ FsLazyWindowApplicationBase *FsLazyWindowApplicationBase::GetApplication(void)
{
	if (nullptr == appPtr)
	{
		appPtr = new FsLazyWindowApplication;
	}
	return appPtr;
}