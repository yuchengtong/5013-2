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

    int power = 1;
    QString base = f;

    int powPos = f.indexOf('^');
    if (powPos != -1)
    {
        base = f.left(powPos).trimmed();
        QString powStr = f.mid(powPos + 1).trimmed();
        power = powStr.toInt();
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

    QStringList factors = t.split('*');
    double val = 1.0;

    for (int i = 0; i < factors.size(); ++i)
    {
        QString f = factors[i].trimmed();
        if (f.isEmpty()) continue;
        val *= parseFactor(f, varMap);
    }
    return val;
}

double InReverseFormulaSolver::calculate(const QString& expr, const QMap<char, double>& varMap)
{
    QString s = expr.trimmed();
    double total = 0.0;

    if (s.isEmpty()) return 0.0;
    if (s[0] != '+' && s[0] != '-')
        s = "+" + s;

    int start = 0;
    int len = s.length();
    while (start < len)
    {
        int end = start + 1;
        while (end < len && s[end] != '+' && s[end] != '-')
            end++;

        QString term = s.mid(start, end - start).trimmed();
        start = end;

        if (!term.isEmpty())
            total += parseTerm(term, varMap);
    }
    return total;
}

// 【精准搜索】围绕猜测值搜索
double InReverseFormulaSolver::searchBestValue(const QString& formula, double target,
    const QMap<char, double>& knownVars, char unknownVar,
    double guessValue)
{
    const double SEARCH_RANGE = 0.3;   // 小范围高精度搜索
    const int STEPS = 10000;          // 超高精度

    double bestX = guessValue;
    double minErr = 1e20;

    auto func = [&](double x) -> double {
        QMap<char, double> vars = knownVars;
        vars[unknownVar] = x;
        return calculate(formula, vars);
    };

    double minX = guessValue - SEARCH_RANGE;
    double maxX = guessValue + SEARCH_RANGE;

    // 超高精度小范围搜索
    for (int i = 0; i <= STEPS; ++i)
    {
        double x = minX + (maxX - minX) * i / STEPS;
        double err = fabs(func(x) - target);
        if (err < minErr)
        {
            minErr = err;
            bestX = x;
        }
    }

    return bestX;
}

void InReverseFormulaSolver::solveReverse(const QString& formula, double targetValue,
    const QMap<char, double>& knownVars, char unknownVar,
    double guessValue)
{
    vector<double> result;

    if (knownVars.size() != 4 || !(unknownVar >= 'A' && unknownVar <= 'E'))
    {
        result.push_back(numeric_limits<double>::quiet_NaN());
        emit solveCompleted(result);
        return;
    }

    if (knownVars.contains(unknownVar))
    {
        result.push_back(numeric_limits<double>::quiet_NaN());
        emit solveCompleted(result);
        return;
    }

    double ans = searchBestValue(formula, targetValue, knownVars, unknownVar, guessValue);
    result.push_back(ans);
    emit solveCompleted(result);
}