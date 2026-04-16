#include "APICalculateHepler.h"

#include <V3d_View.hxx>

#include <QMap>
#include <QRandomGenerator>

#include "ModelDataManager.h"


double calculate(const QString& formula,
	double A, double B, double C, double D, double E, double F)
{
	A = (A - 50) / 35;
	B = (B - 20) / 10;
	C = (C - 1) / 4;
	D = (D - 2160) / 7260;
	E = (E - 368) / 736;
	F = (F - 6) / 150;

	QString processedFormula = formula;
	processedFormula.remove(' '); // 移除所有空格

	// 变量映射：公式二使用 A-F
	const QMap<QString, double> varMap = {
		{"A", A}, {"B", B}, {"C", C}, {"D", D}, {"E", E}, {"F", F}
	};

	/************************ 核心：支持 1/2/3/4次项 + 多变量乘积 ************************/
	QRegExp regExp("([+-]?)((?:\\d+(?:\\.\\d*)?)|(?:\\.\\d+))(\\*[A-F](?:\\^[234])?(?:\\*[A-F](?:\\^[234])?)*)?");
	regExp.setMinimal(false);

	double result = 0.0;
	int pos = 0;
	int matchCount = 0;

	// 补全开头符号，统一逻辑（和代码一完全一致）
	if (!processedFormula.isEmpty() && processedFormula[0] != '+' && processedFormula[0] != '-') {
		processedFormula = "+" + processedFormula;
	}

	// 循环匹配所有项（结构和代码一完全一致）
	while ((pos = regExp.indexIn(processedFormula, pos)) != -1) {
		++matchCount;

		QString signStr = regExp.cap(1);    // 符号 +/-
		QString coeffStr = regExp.cap(2);   // 系数
		QString varPart = regExp.cap(3);    // 变量部分（如 *A^2*B^3）

		// 1. 符号解析（和代码一一致）
		double sign = (signStr == "-") ? -1.0 : 1.0;

		// 2. 系数解析（和代码一一致）
		bool ok = false;
		double coeff = coeffStr.toDouble(&ok);
		if (!ok) {
			throw std::invalid_argument(QString("无效系数: %1").arg(coeffStr).toStdString());
		}

		// 3. 计算变量乘积值（支持 ^2 ^3 ^4）
		double varValue = 1.0;
		if (!varPart.isEmpty()) {
			// 去掉开头的 *，按 * 拆分
			QString vars = varPart.mid(1);
			QStringList units = vars.split('*');

			for (const QString& unit : units) {
				if (unit.contains('^')) {
					// 处理次方项 A^3 / F^4
					QStringList parts = unit.split('^');
					QString vName = parts[0];
					int power = parts[1].toInt();

					if (!varMap.contains(vName)) {
						throw std::invalid_argument(QString("未知变量: %1").arg(vName).toStdString());
					}
					double val = varMap[vName];
					varValue *= std::pow(val, power);
				}
				else {
					// 单次变量 A/B/C
					if (!varMap.contains(unit)) {
						throw std::invalid_argument(QString("未知变量: %1").arg(unit).toStdString());
					}
					varValue *= varMap[unit];
				}
			}
		}

		// 累加当前项
		result += sign * coeff * varValue;
		pos += regExp.matchedLength();
	}

	// 公式校验
	if (matchCount == 0) {
		throw std::invalid_argument(QString("公式格式错误: %1").arg(formula).toStdString());
	}

	return result;
}




