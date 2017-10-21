#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <ysclass.h>

#include <fslazywindow.h>

#include <ysshellext.h>

#include "glutil.h"
#include <dha.h>


////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////

class FsLazyWindowApplication : public FsLazyWindowApplicationBase
{
protected:
	bool needRedraw;

	YsMatrix4x4 Rc;
	double d;
	YsVec3 t;

	YsShellExt mesh;
	std::vector <float> vtx,nom,col;
	YsVec3 bbx[2];

	std::vector <float> fgBoundaryVtx;

	void RemakeVertexArray(void);

	YsMatrix4x4 GetProjection(void) const;
	YsMatrix4x4 GetModelView(void) const;
	YsShellExt::PolygonHandle PickedPlHd(int mx,int my) const;
	YsShellExt::VertexHandle PickedVtHd(int mx,int my,int pixRange) const;

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

void FsLazyWindowApplication::RemakeVertexArray(void)
{
	vtx.clear();
	nom.clear();
	col.clear();

	for(auto plHd=mesh.NullPolygon(); true==mesh.MoveToNextPolygon(plHd); )
	{
		auto plVtHd=mesh.GetPolygonVertex(plHd);
		auto plCol=mesh.GetColor(plHd);
		auto plNom=mesh.GetNormal(plHd);

		// Let's assume every polygon is a triangle for now.
		if(3==plVtHd.size())
		{
			for(int i=0; i<3; ++i)
			{
				auto vtPos=mesh.GetVertexPosition(plVtHd[i]);
				vtx.push_back(vtPos.xf());
				vtx.push_back(vtPos.yf());
				vtx.push_back(vtPos.zf());
				nom.push_back(plNom.xf());
				nom.push_back(plNom.yf());
				nom.push_back(plNom.zf());
				col.push_back(plCol.Rf());
				col.push_back(plCol.Gf());
				col.push_back(plCol.Bf());
				col.push_back(plCol.Af());
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

YsShellExt::PolygonHandle FsLazyWindowApplication::PickedPlHd(int mx,int my) const
{
	int wid,hei;
	FsGetWindowSize(wid,hei);
	auto vp=WindowToViewPort(wid,hei,mx,my);

	auto perspective=GetProjection();
	auto modelView=GetModelView();

	YsMatrix4x4 all=perspective*modelView;
	all.Invert();

	auto nearPos=vp,farPos=vp;
	nearPos.SetZ(-1.0);
	farPos.SetZ(1.0);

	YsVec3 ln[2];
	all.Mul(ln[0],nearPos,1.0);
	all.Mul(ln[1],farPos,1.0);

	YsShellExt::PolygonHandle pickedPlHd=mesh.NullPolygon();
	double pickedZ=0.0;
	for(auto plHd=mesh.NullPolygon(); true==mesh.MoveToNextPolygon(plHd); )
	{
		std::vector <YsVec3> plVtPos;
		for(auto vtHd : mesh.GetPolygonVertex(plHd))
		{
			plVtPos.push_back(mesh.GetVertexPosition(vtHd));
		}
		YsPlane pln;
		YsVec3 crs;
		if(YSOK==pln.MakeBestFitPlane(plVtPos) &&
		   YSOK==pln.GetPenetration(crs,ln[0],ln[1]) &&
		   YSINSIDE==YsCheckInsidePolygon3(crs,plVtPos))
		{
			crs=modelView*crs;
			if(mesh.NullPolygon()==pickedPlHd || crs.z()>pickedZ)
			{
				pickedPlHd=plHd;
				pickedZ=crs.z();
			}
		}
	}

	return pickedPlHd;
}

YsShellExt::VertexHandle FsLazyWindowApplication::PickedVtHd(int mx,int my,int pixRange) const
{
	int wid,hei;
	FsGetWindowSize(wid,hei);
	auto vp=WindowToViewPort(wid,hei,mx,my);

	auto projection=GetProjection();
	auto modelView=GetModelView();

	double pickedZ=0.0;
	auto pickedVtHd=mesh.NullVertex();
	for(auto vtHd=mesh.NullVertex(); true==mesh.MoveToNextVertex(vtHd); )
	{
		auto vtPos=mesh.GetVertexPosition(vtHd);
		vtPos=projection*modelView*vtPos;
		auto winPos=ViewPortToWindow(wid,hei,vtPos);
		int dx=(mx-winPos.x()),dy=(my-winPos.y());
		if(-pixRange<=dx && dx<=pixRange && -pixRange<=dy && dy<=pixRange)
		{
			if(mesh.NullVertex()==pickedVtHd || vtPos.z()<pickedZ)
			{
				pickedVtHd=vtHd;
				pickedZ=vtPos.z();
			}
		}
	}

	return pickedVtHd;
}

FsLazyWindowApplication::FsLazyWindowApplication()
{
	needRedraw=false;
	d=10.0;
	t=YsVec3::Origin();
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
	if(2<=argc && true==mesh.LoadStl(argv[1]))
	{
		mesh.EnableSearch();

		mesh.GetBoundingBox(bbx[0],bbx[1]);

		t=(bbx[0]+bbx[1])/2.0;
		d=(bbx[1]-bbx[0]).GetLength()*1.2;

		printf("Target %s\n",t.Txt());
		printf("Diagonal %lf\n",d);

		if(3<=argc)
		{
			auto fg=MakeDihedralAngleBasedSegmentation(mesh,atof(argv[2])*YsPi/180.0);
			fgBoundaryVtx=MakeGroupBoundaryVertexArray(mesh,fg);
			MakeFaceGroupColorMap(mesh,fg);
		}

		RemakeVertexArray();
	}
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


	int mx,my,lb,mb,rb;
	int evt=FsGetMouseEvent(lb,mb,rb,mx,my);
	if(FSMOUSEEVENT_LBUTTONDOWN==evt)
	{
	}

	needRedraw=true;
}
/* virtual */ void FsLazyWindowApplication::Draw(void)
{
	needRedraw=false;

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	int wid,hei;
	FsGetWindowSize(wid,hei);
	auto aspect=(double)wid/(double)hei;
	glViewport(0,0,wid,hei);

	YsMatrix4x4 projection=GetProjection();
	GLfloat projectionGl[16];
	projection.GetOpenGlCompatibleMatrix(projectionGl);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMultMatrixf(projectionGl);

	YsMatrix4x4 globalToCamera=Rc;
	globalToCamera.Invert();

	YsMatrix4x4 modelView=GetModelView();

	GLfloat modelViewGl[16];
	modelView.GetOpenGlCompatibleMatrix(modelViewGl);

	glShadeModel(GL_SMOOTH);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	GLfloat lightDir[]={0.0f,1.0f/(float)sqrt(2.0f),1.0f/(float)sqrt(2.0f),0.0f};
	glLightfv(GL_LIGHT0,GL_POSITION,lightDir);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glMultMatrixf(modelViewGl);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4,GL_FLOAT,0,col.data());
	glNormalPointer(GL_FLOAT,0,nom.data());
	glVertexPointer(3,GL_FLOAT,0,vtx.data());
	glDrawArrays(GL_TRIANGLES,0,vtx.size()/3);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glLineWidth(4);
	glDisable(GL_LIGHTING);
	glColor3ub(0,0,255);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,0,fgBoundaryVtx.data());
	glDrawArrays(GL_LINES,0,fgBoundaryVtx.size()/3);
	glDisableClientState(GL_VERTEX_ARRAY);

	FsSwapBuffers();
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
