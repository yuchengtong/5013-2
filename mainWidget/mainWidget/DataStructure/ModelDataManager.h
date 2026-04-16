#pragma once
#include <Standard_Type.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>

#include <QObject>
#include <string>
#include <QString>
#include <QRandomGenerator>

#include "TriangleStructure.h"


struct UserInfo {
	QString username = "admin";
	QString password = "";
};

struct ModelGeometryInfo {
	TopoDS_Shape shape;
	QString path="";


	double theXmin = 0.0;
	double theYmin = 0.0;
	double theZmin = 0.0;
	double theXmax = 0.0;
	double theYmax = 0.0;
	double theZmax = 0.0;


	double length = 0.0; 
	double width = 0.0;
	double height = 0.0;
	double thickness = 0.0;

	double boreDiameter = 391.0; // 弹体大径
	double equaldiameterSection = 560.0; // 等径段高度
	double boreRadius = 0.0; // 弹体小径
	double variableDiameterSectionHeight = 0.0; // 变径段高度
	double shellThickness = 20.0; // 壳体厚度
	double gasketLayerThickness = 1.0; // 胶层厚度
	double injectionHoleDiameter = 20.0; // 注药孔孔径
	double vacuumHoleDiameter = 8.0; // 真空孔孔径

	
};

struct ModelMeshInfo {
	TriangleStructure triangleStructure;
	TriangleStructure triangleStructure45;
	TriangleStructure triangleStructure90;
	bool isChecked=false;

	double x_min = 0.0;
	double x_max = 0.0;
	double z_min = 0.0;
	double z_max = 0.0;

};

// 壳体材料
struct SteelPropertyInfo {
	QString name = ""; // 材料名称
	QString type = ""; // 材料类型
	double density = 0.0; // 密度
	double thermalConductivity = 0.0;// 导热系数
	double specificHeatCapacity = 0.0;// 比热容
	bool isChecked = false;
};
// 药液材料
struct PropellantPropertyInfo {
	QString name = ""; // 材料名称
	QString type = ""; // 材料类型
	double density = 0.0; // 密度
	double thermalConductivity = 0.0;// 导热系数
	double specificHeatCapacity = 0.0;// 比热容
	bool isChecked = false;
};

// 明胶材料
struct GelatinPropertyInfo {
	QString name = ""; // 材料名称
	QString type = ""; // 材料类型
	double density = 0.0; // 密度
	double thermalConductivity = 0.0;// 导热系数
	double specificHeatCapacity = 0.0;// 比热容
	bool isChecked = false;
};

// 预热正向设计
struct PreForwardPropertyInfo {
	double targetTemperatureValue = 50.0; // 弹体目标温度
	double environmentalTemperatureValue = 90.0; // 烘箱环境温度
	double initialTemperatureValue = 22.0; // 弹体初始温度
	double heatTransferCoefficientValue = 58.0; // 环境对流传热系数
	double absorptionCoefficientValue = 0.5; // 壳体辐射吸收系数
	double environmentalEmissivityValue = 0.5; // 环境发射率
	double preheatingTimeValue = 0.0; // 弹体预热时间
	bool isChecked = false;
};

// 预热逆向寻优
struct PreReversePropertyInfo {
	double targetTemperatureValue = 50.0; // 弹体目标温度
	double environmentalTemperatureValue = 90.0; // 烘箱环境温度
	double initialTemperatureValue = 22.0; // 弹体初始温度
	double heatTransferCoefficientValue = 58.0; // 环境对流传热系数
	double absorptionCoefficientValue = 0.5; // 壳体辐射吸收系数
	double environmentalEmissivityValue = 0.5; // 环境发射率
	double preheatingTimeValue = 0.0; // 弹体预热时间
	bool isChecked = false;
};


// 注药正向设计
struct InForwardPropertyInfo {
	double m_insulationTemperatureValue = 50.0; // 弹体保温温度
	double m_pouringTemperatureValue = 102.0; // 药液浇注温度
	double m_pouringSpeedValue = 15.0; // 药液浇注速度
	double m_vacuumDegreeValue = 0.1; // 真空度
	double m_relativeDensityValue = 0.0; // 相对密度
	double m_injectionTimeValue = 0.0; // 弹体注药时间
	bool isChecked = false;
};

// 注药逆向寻优
struct InReversePropertyInfo {
	double m_insulationTemperatureValue = 50.0; // 弹体保温温度
	double m_pouringTemperatureValue = 102.0; // 药液浇注温度
	double m_pouringSpeedValue = 15.0; // 药液浇注速度
	double m_vacuumDegreeValue = 0.1; // 真空度
	double m_relativeDensityValue = 0.0; // 相对密度
	double m_injectionTimeValue = 0.0; // 弹体注药时间
	bool isChecked = false;
};



// 评判标准类型
struct JudgementPropertyInfo {
	QString name = ""; // 评判标准类型
	bool isChecked = false;
};

// 计算模型数据库
struct CalculationPropertyInfo {

