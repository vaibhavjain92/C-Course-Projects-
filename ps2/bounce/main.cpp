#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <fslazywindow.h>

const double YsPi = 3.1415927;

class FsLazyWindowApplication : public FsLazyWindowApplicationBase
{
protected:
	bool needRedraw;
	enum
	{
		nBall = 100
	};
	void DrawCircle(int xi, int yi, int ri)
	{
		double x = (double)xi;
		double y = (double)yi;
		double r = (double)ri;

		glBegin(GL_TRIANGLE_FAN);
		for (int a = 0; a<64; ++a)
		{
			const double radian = YsPi*2.0*(double)a / 64.0;
			const double c = cos(radian);
			const double s = sin(radian);

			glVertex2d(x + c*r, y + s*r);
		}
		glEnd();
	}

	void ConvertToScreen(int &sx, int &sy, double x, double y)
	{
		sx = 400 + (int)(x*10.0);
		sy = 600 - (int)(y*10.0);
	}

	void MoveBall(double &x, double &y, double &vx, double &vy, double m, double dt)
	{
		x += vx*dt;
		y += vy*dt;

		double Fx = 0.0;
		double Fy = -9.8*m;

		double ax = Fx / m;
		double ay = Fy / m;

		vx += ax*dt;
		vy += ay*dt;
	}

	void BounceBall(double &x, double &y, double &vx, double &vy)
	{
		if (y<1.0 && vy<0.0)
		{
			vy = -vy;
		}
		if ((x<-39.0 && vx<0.0) || (39.0<x && 0.0<vx))
		{
			vx = -vx;
		}
	}

	void Collision(double x1, double y1, double &vx1, double &vy1, double r1,
		double x2, double y2, double &vx2, double &vy2, double r2)
	{
		double dx = x2 - x1;
		double dy = y2 - y1;
		double distSq = dx*dx + dy*dy;
		double dist = sqrt(distSq);
		if (dist < r1 + r2)
		{
			if (0.0 < distSq)
			{
				double nx = (x2 - x1) / dist;
				double ny = (y2 - y1) / dist;
				double k1 = vx1*nx + vy1*ny;
				double k2 = vx2*nx + vy2*ny;

				if ((k2 > 0.0 && k2 < k1) ||
					(k1 < 0.0 && k2 < k1) ||
					(k1 > 0.0 && k2 < 0.0))
				{
					vx1 = vx1 + nx*(k2 - k1);
					vy1 = vy1 + ny*(k2 - k1);

					vx2 = vx2 + nx*(k1 - k2);
					vy2 = vy2 + ny*(k1 - k2);
				}
			}
		}
	}

	//const int nBall;
	double ballX[nBall], ballY[nBall];
	double ballVx[nBall], ballVy[nBall];
	double dt;

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
};

FsLazyWindowApplication::FsLazyWindowApplication()
{
	needRedraw=false;
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
	dt = 0.025;
	srand((int)time(NULL));

	for (int i = 0; i<nBall; ++i)
	{
		int x = rand() % 20 - 10;
		int y = rand() % 20 + 20;
		int vx = rand() % 10 - 20;
		int vy = rand() % 10 - 20;

		ballX[i] = (double)x;
		ballY[i] = (double)y;
		ballVx[i] = (double)vx;
		ballVy[i] = (double)vy;
	}
}
/* virtual */ void FsLazyWindowApplication::Interval(void)
{
	auto key=FsInkey();
	if(FSKEY_ESC==key)
	{
		SetMustTerminate(true);
	}

	for (int i = 0; i < nBall; ++i)
	{
		MoveBall(ballX[i], ballY[i], ballVx[i], ballVy[i], 1.0, dt);
		BounceBall(ballX[i], ballY[i], ballVx[i], ballVy[i]);
	}

	for (int i = 0; i < nBall; ++i)
	{
		for (int j = i + 1; j < nBall; ++j)
		{
			Collision(ballX[i], ballY[i], ballVx[i], ballVy[i], 1.0,
				ballX[j], ballY[j], ballVx[j], ballVy[j], 1.0);
		}
	}

	needRedraw = true;
}
/* virtual */ void FsLazyWindowApplication::Draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3ub(0, 0, 255);

	for (int i = 0; i<nBall; ++i)
	{
		int sx, sy;
		ConvertToScreen(sx, sy, ballX[i], ballY[i]);
		DrawCircle(sx, sy, 10);
	}

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
	return 25;
}
/* virtual */ void FsLazyWindowApplication::BeforeTerminate(void)
{
}
/* virtual */ bool FsLazyWindowApplication::NeedRedraw(void) const
{
	return needRedraw;
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
