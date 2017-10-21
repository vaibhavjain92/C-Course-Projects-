/* ////////////////////////////////////////////////////////////

File Name: main.cpp
Copyright (c) 2017 Soji Yamakawa.  All rights reserved.
http://www.ysflight.com

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS 
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//////////////////////////////////////////////////////////// */

#include <stdio.h>
#include <vector>
#include <fslazywindow.h>
#include <ysgl.h>

#include "renderer.h"
#include "glutil.h"


class FsLazyWindowApplication : public FsLazyWindowApplicationBase
{
protected:
	bool needRedraw;
	BezierRenderer bez;

	YsMatrix4x4 Rc;
	double d;
	YsVec3 t;

	YsVec3 ctp[16];	// Cubic Bezier Patch control points.
	std::vector<float> ctrlpts;
	std::vector<float> param;
	YsVec3 CubicBezierCurve(const YsVec3 ctp[4],double s) const;
	YsVec3 CubicBezierSurface(const YsVec3 ctp[16],double s,double t) const;
	void GetCubicBezierBoundingBox(const YsVec3 ctp[16],YsVec3 bbx[2]) const;

	std::vector <float> vtx,nom,col;
	YsVec3 bbx[2];

	void RemakeVertexArray(void);
	YsMatrix4x4 GetProjection(void) const;
	YsMatrix4x4 GetModelView(void) const;

	void MakeCtrlPts()
	{
		for (int i = 0; i < 16; i++)
		{
			ctrlpts.push_back(ctp[i].xf());
			ctrlpts.push_back(ctp[i].yf());
			ctrlpts.push_back(ctp[i].zf());
		}
	};

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

YsVec3 FsLazyWindowApplication::CubicBezierCurve(const YsVec3 ctp[4],double s) const
{
	// Recursive form of Cubic Bezier Curve.
	// (1) Interpolate ctp[0-1], ctp[1-2], and ctp[2-3] with parameter s -> a[0], a[1], a[2]
	// (2) Interpolate a[0-1], a[1-2] with parameter s -> b[0], b[1]
	// (3) Interpolate b[0-1]
	// Easy to remember.  You don't have to remember the Bezier parameter equation.
	const YsVec3 a[3]=
	{
		ctp[0]*(1.0-s)+ctp[1]*s,
		ctp[1]*(1.0-s)+ctp[2]*s,
		ctp[2]*(1.0-s)+ctp[3]*s,
	};
	const YsVec3 b[2]=
	{
		a[0]*(1.0-s)+a[1]*s,
		a[1]*(1.0-s)+a[2]*s,
	};
	return b[0]*(1.0-s)+b[1]*s;
}

YsVec3 FsLazyWindowApplication::CubicBezierSurface(const YsVec3 ctp[16],double s,double t) const
{
	// Calculating a point on a Cubic Bezier Patch.
	// (1) First, calculate a point on four Bezier curves defined by ctp[0-3], [4-7], [8-11], [12-15], and parameter s -> a[0], a[1], a[2], a[3]
	// (2) Calculate on a point on the Bezier curve defined by a[0-4] and parameter t.
	// Easy to remember.  You don't have to remember the Bezier parameter equation.
	const YsVec3 a[4]=
	{
		CubicBezierCurve(ctp   ,s),
		CubicBezierCurve(ctp +4,s),
		CubicBezierCurve(ctp +8,s),
		CubicBezierCurve(ctp+12,s),
	};
	return CubicBezierCurve(a,t);
}

void FsLazyWindowApplication::GetCubicBezierBoundingBox(const YsVec3 ctp[16],YsVec3 bbx[2]) const
{
	YsBoundingBoxMaker3 mkBbx;
	for(int i=0; i<=100; i+=5)
	{
		const double s=(double)i/100.0;
		for(int j=0; j<=100; j+=5)
		{
			const double t=(double)j/100.0;
			mkBbx.Add(CubicBezierSurface(ctp,s,t));
		}
	}
	mkBbx.Get(bbx);
}

void FsLazyWindowApplication::RemakeVertexArray(void)
{
	param.clear();

	for(int i=0; i<100; i+=5)
	{
		const double s0=(double)i/100.0,s1=(double)(i+5)/100.0;
		for(int j=0; j<100; j+=5)
		{
			const double t0=(double)j/100.0,t1=(double)(j+5)/100.0;
			const double triParam[2][6]=
			{
				{s0,t0,s1,t0,s0,t1},
				{s1,t0,s1,t1,s0,t1}
			};

			for(int k=0; k<2; ++k)
			{
				param.push_back((float)triParam[k][0]);
				param.push_back((float)triParam[k][1]);
				param.push_back((float)triParam[k][2]);
				param.push_back((float)triParam[k][3]);
				param.push_back((float)triParam[k][4]);
				param.push_back((float)triParam[k][5]);
				
			}
		}
	}
}

YsMatrix4x4 FsLazyWindowApplication::GetProjection(void) const
{
	int wid,hei;
	FsGetWindowSize(wid,hei);
	auto aspect=(double)wid/(double)hei;
	return MakePerspective(45.0,aspect,d/10.0,d*2.0);
}

YsMatrix4x4 FsLazyWindowApplication::GetModelView(void) const
{
	YsMatrix4x4 globalToCamera=Rc;
	globalToCamera.Invert();

	YsMatrix4x4 modelView;
	modelView.Translate(0,0,-d);
	modelView*=globalToCamera;
	modelView.Translate(-t);
	return modelView;
}

FsLazyWindowApplication::FsLazyWindowApplication()
{
	needRedraw=false;

	d=10.0;
	t=YsVec3::Origin();
}

/* virtual */ void FsLazyWindowApplication::BeforeEverything(int argc,char *argv[])
{
	ctp[ 0].Set(0.0,0.0,3.0);
	ctp[ 1].Set(1.0,0.5,3.0);
	ctp[ 2].Set(2.0,0.5,3.0);
	ctp[ 3].Set(3.0,0.0,3.0);
	ctp[ 4].Set(0.0,0.0,2.0);
	ctp[ 5].Set(1.0,2.0,2.0);
	ctp[ 6].Set(2.0,2.0,2.0);
	ctp[ 7].Set(3.0,0.0,2.0);
	ctp[ 8].Set(0.0,0.0,1.0);
	ctp[ 9].Set(1.0,2.0,1.0);
	ctp[10].Set(2.0,2.0,1.0);
	ctp[11].Set(3.0,0.0,1.0);
	ctp[12].Set(0.0,0.0,0.0);
	ctp[13].Set(1.0,0.5,0.0);
	ctp[14].Set(2.0,0.5,0.0);
	ctp[15].Set(3.0,0.0,0.0);

	MakeCtrlPts();
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
	FsChangeToProgramDir();

	RemakeVertexArray();
	GetCubicBezierBoundingBox(ctp,bbx);

	t=(bbx[0]+bbx[1])/2.0;
	d=(bbx[1]-bbx[0]).GetLength()*1.2;

	printf("Target %s\n",t.Txt());
	printf("Diagonal %lf\n",d);

	bez.CompileFile("bezier_vertexShader.glsl",
					"bezier_fragmentShader.glsl");
}
/* virtual */ void FsLazyWindowApplication::Interval(void)
{
	auto key=FsInkey();
	if(FSKEY_ESC==key)
	{
		SetMustTerminate(true);
	}

	if(FsGetKeyState(FSKEY_LEFT))
	{
		Rc.RotateXZ(YsPi/60.0);
	}
	if(FsGetKeyState(FSKEY_RIGHT))
	{
		Rc.RotateXZ(-YsPi/60.0);
	}
	if(FsGetKeyState(FSKEY_UP))
	{
		Rc.RotateYZ(YsPi/60.0);
	}
	if(FsGetKeyState(FSKEY_DOWN))
	{
		Rc.RotateYZ(-YsPi/60.0);
	}

	needRedraw=true;
}
/* virtual */ void FsLazyWindowApplication::Draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	int wid,hei;
	FsGetWindowSize(wid,hei);
	glViewport(0,0,wid,hei);