	// 预热工艺工程计算模型
	QString preForwardCalculateFormula = "340.36210967-1067.26983815*A+224.43952784*B-232.50668176*C+531.22537911*D+301.92996804*E-272.00468157*F+7246.20722434*A^2+298.76335298*A*B-916.76663788*A*C+1973.63586011*A*D+1965.32179943*A*E-552.33403266*A*F-267.12344202*B^2-83.25807155*B*C+118.32458792*B*D+196.94548638*B*E-23.78257687*B*F+598.49323067*C^2+149.35934868*C*D+34.71164314*C*E-252.25568167*C*F-497.47957785*D^2-49.07147517*D*E+129.61034877*D*F+327.83915059*E^2-336.65496753*E*F+808.76372735*F^2-12467.09212988*A^3-1506.22894326*A^2*B+2398.90470582*A^2*C-4050.30706421*A^2*D-4272.9119075*A^2*E+2198.12371809*A^2*F+350.96884875*A*B^2-99.08713132*A*B*C-46.68990051*A*B*D+260.68170225*A*B*E+140.0321197*A*B*F+364.59776476*A*C^2+795.25207065*A*C*D+791.09504031*A*C*E-467.73287573*A*C*F-730.80266957*A*D^2+906.01655557*A*D*E+977.46837326*A*D*F-253.26862744*A*E^2-811.18771601*A*E*F+435.14463685*A*F^2+152.04262591*B^3+4.30446398*B^2*C+235.99703487*B^2*D+197.63083881*B^2*E-201.5628914*B^2*F+113.81757717*B*C^2-38.57662377*B*C*D+0.73382546*B*C*E-109.63020617*B*C*F+109.61540992*B*D^2-131.29941918*B*D*E-8.83887643*B*D*F-67.95668944*B*E^2-145.33723259*B*E*F+460.49174563*B*F^2-1063.19187773*C^3+432.05296011*C^2*D+374.72910735*C^2*E+231.24544494*C^2*F-306.86645936*C*D^2-82.66240803*C*D*E+6.67850394*C*D*F+105.79290485*C*E^2-226.45415421*C*E*F+346.25519323*C*F^2+409.77875293*D^3+815.76930212*D^2*E-316.74095932*D^2*F+913.92252725*D*E^2-268.34571336*D*E*F+537.18820845*D*F^2-825.19274616*E^3-87.01156629*E^2*F+925.77834354*E*F^2-1654.97776971*F^3+5778.55726496*A^4+1583.65725439*A^3*B+16.98361215*A^3*C+5191.14300679*A^3*D+3990.74702*A^3*E-489.14965595*A^3*F-227.08094084*A^2*B^2-92.00773478*A^2*B*C+1540.89627581*A^2*B*D+1175.16177334*A^2*B*E-1465.60441867*A^2*B*F+173.21292235*A^2*C^2-2637.25298528*A^2*C*D-2748.55540693*A^2*C*E+486.96240587*A^2*C*F+377.95584225*A^2*D^2+3586.50993492*A^2*D*E-2737.64347914*A^2*D*F+276.69822499*A^2*E^2+238.59093033*A^2*E*F+92.3337951*A^2*F^2-68.73198138*A*B^3-0.15285679*A*B^2*C-68.1918589*A*B^2*D-132.72855045*A*B^2*E+170.80964598*A*B^2*F+177.85395147*A*B*C^2-147.57935416*A*B*C*D+6.10644722*A*B*C*E-54.21834406*A*B*C*F+799.04824506*A*B*D^2-632.83480424*A*B*D*E-28.01972865*A*B*D*F+789.28506093*A*B*E^2-766.58345674*A*B*E*F+425.67188644*A*B*F^2-841.43343567*A*C^3+809.11657718*A*C^2*D+807.03806201*A*C^2*E+177.62410398*A*C^2*F-461.18426449*A*C*D^2+357.22534809*A*C*D*E+392.95125693*A*C*D*F-222.41724342*A*C*E^2-501.3767877*A*C*E*F+121.78938872*A*C*F^2+246.48214214*A*D^3+164.791241*A*D^2*E-370.07611318*A*D^2*F-216.3706959*A*D*E^2-443.91603008*A*D*E*F+844.39001322*A*D*F^2-214.14506006*A*E^3+107.05927044*A*E^2*F+1043.54097404*A*E*F^2-1004.69159601*A*F^3+5.19197962*B^4-30.61731571*B^3*C-180.16141412*B^3*D-163.56709028*B^3*E+42.02072776*B^3*F-115.74992669*B^2*C^2+186.93160993*B^2*C*D+167.7485119*B^2*C*E-31.84835321*B^2*C*F-153.1351102*B^2*D^2+167.18251016*B^2*D*E+83.99793224*B^2*D*F-32.44276562*B^2*E^2-23.08956848*B^2*E*F+135.41007133*B^2*F^2-63.89131034*B*C^3+58.43499459*B*C^2*D+78.0902192*B*C^2*E+22.90820339*B*C^2*F+5.93824609*B*C*D^2-114.51916846*B*C*D*E-53.28889708*B*C*D*F-82.84780359*B*C*E^2-121.53807516*B*C*E*F+181.37641395*B*C*F^2-84.23618376*B*D^3+570.50243509*B*D^2*E+20.80711976*B*D^2*F+276.14616487*B*D*E^2-187.55469748*B*D*E*F+231.77207882*B*D*F^2+39.6731336*B*E^3-159.44391551*B*E^2*F+474.38429736*B*E*F^2-799.43394388*B*F^3+841.18524636*C^4-465.19276648*C^3*D-493.85469286*C^3*E-565.59652406*C^3*F+25.25341321*C^2*D^2+137.35543887*C^2*D*E+182.02589486*C^2*D*F+231.58309532*C^2*E^2+65.45956578*C^2*E*F+351.8142395*C^2*F^2+175.82604124*C*D^3+378.82131584*C*D^2*E-187.43381488*C*D^2*F+427.89792841*C*D*E^2-163.2361919*C*D*E*F+239.53076901*C*D*F^2-441.6597083*C*E^3-72.56911837*C*E^2*F+433.82583655*C*E*F^2-856.55222008*C*F^3-92.77090641*D^4-584.93085751*D^3*E+170.88879127*D^3*F-176.59980586*D^2*E^2+285.97966317*D^2*E*F+77.82103737*D^2*F^2-778.55977016*D*E^3+331.49569283*D*E^2*F+412.88005697*D*E*F^2-761.08571247*D*F^3+716.84265006*E^4-56.09643789*E^3*F+73.74173358*E^2*F^2-1052.59064854*E*F^3+1562.70740119*F^4";
	// 注药工艺工程计算
	QString inForwardCalculateFormula = "";
	
