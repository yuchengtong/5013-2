#include "APISetNodeValue.h"
#include "ModelDataManager.h"
#include <algorithm>
#include <cmath>
#include <MeshVS_Mesh.hxx>
#include <MeshVS_DataMapOfIntegerColor.hxx>
#include <MeshVS_NodalColorPrsBuilder.hxx>
#include <MeshVS_DrawerAttribute.hxx>
#include <MeshVS_Drawer.hxx>


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

	// 处理特殊情况：避免除零
	if (max <= min) {
		Quantity_Color defaultColor(0.5, 0.5, 0.5, Quantity_TOC_RGB); // 灰色
		for (size_t i = 0; i < tt.size(); ++i) {
			colormap.Bind(i + 1, defaultColor);
		}
		return colormap;
	}

	for (double t : tt)
	{
		// 1. 归一化到[0,1]范围
		a = (t - min) / (max - min);
		a = std::clamp(a, 0.0, 1.0); // 确保值在有效范围

		// 2. 定义HSV参数（保持蓝→绿→红趋势）
		// H: 0.666(蓝) → 0.333(绿) → 0(红)，对应HSV色轮
		double h = 0.666 - a * 0.666; // 从蓝色(0.666)过渡到红色(0)
		double s = 1.0;               // 最大饱和度
		double v = 1.0;               // 最大明度

		// 3. 手动转换HSV到RGB
		HSVtoRGB(h, s, v, r, g, b);

		// 4. 绑定颜色到索引
		if (t == -1.0) { // 如果检测到特殊值，则设置为灰色
			colormap.Bind(index + 1, Quantity_Color(0.5, 0.5, 0.5, Quantity_TOC_RGB));
		}
		else {
			colormap.Bind(index + 1, Quantity_Color(r, g, b, Quantity_TOC_RGB));
		}
		index++;
	}
	return colormap;
}

bool APISetNodeValue::SetPreForwardDesignResult0(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.y_min + meshInfo.y_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.y_min };
	Point p2{ meshInfo.x_max, meshInfo.y_min };
	Point p3{ meshInfo.x_max, meshInfo.y_max };
	Point p4{ meshInfo.x_min, meshInfo.y_max };

	// 边界参数
	const double x_min = meshInfo.x_min;  // 左侧（注入入口）
	const double x_max = meshInfo.x_max;  // 右侧
	const double y_min = meshInfo.y_min;  // 底部
	const double y_max = meshInfo.y_max;  // 顶部

	// 几何参数
	const double center_y = (y_min + y_max) / 2.0;  // 垂直中心（Y方向）
	const double x_inlet = x_min;                    // 注入入口在左侧（X最小值）

	// 物理参数
	const double max_value = 6.67;   // 根据你的色条最大值调整
	const double min_value = 0.0;
	const double inlet_r = 5.0;      // 注入口半径（Y方向）
	const double decay_x = 150.0;    // 水平衰减长度（X方向，加大让渲染更长）
	const double decay_y = 15.0;     // 垂直扩散长度（Y方向）

	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;
		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure);

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);  // X：水平方向（左右）
			double y = nodecoords->Value(nodeID, 2);  // Y：垂直方向（上下）

			// 到垂直中心线（Y方向）的距离
			double r = fabs(y - center_y);

			// 到注入面的水平距离（从左侧向右扩散）
			double dx = x - x_inlet;  // x 越大，dx 越大，衰减越多

			double value = min_value;

			if (dx >= 0) {  // 只在入口右侧区域计算
				// 水平衰减：从入口向右指数衰减
				double v_axial = exp(-dx / decay_x);
				// 垂直扩散：高斯型
				double v_radial = exp(-(r * r) / (decay_y * decay_y));
				value = max_value * v_axial * v_radial;
			}

			// 钳制
			if (value > max_value) value = max_value;
			if (value < min_value) value = min_value;

			nodeValues.push_back(value);
		}
		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;

}

