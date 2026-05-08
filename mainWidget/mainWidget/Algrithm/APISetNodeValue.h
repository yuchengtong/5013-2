#pragma once
#include "occView.h"
#include <MeshVS_DataMapOfIntegerColor.HXX>

class APISetNodeValue
{
public:
	static void HSVtoRGB(double h, double s, double v, double& r, double& g, double& b);
	static MeshVS_DataMapOfIntegerColor GetMeshDataMap(std::vector<double> tt, double min, double max);
	//µøÂä
	//static bool SetFallStressResult(OccView* occView, std::vector<double>& nodeValues);
	
	static bool SetPreForwardDesignResult0(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPreForwardDesignResult1(OccView* occView, std::vector<double>& nodeValues);
	static bool SetPreForwardDesignResult2(OccView* occView, std::vector<double>& nodeValues);

};