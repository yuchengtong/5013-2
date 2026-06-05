#include "APISetNodeValue.h"
#include "ModelDataManager.h"
#include <algorithm>
#include <cmath>
#include <MeshVS_Mesh.hxx>
#include <MeshVS_DataMapOfIntegerColor.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_Drawer.hxx>



#include <AIS_Shape.hxx>
#include <AIS_ColorScale.hxx>

#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRep_Builder.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAlgo_FaceRestrictor.hxx>
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <BRepProj_Projection.hxx>
#include <BRepGProp.hxx>


#include <GProp_GProps.hxx>

#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>

#include <MeshVS_Mesh.hxx>
#include <MeshVS_Drawer.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_MeshPrsBuilder.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>

#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Drawer.hxx>
#include <Quantity_ColorRGBA.hxx>

#include <RWStl.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI_Reader.hxx>

#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopExp_Explorer.hxx>
#include <TColStd_HArray2OfInteger.hxx>
#include <TColStd_HArray2OfReal.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <BRepAlgoAPI_Fuse.hxx>



APISetNodeValue::FrameParams APISetNodeValue::frames[12] = {
	{0.20, 0.008, 0.025, false, 0.00, 0.00, 0.00, false},
	{0.35, 0.008, 0.025, false, 0.00, 0.00, 0.00, false},
	{0.50, 0.010, 0.025, false, 0.00, 0.00, 0.00, false},
	{0.65, 0.010, 0.025, false, 0.00, 0.00, 0.00, false},
	{0.80, 0.010, 0.025, false, 0.00, 0.00, 0.00, false},
	{0.95, 0.010, 0.025, false, 0.00, 0.00, 0.00, false},

	{0.95, 0.022, 0.055, true,  0.18, 0.80, 1.00, true},
	{0.95, 0.020, 0.050, true,  0.14, 0.70, 0.98, true},
	{0.95, 0.018, 0.045, true,  0.10, 0.60, 0.96, true},
	{0.95, 0.016, 0.040, true,  0.08, 0.50, 0.94, true},
	{0.95, 0.014, 0.035, true,  0.06, 0.40, 0.92, true},
	{0.95, 0.012, 0.030, true,  0.04, 0.30, 0.90, true},
};