bool APISetNodeValue::SetPreForwardDesignResult1(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
		(meshInfo.y_min + meshInfo.y_max) / 2.0 };
	Point p1{ meshInfo.x_min, meshInfo.y_min };
	Point p2{ meshInfo.x_max, meshInfo.y_min };
	Point p3{ meshInfo.x_max, meshInfo.y_max };
	Point p4{ meshInfo.x_min, meshInfo.y_max };

	// 边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double y_min = meshInfo.y_min;
	const double y_max = meshInfo.y_max;

	// 几何参数
	const double center_y = (y_min + y_max) / 2.0;
	const double width = x_max - x_min;
	const double height = y_max - y_min;

	// 物理参数
	const double max_value = 10.0;
	const double min_value = 0.0;

	// 射流参数
	const double jet_half_height = height * 0.03;      // 射流半高
	const double jet_core_width = height * 0.015;      // 红色核心区半高

	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;
		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure);

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double y = nodecoords->Value(nodeID, 2);

			double value = min_value;
			double r = fabs(y - center_y);

			// ========== 射流核心区（全红，贯穿整个宽度） ==========
			if (r < jet_core_width) {
				// 核心区：全红，从左到右贯穿
				value = max_value;
			}
			// ========== 射流过渡区（核心区外侧，高斯衰减） ==========
			else if (r < jet_half_height * 4) {
				// 过渡区：高斯衰减
				value = max_value * exp(-pow((r - jet_core_width) / (jet_half_height * 0.6), 2));
			}
			// ========== 外部气相区 ==========
			else {
				value = min_value;
			}

			// ========== 右侧壁面边界层（只在射流核心区外生效） ==========
			double dist_to_right = x_max - x;
			double right_boundary = width * 0.02;

			if (dist_to_right < right_boundary) {
				// 右侧边界层：所有值衰减到0
				double factor = dist_to_right / right_boundary;
				value *= factor;
			}

			// 上下壁面边界层
			double dist_to_top = y - y_min;
			double dist_to_bottom = y_max - y;
			double wall_dist_y = std::min(dist_to_top, dist_to_bottom);
			if (wall_dist_y < height * 0.03 && value > min_value) {
				double factor = wall_dist_y / (height * 0.03);
				value = min_value + (value - min_value) * factor;
			}

			// 钳制
			if (value > max_value) value = max_value;
			if (value < min_value) value = min_value;

			nodeValues.push_back(value);
		}



		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}

bool APISetNodeValue::SetPreForwardDesignResult2(OccView* occView, std::vector<double>& nodeValues)
{
	Handle(AIS_InteractiveContext) context = occView->getContext();
	Handle(V3d_View) view = occView->getView();

	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();

	// 边界参数
	const double x_min = meshInfo.x_min;
	const double x_max = meshInfo.x_max;
	const double y_min = meshInfo.y_min;
	const double y_max = meshInfo.y_max;

	// 几何参数
	const double center_y = (y_min + y_max) / 2.0;
	const double width = x_max - x_min;
	const double height = y_max - y_min;

	// 物理参数
	const double max_value = 10.0;
	const double min_value = 0.0;

	// 射流核心参数（减小！）
	const double jet_half_height = height * 0.04;      // 射流半高（减小）
	const double jet_core_width = height * 0.02;       // 红色核心区半高（减小）

	// 界面参数
	const double interface_base = x_min + width * 0.38;
	const double wave_amp = height * 0.10;
	const double wave_center = center_y;

	{
		TColStd_PackedMapOfInteger allnode;
		Handle(TColStd_HArray2OfReal) nodecoords;
		Handle(MeshVS_Mesh) aMesh = nullptr;

		allnode = modelMeshInfo.triangleStructure.GetAllNodes();
		nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();

		aMesh = new MeshVS_Mesh();
		aMesh->SetDataSource(&modelMeshInfo.triangleStructure);

		for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next())
		{
			int nodeID = it.Key();
			double x = nodecoords->Value(nodeID, 1);
			double y = nodecoords->Value(nodeID, 2);

			double value = min_value;
			double r = fabs(y - center_y);
			double dx = x - x_min;

			// ========== 涡旋界面位置 ==========
			double wave_shape2 =
				exp(-pow((y - (wave_center - height * 0.22)) / (height * 0.10), 2)) +
				exp(-pow((y - (wave_center + height * 0.22)) / (height * 0.10), 2));

			double local_interface = interface_base - wave_amp * wave_shape2;
			double interface_thickness = width * 0.025;  // 加宽过渡区

			// ========== 射流核心（细化） ==========
			double jet_length = local_interface - x_min;
			double v_jet = min_value;

			if (dx < jet_length) {
				if (r < jet_core_width) {
					v_jet = max_value;  // 核心区全红（窄）
				}
				else if (r < jet_half_height * 3) {
					// 过渡区：高斯衰减（黄→绿→青→蓝）
					v_jet = max_value * exp(-pow((r - jet_core_width) / (jet_half_height * 0.8), 2));
				}
				else {
					v_jet = min_value;
				}
			}

			// ========== 液相区（右侧主体） ==========
			double v_liquid = min_value;
			if (x > local_interface + interface_thickness) {
				v_liquid = max_value;

				// 壁面边界层（右侧）
				double dist_to_right = x_max - x;
				if (dist_to_right < width * 0.04) {
					v_liquid = min_value + (max_value - min_value) * (dist_to_right / (width * 0.04));
				}
			}

			// ========== 界面过渡区（关键！恢复渐变） ==========
			double v_interface = min_value;
			if (x >= local_interface - interface_thickness && x <= local_interface + interface_thickness) {
				// 在界面厚度内
				double frac = (x - (local_interface - interface_thickness)) / (2 * interface_thickness);

				// 界面左侧（气相侧）：蓝→绿→黄→红
				if (frac < 0.5) {
					v_interface = max_value * frac * 2;  // 0 -> 0.5*max -> max
				}
				else {
					v_interface = max_value;  // 液相侧全红
				}
			}

			// ========== 综合：取三个区域的最大值 ==========
			value = std::max({ v_jet, v_liquid, v_interface });

			// 上下壁面边界层
			double dist_to_top = y - y_min;
			double dist_to_bottom = y_max - y;
			double wall_dist_y = std::min(dist_to_top, dist_to_bottom);
			if (wall_dist_y < height * 0.03 && value > min_value) {
				double factor = wall_dist_y / (height * 0.03);
				value = min_value + (value - min_value) * factor;
			}

			// 钳制
			if (value > max_value) value = max_value;
			if (value < min_value) value = min_value;

			nodeValues.push_back(value);
		}




		// 设置颜色映射和显示（与原逻辑一致）
		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
		nodal->SetColors(colormap);
		aMesh->AddBuilder(nodal);
		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
		context->EraseAll(true);
		context->Display(aMesh, Standard_True);
		occView->fitAll();
	}

	return true;
}


