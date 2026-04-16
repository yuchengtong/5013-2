#include "ModelDataManager.h"
#include <fstream>
#include <iostream>
#include <sstream>

// 初始化静态成员（单例实例）
ModelDataManager* ModelDataManager::m_Instance = nullptr;

ModelDataManager::ModelDataManager()
{
	//初始化
	Reset();
}

// 获取单例实例
ModelDataManager* ModelDataManager::GetInstance() {
	if (m_Instance == nullptr) {
		m_Instance = new ModelDataManager();
	}
	return m_Instance;
}

// 设置模型数据
void ModelDataManager::SetModelGeometryInfo(const ModelGeometryInfo& info)
{
	m_ModelGeometryInfo = info;
}

// 获取模型数据
const ModelGeometryInfo& ModelDataManager::GetModelGeometryInfo() const 
{
	return m_ModelGeometryInfo;
}

void ModelDataManager::SetModelMeshInfo(const ModelMeshInfo & info)
{
	m_ModelMeshInfo = info;
}

const ModelMeshInfo & ModelDataManager::GetModelMeshInfo() const
{
	return m_ModelMeshInfo;
}


// 材料库
const SteelPropertyInfo & ModelDataManager::GetSteelPropertyInfo() const
{
	return m_SteelPropertyInfo;
}

void ModelDataManager::SetSteelPropertyInfo(const SteelPropertyInfo& info)
{
	m_SteelPropertyInfo = info;
}

const PropellantPropertyInfo & ModelDataManager::GetPropellantPropertyInfo() const
{
	return m_PropellantPropertyInfo;
}

void ModelDataManager::SetPropellantPropertyInfo(const PropellantPropertyInfo& info)
{
	m_PropellantPropertyInfo = info;
}

const GelatinPropertyInfo& ModelDataManager::GetGelatinPropertyInfo() const
{
	return m_GelatinPropertyInfo;
}

void ModelDataManager::SetGelatinPropertyInfo(const GelatinPropertyInfo& info)
{
	m_GelatinPropertyInfo = info;
}

const CalculationPropertyInfo & ModelDataManager::GetCalculationPropertyInfo() const
{
	return m_CalculationPropertyInfo;
}

void ModelDataManager::SetCalculationPropertyInfo(const CalculationPropertyInfo& info)
{
	m_CalculationPropertyInfo = info;
}

const UserInfo & ModelDataManager::GetUserInfo() const
{
	return m_UserInfo;
}

void ModelDataManager::SetUserInfo(const UserInfo& info)
{
	m_UserInfo = info;
}

void ModelDataManager::SetJudgementPropertyInfo(const JudgementPropertyInfo & info)
{
	m_JudgementPropertyInfo = info;
}

const JudgementPropertyInfo & ModelDataManager::GetJudgementPropertyInfo() const
{
	return m_JudgementPropertyInfo;
}

void ModelDataManager::SetPreForwardPropertyInfo(const PreForwardPropertyInfo& info)
{
	m_PreForwardPropertyInfo = info;
}

const PreForwardPropertyInfo& ModelDataManager::GetPreForwardPropertyInfo() const
{
	return m_PreForwardPropertyInfo;
}

void ModelDataManager::SetPreReversePropertyInfo(const PreReversePropertyInfo& info)
{
	m_PreReversePropertyInfo = info;
}

const PreReversePropertyInfo& ModelDataManager::GetPreReversePropertyInfo() const
{
	return m_PreReversePropertyInfo;
}

void ModelDataManager::SetInForwardPropertyInfo(const InForwardPropertyInfo& info)
{
	m_InForwardPropertyInfo = info;
}

const InForwardPropertyInfo& ModelDataManager::GetInForwardPropertyInfo() const
{
	return m_InForwardPropertyInfo;
}

void ModelDataManager::SetInReversePropertyInfo(const InReversePropertyInfo& info)
{
	m_InReversePropertyInfo = info;
}

const InReversePropertyInfo& ModelDataManager::GetInReversePropertyInfo() const
{
	return m_InReversePropertyInfo;
}

// 重置数据
void ModelDataManager::Reset() 
{
	m_ModelGeometryInfo = ModelGeometryInfo();
	m_ModelMeshInfo = ModelMeshInfo();

	m_SteelPropertyInfo = SteelPropertyInfo();
	m_PropellantPropertyInfo = PropellantPropertyInfo();
	m_GelatinPropertyInfo = GelatinPropertyInfo();
	m_CalculationPropertyInfo = CalculationPropertyInfo();
}

