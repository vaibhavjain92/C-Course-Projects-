#include <fssimplewindow.h> // For FsSubSecondTimer
#include <meshlattice.h>
#include <vector>
#include "polygonalmesh.h"
#include "ps6lib.h"
#include <unordered_set>

std::vector <float> MakeSliceVertexArray(const YsShellExt &mesh)
{
	auto t0=FsSubSecondTimer();
	std::vector <float> sliceVtx;

	YsVec3 bbx[2];
	mesh.GetBoundingBox(bbx[0], bbx[1]);
	float minY = min(bbx[0].y(), bbx[1].y());
	float maxY = max(bbx[0].y(), bbx[1].y());
	float planeInterval = (maxY - minY) / 99;
	
	YsVec3 crs1, crs2, crs3;
		
	for (auto plHd : mesh.AllPolygon()) {
		auto plVtHd = mesh.GetPolygonVertex(plHd);
			
		YsVec3 v1 = mesh.GetVertexPosition(plVtHd[0]);
		YsVec3 v2 = mesh.GetVertexPosition(plVtHd[1]);
		YsVec3 v3 = mesh.GetVertexPosition(plVtHd[2]);
			
		double Ymin = min(v1.y(), min(v2.y(), v3.y()));
		double Ymax = max(v1.y(), max(v2.y(), v3.y()));
			
		for (double i = minY; i < maxY; i += planeInterval) {
			YsPlane pln(YsVec3(0, i, 0), YsVec3(0, 1, 0));
			if (i >= Ymin && i <= Ymax) {
				auto res1 = pln.GetPenetration(crs1, v1, v2);
				auto res2 = pln.GetPenetration(crs2, v2, v3);
				auto res3 = pln.GetPenetration(crs3, v1, v3);
				int count = 0;
				if (res1 == YSOK) { count++; }
				if (res2 == YSOK) { count++; }
				if (res3 == YSOK) { count++; }
				if (count == 2) {
					if (res1 == YSOK) { sliceVtx.push_back(crs1.x()); sliceVtx.push_back(crs1.y()); sliceVtx.push_back(crs1.z()); }
					if (res2 == YSOK) { sliceVtx.push_back(crs2.x()); sliceVtx.push_back(crs2.y()); sliceVtx.push_back(crs2.z()); }
					if (res3 == YSOK) { sliceVtx.push_back(crs3.x()); sliceVtx.push_back(crs3.y()); sliceVtx.push_back(crs3.z()); }
				}
			}
		}
	}
	printf("%d milli seconds\n",(int)FsSubSecondTimer()-t0);
	return sliceVtx;
}

#define USE_LATTICE


std::vector <float> MakeRayIntersectionVertexArray(const YsShellExt &mesh)
{
	auto t0=FsSubSecondTimer();
	std::vector <float> rayItscVtx;
	
	YsVec3 bbx[2];
	mesh.GetBoundingBox(bbx[0], bbx[1]);
	float minX = min(bbx[0].x(), bbx[1].x());
	float maxX = max(bbx[0].x(), bbx[1].x());

	float minY = min(bbx[0].y(), bbx[1].y());
	float maxY = max(bbx[0].y(), bbx[1].y());
	
	float minZ = min(bbx[0].z(), bbx[1].z());
	float maxZ = max(bbx[0].z(), bbx[1].z());

	float intervalX = (maxX - minX) / 99;
	float intervalY = (maxY - minY) / 99;

	YsVec3 crs;

	for (auto plHd : mesh.AllPolygon()) 
	{
		auto vtxList = mesh.GetPolygonVertexPosition(plHd);
		
		double Xmin = min(vtxList[0].x(), min(vtxList[1].x(), vtxList[2].x()));
		double Xmax = max(vtxList[0].x(), max(vtxList[1].x(), vtxList[2].x()));

		double Ymin = min(vtxList[0].y(), min(vtxList[1].y(), vtxList[2].y()));
		double Ymax = max(vtxList[0].y(), max(vtxList[1].y(), vtxList[2].y()));
		
		YsPlane polyPlane(vtxList[0], mesh.GetNormal(plHd));
		double startx = ceil((Xmin - minX) / intervalX) * intervalX + minX;
		double endx = floor((Xmax - minX) / intervalX) * intervalX + minX;

		double starty = ceil((Ymin - minY) / intervalY) * intervalY + minY;
		double endy = floor((Ymax - minY) / intervalY) * intervalY + minY;

		for (double i = startx; i <= endx; i += intervalX)
		{
			for (double j = starty; j <= endy; j += intervalY)
			{
				YsVec3 v1(i, j, minZ-1);
				YsVec3 v2(i, j, maxZ+1);
				auto res = polyPlane.GetPenetration(crs, v1, v2);
				if (res == YSOK) 
				{
					if ((YSINSIDE == YsCheckInsidePolygon3(crs, vtxList)) ||
						(YSBOUNDARY == YsCheckInsidePolygon3(crs, vtxList))) 
					{
						rayItscVtx.push_back(crs.x());
						rayItscVtx.push_back(crs.y());
						rayItscVtx.push_back(crs.z());
					}
					
				}
			}

		}
	}
	printf("%d milli seconds\n",(int)FsSubSecondTimer()-t0);
	return rayItscVtx;
}

void Paint(YsShellExt &mesh,YsShell::PolygonHandle from,const double angleTolerance) 
{
	std::unordered_set <unsigned int> visited;
	std::vector <YsShell::PolygonHandle> connPlHd;
	connPlHd.push_back(from);
	mesh.SetPolygonColor(from, YsRed());
	visited.insert(mesh.GetSearchKey(from));

	YsVec3 plNom = mesh.GetNormal(from);
	
	for (int i = 0; i < connPlHd.size(); ++i)
	{
		auto plHd = connPlHd[i];
		auto plVtHd = mesh.GetPolygonVertex(plHd);
		for (int e = 0; e < plVtHd.size(); ++e)
		{
			auto neiPlHd = mesh.GetNeighborPolygon(plHd, e);
			
			if (mesh.NullPolygon() != neiPlHd && visited.end() == visited.find(mesh.GetSearchKey(neiPlHd)))
			{
				YsVec3 neiPlNom = mesh.GetNormal(neiPlHd);

				if (acos(plNom*neiPlNom) < angleTolerance*(YsPi/180.0)) 
				{
					connPlHd.push_back(neiPlHd);
					mesh.SetPolygonColor(neiPlHd, YsRed());
				}
				visited.insert(mesh.GetSearchKey(neiPlHd));
			}
		}
	}
}