    glClear(GL_DEPTH_BUFFER_BIT);

	YsMatrix4x4 projection=GetProjection();
	GLfloat projectionGl[16];
	projection.GetOpenGlCompatibleMatrix(projectionGl);

	glUseProgram(bez.programIdent);
	glUniformMatrix4fv(bez.uniformProjectionPos, 1, GL_FALSE, projectionGl);
	
	YsMatrix4x4 modelView=GetModelView();
	GLfloat modelViewGl[16];
	modelView.GetOpenGlCompatibleMatrix(modelViewGl);
	
	glUniformMatrix4fv(bez.uniformModelViewPos, 1, GL_FALSE, modelViewGl);
	glUniform3fv(bez.uniformCtrlPtsPos, 16, ctrlpts.data());
	
	glEnableVertexAttribArray(bez.attribParamPos);

	glVertexAttribPointer(bez.attribParamPos, 2, GL_FLOAT, GL_FALSE, 0, param.data());
	
	glDrawArrays(GL_TRIANGLES,0,param.size()/2);
    
	glDisableVertexAttribArray(bez.attribParamPos);

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

static FsLazyWindowApplication *appPtr=nullptr;

/* static */ FsLazyWindowApplicationBase *FsLazyWindowApplicationBase::GetApplication(void)
{
	if(nullptr==appPtr)
	{
		appPtr=new FsLazyWindowApplication;
	}
	return appPtr;
}