	bool isChecked = false;
};



// 单例模式的模型数据管理类
class ModelDataManager : public QObject 
{
	Q_OBJECT
private:
	ModelDataManager();

public:
	// 禁止拷贝和赋值（确保单例唯一性）
	ModelDataManager(const ModelDataManager&) = delete;
	ModelDataManager& operator=(const ModelDataManager&) = delete;


	static ModelDataManager* GetInstance();

	// 设置模型数据
	void SetModelGeometryInfo(const ModelGeometryInfo& info);
	const ModelGeometryInfo& GetModelGeometryInfo() const;

	void SetModelMeshInfo(const ModelMeshInfo& info);
	const ModelMeshInfo& GetModelMeshInfo() const;
		
	void SetSteelPropertyInfo(const SteelPropertyInfo& info);
	const SteelPropertyInfo& GetSteelPropertyInfo() const;

	void SetPropellantPropertyInfo(const PropellantPropertyInfo& info);
	const PropellantPropertyInfo& GetPropellantPropertyInfo() const;

	void SetGelatinPropertyInfo(const GelatinPropertyInfo& info);
	const GelatinPropertyInfo& GetGelatinPropertyInfo() const;

	void SetCalculationPropertyInfo(const CalculationPropertyInfo& info);
	const CalculationPropertyInfo& GetCalculationPropertyInfo() const;

	void SetUserInfo(const UserInfo& info);
	const UserInfo& GetUserInfo() const;

	void SetJudgementPropertyInfo(const JudgementPropertyInfo& info);
	const JudgementPropertyInfo& GetJudgementPropertyInfo() const;


	void SetPreForwardPropertyInfo(const PreForwardPropertyInfo& info);
	const PreForwardPropertyInfo& GetPreForwardPropertyInfo() const;

	void SetPreReversePropertyInfo(const PreReversePropertyInfo& info);
	const PreReversePropertyInfo& GetPreReversePropertyInfo() const;

	void SetInForwardPropertyInfo(const InForwardPropertyInfo& info);
	const InForwardPropertyInfo& GetInForwardPropertyInfo() const;

	void SetInReversePropertyInfo(const InReversePropertyInfo& info);
	const InReversePropertyInfo& GetInReversePropertyInfo() const;

	void Reset();

	

private:
	static ModelDataManager* m_Instance; 
	ModelGeometryInfo m_ModelGeometryInfo; 
	ModelMeshInfo m_ModelMeshInfo;

	SteelPropertyInfo m_SteelPropertyInfo;
	PropellantPropertyInfo m_PropellantPropertyInfo;
	GelatinPropertyInfo m_GelatinPropertyInfo;
	CalculationPropertyInfo m_CalculationPropertyInfo;

	JudgementPropertyInfo m_JudgementPropertyInfo;
	
	UserInfo m_UserInfo;

	
	PreForwardPropertyInfo m_PreForwardPropertyInfo;
	PreReversePropertyInfo m_PreReversePropertyInfo;
	InForwardPropertyInfo m_InForwardPropertyInfo;
	InReversePropertyInfo m_InReversePropertyInfo;
};



