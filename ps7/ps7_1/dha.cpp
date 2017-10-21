#include "dha.h"
#include<stdlib.h>
#include <unordered_set>

// Input parameters are a mesh and dihedral-angle threshold.
// The return value is a map from a polygon search key to a segment identifier.
std::unordered_map <YSHASHKEY, int> MakeDihedralAngleBasedSegmentation(const YsShellExt &mesh, const double dhaThr)
{
	std::unordered_map <YSHASHKEY, int> faceGrp;
	std::unordered_set <unsigned int> visited;
	std::vector <YsShell::PolygonHandle> connPlHd;
	int count = 1;

	for (auto plHd : mesh.AllPolygon())
	{
		if (faceGrp.find(mesh.GetSearchKey(plHd)) != faceGrp.end())
		{
			continue;
		}
		connPlHd.push_back(plHd);
		faceGrp[mesh.GetSearchKey(plHd)] = count;
		visited.insert(mesh.GetSearchKey(plHd));

		for (int i = 0; i < connPlHd.size(); ++i)
		{
			auto plHd = connPlHd[i];
			
			auto plVtHd = mesh.GetPolygonVertex(plHd);
			for (int e = 0; e < plVtHd.size(); ++e)
			{
				auto neiPlHd = mesh.GetNeighborPolygon(plHd, e);

				if (mesh.NullPolygon() != neiPlHd && visited.end() == visited.find(mesh.GetSearchKey(neiPlHd))
					&& faceGrp.find(mesh.GetSearchKey(neiPlHd)) == faceGrp.end())
				{
					if (fabs(mesh.ComputeDihedralAngle(plHd, neiPlHd)) <= dhaThr)
					{
						connPlHd.push_back(neiPlHd);
						faceGrp[mesh.GetSearchKey(neiPlHd)] = count;
						visited.insert(mesh.GetSearchKey(neiPlHd));
					}
					
				}
			}
		}
		count++;
	}
	return faceGrp;
}

// Input parameters are a mesh and the segmentation (face grouping) obtained from MakeDihedralAngleBasedSegmentaion.
// Output is a vertex array that can be drawn as GL_LINES.
std::vector <float> MakeGroupBoundaryVertexArray(const YsShellExt &mesh,const std::unordered_map <YSHASHKEY,int> &faceGroupInfo)
{
	std::vector <float> vtxArray;
	for (auto plHd : mesh.AllPolygon())
	{
		auto plVtHd = mesh.GetPolygonVertex(plHd);
		for (int i = 0; i < plVtHd.size(); ++i)
		{
			int j = 0;
			if (i < plVtHd.size() - 1)
			{
				j = i + 1;
			}
			else
			{
				j = 0;
			}
			
			auto neiPlHd = mesh.GetNeighborPolygon(plHd, plVtHd[i], plVtHd[j]);
			
			
			if (faceGroupInfo.at(mesh.GetSearchKey(plHd)) != faceGroupInfo.at(mesh.GetSearchKey(neiPlHd)))
			{
				
				YsVec3 v1 = mesh.GetVertexPosition(plVtHd[i]);
				YsVec3 v2 = mesh.GetVertexPosition(plVtHd[j]);
				
				vtxArray.push_back(v1.x());
				vtxArray.push_back(v1.y());
				vtxArray.push_back(v1.z());

				vtxArray.push_back(v2.x());
				vtxArray.push_back(v2.y());
				vtxArray.push_back(v2.z());
			}
		}

	}
	return vtxArray;
}

// For bonus questions:
// Input parameters are a mesh and the segmentation (face grouping) obtained from MakeDihedralAngleBasedSegmentaion.
// Paint polygons so that no two neighboring face groups have a same color.
void MakeFaceGroupColorMap(YsShellExt &mesh,const std::unordered_map <YSHASHKEY,int> &faceGroupInfo)
{
}

