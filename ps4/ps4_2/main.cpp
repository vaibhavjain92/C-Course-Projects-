#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <ysglfontdata.h>
#include <fslazywindow.h>

#include "bintree.h"

class FsLazyWindowApplication : public FsLazyWindowApplicationBase
{
protected:
	bool needRedraw;
	BinaryTree <int,int> tree;
	BinaryTree <int,int>::NodeHandle mouseOn;

public:
	FsLazyWindowApplication();
	virtual void BeforeEverything(int argc,char *argv[]);
	virtual void GetOpenWindowOption(FsOpenWindowOption &OPT) const;
	virtual void Initialize(int argc,char *argv[]);
	virtual void Interval(void);
	virtual void BeforeTerminate(void);
	virtual void Draw(void);
	virtual bool UserWantToCloseProgram(void);
	virtual bool MustTerminate(void) const;
	virtual long long int GetMinimumSleepPerInterval(void) const;
	virtual bool NeedRedraw(void) const;

	void DrawNode(int x0,int x1,int y0,int yStep,int prevX,int prevY,BinaryTree<int,int>::NodeHandle ndHd) const;
	BinaryTree <int,int>::NodeHandle FindNodeFromMouseCoord(int mx,int my) const;
	BinaryTree <int,int>::NodeHandle FindNodeFromMouseCoord(int mx,int my,int x0,int x1,int y0,int yStep,BinaryTree<int,int>::NodeHandle ndHd) const;
};

FsLazyWindowApplication::FsLazyWindowApplication()
{
	needRedraw=false;
	mouseOn.Nullify();
}

/* virtual */ void FsLazyWindowApplication::BeforeEverything(int argc,char *argv[])
{
}
/* virtual */ void FsLazyWindowApplication::GetOpenWindowOption(FsOpenWindowOption &opt) const
{
	opt.x0=0;
	opt.y0=0;
	opt.wid=1200;
	opt.hei=800;
}
/* virtual */ void FsLazyWindowApplication::Initialize(int argc,char *argv[])
{
	srand((int)time(nullptr));
	tree.autoRebalance = true;
	for(int i=0; i<50; ++i)
	{
		tree.Insert(rand()%100,0);
	}
}
/* virtual */ void FsLazyWindowApplication::Interval(void)
{
	auto key=FsInkey();
	if(FSKEY_ESC==key)
	{
		SetMustTerminate(true);
	}
	if(FSKEY_DEL==key)
	{
		tree.Delete(mouseOn);
		needRedraw=true;
	}
	else if(FSKEY_INS==key)
	{
		tree.Insert(rand()%100,0);
		needRedraw=true;
	}
	else if(FSKEY_L==key)
	{
		tree.RotateLeft(mouseOn);
		needRedraw=true;
	}
	else if (FSKEY_R == key)
	{
		tree.RotateRight(mouseOn);
		needRedraw = true;
	}
	
	else if (FSKEY_SPACE == key)
	{
		tree.Insert(rand() % 100, 0);
		needRedraw = true;
	}

	int lb,mb,rb,mx,my;
	auto evt=FsGetMouseEvent(lb,mb,rb,mx,my);
	auto pick=FindNodeFromMouseCoord(mx,my);
	if(pick!=mouseOn)
	{
		mouseOn=pick;
		needRedraw=true;
	}
}
/* virtual */ void FsLazyWindowApplication::Draw(void)
{
	int wid,hei;
	FsGetWindowSize(wid,hei);

	glViewport(0,0,wid,hei);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,(float)wid-1,(float)hei-1,0,-1,1);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	DrawNode(0,wid,0,40, wid/2,20, tree.RootNode());
	FsSwapBuffers();
	needRedraw=false;
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

void FsLazyWindowApplication::DrawNode(int x0,int x1,int y0,int yStep,int prevX,int prevY,BinaryTree<int,int>::NodeHandle ndHd) const
{
	if(ndHd.IsNotNull())
	{
		int x=(x0+x1)/2,y=y0+yStep/2;
		if(ndHd==mouseOn)
		{
			glColor3ub(255,0,0);
		}
		else
		{
			glColor3ub(0,0,0);
		}
		glBegin(GL_LINES);
		glVertex2i(prevX,prevY);
		glVertex2i(x,y);
		glEnd();

		glRasterPos2i(x,y);
		char str[256];
		sprintf(str,"%d(%d)",tree.GetKey(ndHd),tree.GetHeight(ndHd));
		YsGlDrawFontBitmap12x16(str);

		DrawNode(x0,(x0+x1)/2,y0+yStep,yStep,x,y,tree.Left(ndHd));
		DrawNode((x0+x1)/2,x1,y0+yStep,yStep,x,y,tree.Right(ndHd));
	}
}

BinaryTree <int,int>::NodeHandle FsLazyWindowApplication::FindNodeFromMouseCoord(int mx,int my) const
{
	int wid,hei;
	FsGetWindowSize(wid,hei);
	return FindNodeFromMouseCoord(mx,my,0,wid,0,40, tree.RootNode());
}

BinaryTree <int,int>::NodeHandle FsLazyWindowApplication::FindNodeFromMouseCoord(
    int mx,int my,int x0,int x1,int y0,int yStep,BinaryTree<int,int>::NodeHandle ndHd) const
{
	if(ndHd.IsNotNull())
	{
		if(x0<=mx && mx<x1 && y0<=my && my<y0+yStep)
		{
			return ndHd;
		}
		auto fromLeft=FindNodeFromMouseCoord(mx,my,x0,(x0+x1)/2,y0+yStep,yStep,tree.Left(ndHd));
		if(fromLeft.IsNotNull())
		{
			return fromLeft;
		}
		auto fromRight=FindNodeFromMouseCoord(mx,my,(x0+x1)/2,x1,y0+yStep,yStep,tree.Right(ndHd));
		if(fromRight.IsNotNull())
		{
			return fromRight;
		}
	}
	return tree.Null();
}

static FsLazyWindowApplication *appPtr=nullptr;

/* static */ FsLazyWindowApplicationBase *FsLazyWindowApplicationBase::GetApplication(void)
{
	if(nullptr==appPtr)
	{
		appPtr=new FsLazyWindowApplication;
	}
	return appPtr;
}