// ==================== 提取并分类边线 ====================
std::vector<APISetNodeValue::ModelEdge> APISetNodeValue::ExtractAndClassifyEdges(const TopoDS_Shape& shape) {
	std::vector<APISetNodeValue::ModelEdge> edges;

	double minX = 999999, maxX = -999999;
	double minY = 999999, maxY = -999999;

	TopExp_Explorer exp1(shape, TopAbs_EDGE);
	for (; exp1.More(); exp1.Next()) {
		TopoDS_Edge edge = TopoDS::Edge(exp1.Current());
		TopoDS_Vertex v1, v2;
		TopExp::Vertices(edge, v1, v2);
		gp_Pnt p1 = BRep_Tool::Pnt(v1);
		gp_Pnt p2 = BRep_Tool::Pnt(v2);

		minX = std::min(minX, std::min(p1.X(), p2.X()));
		maxX = std::max(maxX, std::max(p1.X(), p2.X()));
		minY = std::min(minY, std::min(p1.Y(), p2.Y()));
		maxY = std::max(maxY, std::max(p1.Y(), p2.Y()));
	}

	const double tol = std::max(1e-3, std::max(maxX - minX, maxY - minY) * 0.005);
	const double midTol = tol * 2.0;
	const double dupTol = 1e-6;

	TopExp_Explorer exp2(shape, TopAbs_EDGE);
	for (; exp2.More(); exp2.Next()) {
		TopoDS_Edge edge = TopoDS::Edge(exp2.Current());
		TopoDS_Vertex v1, v2;
		TopExp::Vertices(edge, v1, v2);
		gp_Pnt p1 = BRep_Tool::Pnt(v1);
		gp_Pnt p2 = BRep_Tool::Pnt(v2);

		double edgeMinX = std::min(p1.X(), p2.X());
		double edgeMaxX = std::max(p1.X(), p2.X());
		double edgeMinY = std::min(p1.Y(), p2.Y());
		double edgeMaxY = std::max(p1.Y(), p2.Y());
		double dx = std::fabs(p1.X() - p2.X());
		double dy = std::fabs(p1.Y() - p2.Y());

		// 快速剔除：外接矩形完全在内部
		if (edgeMinX > minX + tol && edgeMaxX < maxX - tol &&
			edgeMinY > minY + tol && edgeMaxY < maxY - tol) {
			continue;
		}

		// 【关键】垂直边（平行于Y轴）硬性过滤：必须经过 X 最大的点
		// 即：只有贴在最右边界上的垂直壁才保留，内部垂直加强筋全部剔除
		if (dx < tol && edgeMaxX < maxX - tol) {
			continue; // 内部垂直边，过滤
		}

		ModelEdge me;
		me.p1 = p1;
		me.p2 = p2;
		me.isTopEdge = false;
		me.isRightEdge = false;
		me.isBottomEdge = false;

		// 左侧入口边（X最小）：排除
		if (edgeMaxX < minX + tol) {
			continue;
		}
		// 底部边（X最大）
		else if (edgeMinX > maxX - tol) {
			me.isRightEdge = true;
		}
		// 右侧壁（Y最大）
		else if (edgeMaxY > maxY - tol) {
			me.isTopEdge = true;
		}
		// 左侧壁（Y最小）
		else if (edgeMinY < minY + tol) {
			me.isBottomEdge = true;
		}
		// 倾斜外轮廓边
		else {
			double midX = (p1.X() + p2.X()) / 2.0;
			double midY = (p1.Y() + p2.Y()) / 2.0;

			bool midOnBoundary = (midX > maxX - midTol) ||
				(midY > maxY - midTol) ||
				(midY < minY + midTol);
			if (!midOnBoundary) continue;

			bool hasOuter = (p1.X() > maxX - tol) || (p2.X() > maxX - tol) ||
				(p1.Y() > maxY - tol) || (p2.Y() > maxY - tol) ||
				(p1.Y() < minY + tol) || (p2.Y() < minY + tol);
			if (!hasOuter) continue;

			if (midY > (minY + maxY) / 2.0) me.isTopEdge = true;
			else me.isBottomEdge = true;
		}

		edges.push_back(me);
	}

	// ==================== 几何去重 ====================
	auto arePointsEqual = [&](const gp_Pnt& a, const gp_Pnt& b) {
		return std::fabs(a.X() - b.X()) < dupTol && std::fabs(a.Y() - b.Y()) < dupTol;
	};

	std::vector<ModelEdge> uniqueEdges;
	for (const auto& e : edges) {
		bool isDup = false;
		for (const auto& u : uniqueEdges) {
			if ((arePointsEqual(e.p1, u.p1) && arePointsEqual(e.p2, u.p2)) ||
				(arePointsEqual(e.p1, u.p2) && arePointsEqual(e.p2, u.p1))) {
				isDup = true;
				break;
			}
		}
		if (!isDup) uniqueEdges.push_back(e);
	}

	return uniqueEdges;
}

// ==================== 判断点是否在壁厚区域 ====================
bool APISetNodeValue::IsInWallRegion(double x, double y,
	const std::vector<ModelEdge>& edges,
	double wallThickness) {
	for (const auto& edge : edges) 
	{
		if (edge.isTopEdge || edge.isRightEdge || edge.isBottomEdge) 
		{
			double dist = PointToSegmentDistance(x, y, edge.p1, edge.p2);
			if (dist < wallThickness) 
			{
				return true;
			}
		}
	}
	return false;
}


double APISetNodeValue::PointToSegmentDistance(double x, double y, const gp_Pnt& a, const gp_Pnt& b) {
	double dx = b.X() - a.X();
	double dy = b.Y() - a.Y();
	double len2 = dx * dx + dy * dy;

	if (len2 < 1e-10) {
		return sqrt((x - a.X()) * (x - a.X()) + (y - a.Y()) * (y - a.Y()));
	}

	double t = std::clamp(((x - a.X()) * dx + (y - a.Y()) * dy) / len2, 0.0, 1.0);

	double closestX = a.X() + t * dx;
	double closestY = a.Y() + t * dy;

	return sqrt((x - closestX) * (x - closestX) + (y - closestY) * (y - closestY));
}



struct Point {
	double x;
	double y;
};

