#include "astar.h"

double GetDistance(const YsShellExt &shl,
	YsShellExt::VertexHandle startVtHd, YsShellExt::VertexHandle goalVtHd)
{
	YsVec3 d1 = shl.GetVertexPosition(startVtHd);
	YsVec3 d2 = shl.GetVertexPosition(goalVtHd);

	return ((d2 - d1).GetLength());
}

std::vector <YsShellExt::VertexHandle> A_Star(const YsShellExt &shl,
	YsShellExt::VertexHandle startVtHd, YsShellExt::VertexHandle goalVtHd)
{
	//std::unordered_map <YSHASHKEY, int> closedSet;
	//std::unordered_map <YSHASHKEY, int> openSet;

	std::unordered_map <YSHASHKEY, YsShellExt::VertexHandle> cameFrom;
	std::unordered_map <YSHASHKEY, double> g_score;
	std::unordered_map <YSHASHKEY, double> f_score;
	std::unordered_set <YSHASHKEY> open_set;
	std::unordered_set <YSHASHKEY> closed_set;

	open_set.insert(shl.GetSearchKey(startVtHd));

	for (auto vtHd : shl.AllVertex())
	{

		g_score[shl.GetSearchKey(vtHd)] = std::numeric_limits<double>::max();
		f_score[shl.GetSearchKey(vtHd)] = std::numeric_limits<double>::max();
	}
	g_score[shl.GetSearchKey(startVtHd)] = 0;
	f_score[shl.GetSearchKey(startVtHd)] = GetDistance(shl, startVtHd, goalVtHd);

	while (open_set.size() > 0)
	{
		double minValue = std::numeric_limits<double>::max();
		YSHASHKEY current;
		for (const auto key : open_set)
		{
			if (f_score[key] < minValue)
			{
				minValue = f_score[key];
				current = key;
			}
		}
		auto newStartVertex = shl.FindVertex(current);

		if (current == shl.GetSearchKey(goalVtHd))
		{
			return A_Star_ReconstructPath(shl, cameFrom, newStartVertex);
		}
		open_set.erase(current);
		closed_set.insert(current);

		for (auto neighbor : shl.GetConnectedVertex(newStartVertex))
		{
			YSHASHKEY newKey = shl.GetSearchKey(neighbor);
			if (closed_set.end() != closed_set.find(newKey))
			{
				continue;
			}
			double tentative_gScore = g_score[current] +
				GetDistance(shl, newStartVertex, neighbor);
			if (open_set.find(newKey) == open_set.end())
			{
				open_set.insert(newKey);
			}
			else if (tentative_gScore >= g_score[newKey])
			{
				continue;
			}
			cameFrom[newKey] = newStartVertex;
			g_score[newKey] = tentative_gScore;
			f_score[newKey] = g_score[newKey] + GetDistance(shl, neighbor, goalVtHd);

		}	
	}
	return std::vector <YsShellExt::VertexHandle>();
}
std::vector <YsShellExt::VertexHandle> A_Star_ReconstructPath(const YsShellExt &shl,
	const std::unordered_map <YSHASHKEY,YsShellExt::VertexHandle> &cameFrom,YsShellExt::VertexHandle current)
{
	std::vector <YsShellExt::VertexHandle> path;
	path.push_back(current);
	while (cameFrom.find(shl.GetSearchKey(current)) != cameFrom.end())
	{
		current = cameFrom.at(shl.GetSearchKey(current));
		path.push_back(current);
		
	}
	return path;
}
