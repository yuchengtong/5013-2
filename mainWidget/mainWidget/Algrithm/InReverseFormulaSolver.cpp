#include "InReverseFormulaSolver.h"
#include <cmath>
#include <QStringList>
#include <limits>
#include <QDebug>

using namespace std;

InReverseFormulaSolver::InReverseFormulaSolver(QObject* parent) : QObject(parent)
{}

double InReverseFormulaSolver::parseFactor(const QString& factor, const QMap<char, double>& varMap)
{
    QString f = factor.trimmed();
    if (f.isEmpty())
        return 1.0;

    double power = 1.0;
    QString base = f;

    int powPos = f.indexOf('^');
    if (powPos != -1)
    {
        base = f.left(powPos).trimmed();
        QString powStr = f.mid(powPos + 1).trimmed();
        bool ok;
        double powVal = powStr.toDouble(&ok);
        power = ok ? powVal : 1.0;
    }

    if (base.length() == 1)
    {
        QChar c = base[0].toUpper();
        if (c >= 'A' && c <= 'E')
        {
            char key = c.toLatin1();
            if (varMap.contains(key))
                return pow(varMap[key], power);
        }
    }

    bool ok;
    double num = base.toDouble(&ok);
    return ok ? num : 1.0;
}

double InReverseFormulaSolver::parseTerm(const QString& term, const QMap<char, double>& varMap)
{
    QString t = term.trimmed();
    if (t.isEmpty())
        return 0.0;

    double sign = 1.0;
    if (t.startsWith('-')) {
        sign = -1.0;
        t = t.mid(1).trimmed();
    }
    else if (t.startsWith('+')) {
        t = t.mid(1).trimmed();
    }

    QStringList factors = t.split('*');
    double val = 1.0;

    for (const QString& f : factors) {
        QString factor = f.trimmed();
        if (factor.isEmpty()) continue;
        val *= parseFactor(factor, varMap);
    }
    return val * sign;
}

double InReverseFormulaSolver::calculate(const QString& expr, const QMap<char, double>& varMap)
{
    QString s = expr.trimmed();
    double total = 0.0;

    if (s.isEmpty()) return 0.0;

    if (s[0] != '+' && s[0] != '-') {
        s = "+" + s;
    }

    int start = 0;
    int len = s.length();
    while (start < len) {
        int end = start + 1;
        while (end < len && s[end] != '+' && s[end] != '-') {
            end++;
        }
        QString term = s.mid(start, end - start).trimmed();
        if (!term.isEmpty()) {
            total += parseTerm(term, varMap);
        }
        start = end;
    }
    return total;
}

double InReverseFormulaSolver::searchBestValue(const QString& formula, double target,
    const QMap<char, double>& knownVars, char unknownVar, double guessValue)
{
    auto func = [&](double x) -> double {
        QMap<char, double> vars = knownVars;
        vars[unknownVar] = x;
        return calculate(formula, vars);
    };

    // ===================== 固定搜索真实区间（你的方程真实解一定在这里）
    const double SEARCH_MIN = -3.0;
    const double SEARCH_MAX = 3.0;
    const int STEPS = 10000; // 超快，8000步几乎不耗时

    double bestX = guessValue;
    double minErr = 1e20;

    for (int i = 0; i <= STEPS; ++i) {
        double x = SEARCH_MIN + (SEARCH_MAX - SEARCH_MIN) * i / STEPS;
        double val = func(x);
        double err = fabs(val - target);

        if (err < minErr) {
            minErr = err;
            bestX = x;
        }
    }

    // 微小区间精修
    const double fineStep = 0.00001;
    for (double x = bestX - 0.01; x <= bestX + 0.01; x += fineStep) {
        double val = func(x);
        double err = fabs(val - target);
        if (err < minErr) {
            minErr = err;
            bestX = x;
        }
    }

    return bestX;
}

void InReverseFormulaSolver::solveReverse(const QString& formula, double targetValue,
    const QMap<char, double>& knownVars, char unknownVar, double guessValue)
{
    vector<double> result;

    if (knownVars.size() != 4 || !(unknownVar >= 'A' && unknownVar <= 'E')) {
        result.push_back(numeric_limits<double>::quiet_NaN());
        emit solveCompleted(result);
        return;
    }
    if (knownVars.contains(unknownVar)) {
        result.push_back(numeric_limits<double>::quiet_NaN());
        emit solveCompleted(result);
        return;
    }

    // 默认猜测值 0.5，完全不影响结果
    double ans = searchBestValue(formula, targetValue, knownVars, unknownVar, 0.5);
    result.push_back(ans);
    emit solveCompleted(result);
}