void APISetNodeValue::HSVtoRGB(double h, double s, double v, double& r, double& g, double& b)
{
	if (s <= 0.0) {
		// 无饱和度时为灰度
		r = v;
		g = v;
		b = v;
		return;
	}

	// HSV转RGB核心计算
	double c = v * s;                  // 色度
	double x = c * (1.0 - std::fabs(std::fmod(h * 6.0, 2.0) - 1.0));
	double m = v - c;                  // 明度偏移值

	// 根据Hue值确定RGB分量
	double r_temp, g_temp, b_temp;
	if (h < 1.0 / 6.0) {
		r_temp = c;
		g_temp = x;
		b_temp = 0.0;
	}
	else if (h < 2.0 / 6.0) {
		r_temp = x;
		g_temp = c;
		b_temp = 0.0;
	}
	else if (h < 3.0 / 6.0) {
		r_temp = 0.0;
		g_temp = c;
		b_temp = x;
	}
	else if (h < 4.0 / 6.0) {
		r_temp = 0.0;
		g_temp = x;
		b_temp = c;
	}
	else if (h < 5.0 / 6.0) {
		r_temp = x;
		g_temp = 0.0;
		b_temp = c;
	}
	else {
		r_temp = c;
		g_temp = 0.0;
		b_temp = x;
	}

	// 应用明度偏移并限制范围
	r = std::clamp(r_temp + m, 0.0, 1.0);
	g = std::clamp(g_temp + m, 0.0, 1.0);
	b = std::clamp(b_temp + m, 0.0, 1.0);
}

MeshVS_DataMapOfIntegerColor APISetNodeValue::GetMeshDataMap(std::vector<double> tt, double min, double max)
{
	double a, r, g, b;
	MeshVS_DataMapOfIntegerColor colormap;
	int index = 0;

	if (max <= min) {
		Quantity_Color defaultColor(0.5, 0.5, 0.5, Quantity_TOC_RGB);
		for (size_t i = 0; i < tt.size(); ++i) {
			colormap.Bind(i + 1, defaultColor);
		}
		return colormap;
	}

	for (double t : tt)
	{
		a = (t - min) / (max - min);
		a = std::clamp(a, 0.0, 1.0);

		// 【修正】线性映射，保留完整黄绿青过渡
		// 如果仍希望红色稍宽，用 1.5 次方而非 2 次方
		// a = 1.0 - std::pow(1.0 - a, 1.5);
		// a = std::clamp(a, 0.0, 1.0);

		double h = 0.666 - a * 0.666;
		double s = 1.0;
		double v = 1.0;

		HSVtoRGB(h, s, v, r, g, b);

		if (t == -1.0) {
			colormap.Bind(index + 1, Quantity_Color(0.5, 0.5, 0.5, Quantity_TOC_RGB));
		}
		else {
			colormap.Bind(index + 1, Quantity_Color(r, g, b, Quantity_TOC_RGB));
		}
		index++;
	}
	return colormap;
}

