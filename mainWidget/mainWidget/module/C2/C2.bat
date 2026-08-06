@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion
::=====================================================================
::                        用户可修改参数区
::=====================================================================
set "WB_PATH=%WORKBENCH_PATH%"
set "SCRIPT_PATH=%~dp0C2.wbjn"
set "ARCHIVE_PATH=%~dp0C2.wbpz"
set "PROJECT_PATH=%~dp0C2.wbpj"
::几何参数
set "GEOM_P1=%GEOM_P1%"
set "GEOM_P2=%GEOM_P2%"
set "GEOM_P3=%GEOM_P3%"
set "GEOM_P5=%GEOM_P5%"
set "GEOM_P6=%GEOM_P6%"
::药液材料 
set	"LIQUID_DENSITY=%LIQUID_DENSITY%"
set	"LIQUID_SPECIFIC_HEAT=%LIQUID_SPECIFIC_HEAT%"
set	"LIQUID_THERMAL_CONDUCTIVITY=%LIQUID_THERMAL_CONDUCTIVITY%"
::胶层材料 
set "BONDLINE_DENSITY=%BONDLINE_DENSITY%"
set "BONDLINE_SPECIFIC_HEAT=%BONDLINE_SPECIFIC_HEAT%"
set "BONDLINE_THERMAL_CONDUCTIVITY=%BONDLINE_THERMAL_CONDUCTIVITY%"
::壳体材料 
set "SHELL_DENSITY=%SHELL_DENSITY%"
set "SHELL_SPECIFIC_HEAT=%SHELL_SPECIFIC_HEAT%"
set "SHELL_THERMAL_CONDUCTIVITY=%SHELL_THERMAL_CONDUCTIVITY%"
::边界条件 
set	"INLET_TOTAL_PRESSURE=%INLET_TOTAL_PRESSURE%"
set	"INLET_TOTAL_TEMPERATURE=%INLET_TOTAL_TEMPERATURE%"
set	"WALL_TEMPERATURE=%WALL_TEMPERATURE%"
::求解控制（新增）
set "NUMBER_OF_TIME_STEPS=%NUMBER_OF_TIME_STEPS%"

:: 启动模式判断
if "%LAUNCH_MODE%"=="batch" (
    echo [模式] 无界面后台运行
    set "WB_ARGS=-B -R"
) else (
    echo [模式] 界面启动
    set "WB_ARGS=-I -R"
)


echo.
echo ======================================================
echo           ANSYS Workbench 2022R2 自动启动
echo ======================================================
echo 启动模式: %LAUNCH_MODE%
echo Workbench路径:%WORKBENCH_PATH%	
echo ArchivePath：%ARCHIVE_PATH%
echo ProjectPath：%PROJECT_PATH%
::几何参数
echo P1：%GEOM_P1%
echo P2：%GEOM_P2%
echo P3：%GEOM_P3%
echo P5：%GEOM_P5%
echo P6：%GEOM_P6%
::药液材料 
echo 药液密度:%LIQUID_DENSITY%
echo 药液比热容:%LIQUID_SPECIFIC_HEAT%
echo 药液热导率:%LIQUID_THERMAL_CONDUCTIVITY%
::胶层材料 
echo 胶层密度:%BONDLINE_DENSITY%
echo 胶层比热容:%BONDLINE_SPECIFIC_HEAT%
echo 胶层热导率:%BONDLINE_THERMAL_CONDUCTIVITY%
::壳体材料 
echo 壳体密度:%SHELL_DENSITY%
echo 壳体比热容:%SHELL_SPECIFIC_HEAT%
echo 壳体热导率:%SHELL_THERMAL_CONDUCTIVITY%
::边界条件 
echo 进气口总压:%INLET_TOTAL_PRESSURE%
echo 进气口总温度:%INLET_TOTAL_TEMPERATURE%
echo 壁温:%WALL_TEMPERATURE%
::求解控制（新增）
echo 时间步数:%NUMBER_OF_TIME_STEPS%


echo.

"%WB_PATH%" %WB_ARGS% "%SCRIPT_PATH%"