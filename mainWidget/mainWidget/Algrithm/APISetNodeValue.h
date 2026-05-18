#pragma once
#include "occView.h"
#include <MeshVS_DataMapOfIntegerColor.HXX>

class APISetNodeValue
{
public:
    struct ModelEdge {
        gp_Pnt p1, p2;
        bool isTopEdge;     // 上边线（Y最大）
        bool isRightEdge;   // 右边线（X最大）
        bool isBottomEdge;  // 下边线（Y最小）
    };

    struct FrameParams {
        double jet_length_ratio;      // 射流长度占内腔宽度比例
        double jet_core_ratio;        // 射流核心宽度比例
        double jet_transition_ratio;  // 过渡区宽度比例
        bool has_interface_wave;      // 是否有界面波动
        double wave_amp_ratio;        // 涡旋幅度
        double liquid_fill_ratio;     // 液相填充位置
        double liquid_core_ratio;     // 液相核心区值比例
        bool has_liquid_fill;         // 是否有液相填充
    };

    static FrameParams frames[12];
    static std::vector<ModelEdge> ExtractAndClassifyEdges(const TopoDS_Shape& shape);
    static bool IsInWallRegion(double x, double y, const std::vector<ModelEdge>& edges, double wallThickness);
    static double PointToSegmentDistance(double x, double y, const gp_Pnt& a, const gp_Pnt& b);
    static void HSVtoRGB(double h, double s, double v, double& r, double& g, double& b);
    static MeshVS_DataMapOfIntegerColor GetMeshDataMap(std::vector<double> tt, double min, double max);

    static bool SetPreForwardDesignResult(OccView* occView, std::vector<double>& nodeValues, int frame);
};