bool APISetNodeValue::SetInForwardDesignResult(OccView* occView, std::vector<double>& nodeValues, int frame)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	const double WALL_THICKNESS = 10.0;
	const double WALL_VALUE = -1.0;
	const double max_value = ModelDataManager::GetInstance()->GetInForwardPropertyInfo().m_relativeDensityValue;
	const double min_value = 0.0;

	auto& boundaryEdges = meshInfo.preForwardBoundaryEdges;
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double y_min = meshInfo.y_min;
	const double y_max = meshInfo.y_max;
	const double y_center = meshInfo.preForwardYCenter;
	const double width = meshInfo.preForwardWidth;
	const double height = meshInfo.preForwardHeight;

	Handle(V3d_View) view = occView->getView();
	FrameParams fp = frames[frame];

	const double jet_length = height * fp.jet_length_ratio;
	const double jet_core_width = width * fp.jet_core_ratio;
	const double jet_transition = width * fp.jet_transition_ratio;

	const double liquid_interface_x = x_min + height * fp.liquid_fill_ratio;
	const double wave_amp = width * fp.wave_amp_ratio;

	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;
		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = meshInfo.triangleStructure.GetAllNodes();
		nodecoords = meshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&meshInfo.triangleStructure);

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double y = nodecoords->Value(nodeID, 2);

			// 壁厚区
			bool isWall = IsInWallRegion(x, y, boundaryEdges, WALL_THICKNESS);
			bool isRightForce = (x > x_max - 15.0);
			bool isBottomWall = (y < y_min + 15.0) || (y > y_max - 15.0);

			if (isWall || isRightForce || isBottomWall) {
				nodeValues.push_back(WALL_VALUE);
				continue;
			}

			double value = min_value;
			double dy = fabs(y - y_center);
			double dx = x - x_min;

			// ========== 射流核心 ==========
			double jet_value = min_value;
			if (dx >= 0 && dx <= jet_length + jet_transition * 2.0) {
				double taper = 1.0 - (dx / std::max(jet_length, 1e-6)) * 0.3;
				taper = std::max(taper, 0.4);

				double local_core = jet_core_width * taper;
				double local_sigma = jet_transition * taper * 0.5;

				double r_eff = std::max(0.0, dy - local_core);
				double profile = exp(-pow(r_eff / local_sigma, 2));

				if (dx <= jet_length) {
					jet_value = max_value * profile;
				}
				else {
					double tail = 0.5 * (1.0 + cos(std::min((dx - jet_length) / (jet_transition * 2.0) * 3.14159265358979323846, 3.14159265358979323846)));
					jet_value = max_value * profile * tail;
				}
			}

			// ========== 液相区（主体全红，界面过渡） ==========
			double liquid_value = min_value;
			if (fp.has_liquid_fill) {
				double hump_left = wave_amp * exp(-pow((y - (y_center - width * 0.22)) / (width * 0.08), 2));
				double hump_right = wave_amp * exp(-pow((y - (y_center + width * 0.22)) / (width * 0.08), 2));
				double crater = wave_amp * 1.5 * exp(-pow((y - y_center) / (width * 0.10), 2));

				double local_interface = liquid_interface_x - hump_left - hump_right + crater;
				double interface_thick = height * 0.015;

				if (x >= local_interface + interface_thick) {
					// 【修正】液相主体直接给 max_value（全红），不乘 liquid_core_ratio
					liquid_value = max_value;
				}
				else if (x >= local_interface - interface_thick) {
					// 界面过渡区，从 0 渐变到 max_value
					double frac = (x - (local_interface - interface_thick)) / (2.0 * interface_thick);
					liquid_value = max_value * frac;
				}
			}

			// 综合
			value = std::max(jet_value, liquid_value);

			// ========== 壁面边界层：红→灰渐变，蓝→灰硬截断 ==========
			double min_wall_dist = 1e9;
			for (auto& e : boundaryEdges) {
				if (e.isTopEdge || e.isRightEdge || e.isBottomEdge) {
					double d = PointToSegmentDistance(x, y, e.p1, e.p2);
					if (d < min_wall_dist) min_wall_dist = d;
				}
			}

			double layer_thick = width * 0.04;
			double red_threshold = max_value * 0.5; // 5.0

			if (min_wall_dist < layer_thick && value > red_threshold) {
				// 红色区域：向壁面渐变到灰色
				double f = min_wall_dist / layer_thick;
				f = f * f;

				// 壁面处 = 低值（灰色边界），内部保持红色
				double wall_target = min_value + (max_value - min_value) * 0.15; // 1.5
				value = wall_target + (value - wall_target) * f;
			}
			// else：蓝色区域不做渐变，硬截断

			value = std::clamp(value, min_value, max_value);
			nodeValues.push_back(value);
		}

		if (!meshInfo.preForwardDisplayCreated)
		{
			//context->EraseAll(true);
			meshInfo.preForwardMesh = new MeshVS_Mesh();
			meshInfo.preForwardMesh->SetDataSource(&meshInfo.triangleStructure);
			meshInfo.preForwardNodalBuilder = new MeshVS_NodalColorPrsBuilder(
				meshInfo.preForwardMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
			meshInfo.preForwardMesh->AddBuilder(meshInfo.preForwardNodalBuilder);
			meshInfo.preForwardMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
			context->Display(meshInfo.preForwardMesh, Standard_True);
			meshInfo.preForwardDisplayCreated = true;
		}
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		meshInfo.preForwardNodalBuilder->SetColors(colormap);
		context->Redisplay(meshInfo.preForwardMesh, Standard_True);
		//occView->fitAll();
		meshInfo.preForwardViewInitialized = true;
	}

	return true;
}