//bool APISetNodeValue::SetFallStressResult(OccView* occView, std::vector<double>& nodeValues)
//{
//	Handle(AIS_InteractiveContext) context = occView->getContext();
//	Handle(V3d_View) view = occView->getView();
//	
//
//	auto modelMeshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
//
//	auto fallAnalysisResultInfo = ModelDataManager::GetInstance()->GetFallAnalysisResultInfo();
//	auto fallSettingInfo = ModelDataManager::GetInstance()->GetFallSettingInfo();
//	auto steelPropertyInfoInfo = ModelDataManager::GetInstance()->GetSteelPropertyInfo();
//
//	auto high = fallSettingInfo.high;
//	auto angle = fallSettingInfo.angle;
//	auto youngModulus = 0;
//
//	auto meshInfo = ModelDataManager::GetInstance()->GetModelMeshInfo();
//	Point p0{ (meshInfo.x_min + meshInfo.x_max) / 2.0,
//		(meshInfo.z_min + meshInfo.z_max) / 2.0 };
//	Point p1{ meshInfo.x_min, meshInfo.z_min };
//	Point p2{ meshInfo.x_max, meshInfo.z_min };
//	Point p3{ meshInfo.x_max, meshInfo.z_max };
//	Point p4{ meshInfo.x_min, meshInfo.z_max };
//
//	// 从角点计算矩形边界参数
//	const double x_min = meshInfo.x_min;
//	const double x_max = meshInfo.x_max;
//	const double z_min = meshInfo.z_min;
//	const double z_max = meshInfo.z_max;
//
//
//	if (fallAnalysisResultInfo.isChecked)
//	{
//		TColStd_PackedMapOfInteger allnode;
//		Handle(TColStd_HArray2OfReal) nodecoords;
//
//		auto max_value = fallAnalysisResultInfo.stressMaxValue;
//		auto min_value = fallAnalysisResultInfo.stressMinValue;
//
//		//std::vector<double> nodeValues;
//		Handle(MeshVS_Mesh) aMesh = nullptr;
//		if (angle == 0)
//		{
//			allnode = modelMeshInfo.triangleStructure.GetAllNodes();
//			nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();
//
//			aMesh = new MeshVS_Mesh();
//			aMesh->SetDataSource(&modelMeshInfo.triangleStructure);
//
//
//			// --- 2. 根据矩形角点计算椭圆参数 ---
//			const double ellipse_h = (x_min + x_max) / 2.0;       
//			const double ellipse_k = z_min + 20;   
//			const double rect_length = x_max - x_min;             
//			const double rect_width = z_max - z_min;              
//			const double ellipse_a = rect_length * 0.8 / 2.0;     
//			const double ellipse_b = rect_width * 0.4 / 2.0;      
//
//			double red_line_z = z_min+10;
//			for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) 
//			{
//				int nodeID = it.Key();
//				double x = nodecoords->Value(nodeID, 1); // 节点x坐标
//				double z = nodecoords->Value(nodeID, 3); // 节点z坐标
//				// --- 数学判断逻辑 ---
//				// 计算椭圆方程左边的值
//				// ((x - h)^2) / (a^2) + ((z - k)^2) / (b^2)
//				double dx = x - ellipse_h;
//				double dz = z - ellipse_k;
//
//				// 为了提高精度和效率，可以比较平方和，避免开方和除法
//				// (dx*dx) * (b*b) + (dz*dz) * (a*a) <= (a*a) * (b*b)
//				double value = (dx * dx) * (ellipse_b * ellipse_b) + (dz * dz) * (ellipse_a * ellipse_a);
//				double threshold = (ellipse_a * ellipse_a) * (ellipse_b * ellipse_b);
//
//				// 考虑浮点计算误差，使用一个小的容差
//				if (z < red_line_z)
//				{
//					nodeValues.push_back(max_value);
//				}
//				else if (z > red_line_z&&z< red_line_z+5)
//				{
//					nodeValues.push_back(min_value+(max_value- min_value)*7.5/9.0);//橙色
//				}
//				else if (z > red_line_z+5 && z < red_line_z + 10)
//				{
//					nodeValues.push_back(min_value + (max_value - min_value) * 6.5 / 9.0);//黄色
//				}
//				else if (z > red_line_z+10 && z < red_line_z + 15)
//				{
//					nodeValues.push_back(min_value + (max_value - min_value) * 5.5 / 9.0);//绿色
//				}
//				else if (z > red_line_z+15 && z < red_line_z + 20)
//				{
//					nodeValues.push_back(min_value + (max_value - min_value) * 3.5 / 9.0);//浅绿色
//				}
//				else
//				{
//					if (value <= threshold + Precision::Confusion())
//					{
//						nodeValues.push_back(min_value + (max_value - min_value) * 2.5 / 9.0);//浅蓝
//					}
//					else
//					{
//						nodeValues.push_back(min_value);
//					}
//				}
//			}
//		}
//		else if (angle == 45)
//		{
//			//点的坐标用0，渲染用45
//			allnode = modelMeshInfo.triangleStructure.GetAllNodes();
//			nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();
//
//			aMesh = new MeshVS_Mesh();
//			aMesh->SetDataSource(&modelMeshInfo.triangleStructure45);
//
//			// --- 2. 根据矩形角点计算椭圆参数 ---
//			const double ellipse_h = (x_min + x_max) / 2.0;
//			const double ellipse_k = (z_min + z_max) / 2.0;
//			const double rect_length = x_max - x_min;
//			const double rect_width = z_max - z_min;
//			const double ellipse_a = rect_length  / 2.0;
//			const double ellipse_b = rect_width  / 2.0;
//
//			for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) {
//				int nodeID = it.Key();
//				double x = nodecoords->Value(nodeID, 1); // 节点x坐标
//				double z = nodecoords->Value(nodeID, 3); // 节点z坐标
//
//				// --- 数学判断逻辑 ---
//				// 计算椭圆方程左边的值
//				// ((x - h)^2) / (a^2) + ((z - k)^2) / (b^2)
//				double dx = x - ellipse_h;
//				double dz = z - ellipse_k;
//
//				// 为了提高精度和效率，可以比较平方和，避免开方和除法
//				// (dx*dx) * (b*b) + (dz*dz) * (a*a) <= (a*a) * (b*b)
//				double value = (dx * dx) * (ellipse_b * ellipse_b) + (dz * dz) * (ellipse_a * ellipse_a);
//				double threshold = (ellipse_a * ellipse_a) * (ellipse_b * ellipse_b);
//
//				// 考虑浮点计算误差，使用一个小的容差
//				if (value <= threshold + Precision::Confusion())
//				{
//					nodeValues.push_back(min_value);
//				}
//				else
//				{
//					if (x > ellipse_h && z < ellipse_k)
//					{
//						if (z >= z_min && z<z_min + 20 && x>x_max - 20 && x <= x_max)
//						{
//							nodeValues.push_back(max_value);
//						}
//						else if (z > z_min + 20 && z < z_min + 30
//							&& x < x_max-20 && x > x_max-30)
//						{
//							nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
//						}
//						else if (z > z_min + 30 && z < z_min + 40
//							&& x < x_max - 30 && x > x_max - 40)
//						{
//							nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
//						}
//						else if (z > z_min + 40 && z < z_min + 50
//							&& x < x_max - 40 && x > x_max - 50)
//						{
//							nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
//						}
//						else
//						{
//							nodeValues.push_back(min_value + (max_value - min_value) * 0.3);
//						}
//					}
//					else
//					{
//						nodeValues.push_back(min_value);
//					}
//				}
//
//
//
//			}
//		}
//		else if (angle == 90)
//		{
//			//点的坐标用0，渲染用90
//			allnode = modelMeshInfo.triangleStructure.GetAllNodes();
//			nodecoords = modelMeshInfo.triangleStructure.GetmyNodeCoords();
//
//			aMesh = new MeshVS_Mesh();
//			aMesh->SetDataSource(&modelMeshInfo.triangleStructure90);
//
//			// --- 2. 根据矩形角点计算椭圆参数 ---
//			const double ellipse_h = (x_min + x_max) / 2.0;
//			const double ellipse_k = (z_min + z_max) / 2.0;
//			const double rect_length = x_max - x_min;
//			const double rect_width = z_max - z_min;
//			const double ellipse_a = rect_length / 2.0;
//			const double ellipse_b = rect_width / 2.0;
//
//			//double red_line_z = z_min + 200;
//
//			for (TColStd_PackedMapOfInteger::Iterator it(allnode); it.More(); it.Next()) {
//				int nodeID = it.Key();
//				double x = nodecoords->Value(nodeID, 1); // 节点x坐标
//				double z = nodecoords->Value(nodeID, 3); // 节点z坐标
//
//				// --- 数学判断逻辑 ---
//				// 计算椭圆方程左边的值
//				// ((x - h)^2) / (a^2) + ((z - k)^2) / (b^2)
//				double dx = x - ellipse_h;
//				double dz = z - ellipse_k;
//
//				// 为了提高精度和效率，可以比较平方和，避免开方和除法
//				// (dx*dx) * (b*b) + (dz*dz) * (a*a) <= (a*a) * (b*b)
//				double value = (dx * dx) * (ellipse_b * ellipse_b) + (dz * dz) * (ellipse_a * ellipse_a);
//				double threshold = (ellipse_a * ellipse_a) * (ellipse_b * ellipse_b);
//
//				if (value <= threshold + Precision::Confusion())
//				{
//					nodeValues.push_back(min_value);
//				}
//				else
//				{
//					if (x > (x_min + x_max) / 2.0)
//					{
//						if (x< x_max && x>x_max - 10)
//						{
//							nodeValues.push_back(max_value);
//						}
//						else if (x< x_max - 10 && x>x_max - 20)
//						{
//							nodeValues.push_back(min_value + (max_value - min_value) * 0.8);
//						}
//						else if (x< x_max - 20 && x>x_max - 30)
//						{
//							nodeValues.push_back(min_value + (max_value - min_value) * 0.6);
//						}
//						else if (x< x_max - 30 && x>x_max - 40)
//						{
//							nodeValues.push_back(min_value + (max_value - min_value) * 0.4);
//						}
//						else
//						{
//							nodeValues.push_back(min_value + (max_value - min_value) * 0.2);
//						}
//					}
//					else
//					{
//						nodeValues.push_back(min_value);
//					}
//				}
//			}
//		}
//
//		// 设置颜色映射和显示（与原逻辑一致）
//		MeshVS_DataMapOfIntegerColor colormap = GetMeshDataMap(nodeValues, min_value, max_value);
//		Handle(MeshVS_NodalColorPrsBuilder) nodal = new MeshVS_NodalColorPrsBuilder(aMesh, MeshVS_DMF_NodalColorDataPrs | MeshVS_DMF_OCCMask);
//		nodal->SetColors(colormap);
//		aMesh->AddBuilder(nodal);
//		aMesh->GetDrawer()->SetBoolean(MeshVS_DA_ShowEdges, false);
//		context->EraseAll(true);
//		context->Display(aMesh, Standard_True);
//		occView->fitAll();
//	}
//
//	return true;
//}
