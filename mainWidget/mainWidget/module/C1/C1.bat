@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion
::=====================================================================
::                        客户可修改参数区
::=====================================================================
set "WB_PATH=C:\work\ANSYS Inc\v222\Framework\bin\Win64\runwb2.exe"
set "SCRIPT_PATH=C:\work\Test\C1\C1.wbjn"
echo.
echo ======================================================
echo           ANSYS Workbench 2022R2 自动启动
echo ======================================================
echo  脚本路径：%SCRIPT_PATH%
echo.
"%WB_PATH%" -I -R "%SCRIPT_PATH%"