bool APISetNodeValue::SetPreForwardDesignResult(OccView* occView, std::vector<double>& nodeValues, int frame)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	auto modelGeometryInfo = ModelDataManager::GetInstance()->GetModelGeometryInfo();
	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	const double WALL_THICKNESS = 10.0;
	const double WALL_VALUE = -1.0;
	const double max_value = 100;
	const double min_value = 0.0;

	auto& boundaryEdges = meshInfo.preForwardBoundaryEdges;
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double y_min = meshInfo.y_min;
	const double y_max = meshInfo.y_max;


	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;
		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = meshInfo.triangleStructure.GetAllNodes();
		nodecoords = meshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&meshInfo.triangleStructure);

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double y = nodecoords->Value(nodeID, 2);

			// 1. 先处理强制边界（右侧、底部），直接给 WALL_VALUE，不参与渐变
			bool isRightForce = (x > x_max - 15.0);
			bool isBottomWall = (y < y_min + 15.0) || (y > y_max - 15.0);

			// 2. 判断是否在壁厚区
			bool isWall = IsInWallRegion(x, y, boundaryEdges, WALL_THICKNESS);

			if (isWall || isRightForce || isBottomWall)
			{
				// 1. 计算到最近边界距离
				double min_wall_dist = 1e9;
				for (auto& e : boundaryEdges)
				{
					if (e.isTopEdge || e.isRightEdge || e.isBottomEdge)
					{
						double d = PointToSegmentDistance(x, y, e.p1, e.p2);
						if (d < min_wall_dist) min_wall_dist = d;
					}
				}

				// 2. 分层参数
				const int TOTAL_LAYERS = 12;
				double layerThick = WALL_THICKNESS / TOTAL_LAYERS;  // ~0.833

				// 当前节点在第几层（0~11，0最靠近壁面）
				int layer = static_cast<int>(min_wall_dist / layerThick);
				if (layer < 0) layer = 0;
				if (layer >= TOTAL_LAYERS) layer = TOTAL_LAYERS - 1;

				// 3. 按帧分配
				// frame=0: 第0层=100, 第1~10层=渐变, 第11层=0
				// frame=1: 第0~1层=100, 第2~10层=渐变, 第11层=0
				// ...
				// frame=10: 第0~10层=100, 第11层=0
				// frame=11: 第0~11层=100（全满）

				double value;

				if (layer <= frame)
				{
					// 红色区：已渗透到的层
					value = max_value;  // 100
				}
				else if (layer == TOTAL_LAYERS - 1)
				{
					// 最内层固定为 0
					value = min_value;  // 0
				}
				else
				{
					// 渐变区：从 100 线性降到 0
					// 渐变区范围：frame+1 ~ TOTAL_LAYERS-2
					int gradStart = frame + 1;           // 渐变起始层
					int gradEnd = TOTAL_LAYERS - 2;      // 渐变结束层
					int gradCount = gradEnd - gradStart + 1;

					if (gradCount <= 0 || layer < gradStart || layer > gradEnd)
					{
						value = min_value;
					}
					else
					{
						// 在渐变区内线性插值：gradStart=100, gradEnd=0
						double frac = static_cast<double>(gradEnd - layer) / gradCount;
						value = max_value * frac;
					}
				}

				nodeValues.push_back(value);
			}
			else
			{
				nodeValues.push_back(WALL_VALUE);
			}
		}

		// ========== 可视化部分 ==========
		if (!meshInfo.preForwardDisplayCreated)
		{
			meshInfo.preForwardMesh = new MeshVS_Mesh();
			meshInfo.preForwardMesh->SetDataSource(&meshInfo.triangleStructure);
			meshInfo.preForwardNodalBuilder = new MeshVS_NodalColorPrsBuilder(
				meshInfo.preForwardMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
			meshInfo.preForwardMesh->AddBuilder(meshInfo.preForwardNodalBuilder);
			meshInfo.preForwardMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
			context->Display(meshInfo.preForwardMesh, Standard_True);
			meshInfo.preForwardDisplayCreated = true;
		}
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		meshInfo.preForwardNodalBuilder->SetColors(colormap);
		context->Redisplay(meshInfo.preForwardMesh, Standard_True);
		meshInfo.preForwardViewInitialized = true;
	}

	return true;
}

