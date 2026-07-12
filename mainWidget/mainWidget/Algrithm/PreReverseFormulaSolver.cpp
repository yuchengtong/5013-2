#include "PreReverseFormulaSolver.h"
#include <cmath>
#include <QStringList>
#include <limits>
using namespace std;
PreReverseFormulaSolver::PreReverseFormulaSolver(QObject* parent) : QObject(parent)
{}

double PreReverseFormulaSolver::parseFactor(const QString& factor, double A, double B, double C, double D, double E, double F, double G)
{
    QString f = factor.trimmed();
    if (f.isEmpty()) return 1.0;
    QChar var = f[0].toUpper();
    int power = 1;
    if (f.contains("^"))
    {
        QStringList sp = f.split("^");
        if (sp.size() >= 2)
            power = sp[1].toInt();
    }
    if (var == 'A') return pow(A, power);
    if (var == 'B') return pow(B, power);
    if (var == 'C') return pow(C, power);
    if (var == 'D') return pow(D, power);
    if (var == 'E') return pow(E, power);
    if (var == 'F') return pow(F, power);
    if (var == 'G') return pow(G, power);
    return f.toDouble();
}

double PreReverseFormulaSolver::parseTerm(const QString& term, double A, double B, double C, double D, double E, double F, double G)
{
    QString t = term.trimmed();
    if (t.isEmpty()) return 0.0;
    QStringList factors = t.split("*");
    double val = 1.0;
    for (QString f : factors)
    {
        f = f.trimmed();
        if (f.isEmpty()) continue;
        val *= parseFactor(f, A, B, C, D, E, F, G);
    }
    return val;
}

double PreReverseFormulaSolver::evaluate(const QString& expr, double A, double B, double C, double D, double E, double F, double G)
{
    QString s = expr;
    s.replace("-", "+-");
    QStringList terms = s.split("+");
    double total = 0.0;
    for (QString term : terms)
    {
        term = term.trimmed();
        if (term.isEmpty()) continue;
        total += parseTerm(term, A, B, C, D, E, F, G);
    }
    return total;
}

double PreReverseFormulaSolver::findBestMatch(const QString& formula, double target,
    double A, double B, double C, double D, double E, double F, double G, char uv)
{
    // 关键：上限缩到 0.9，禁止跑到1.0
    const int steps = 2000;
    const double minX = 0.0;
    const double maxX = 1.5;
    double bestX = 0.5;
    double minErr = 1e20;
    auto func = [&](double x) -> double
    {
        if (uv == 'A') return evaluate(formula, x, B, C, D, E, F, G);
        if (uv == 'B') return evaluate(formula, A, x, C, D, E, F, G);
        if (uv == 'C') return evaluate(formula, A, B, x, D, E, F, G);
        if (uv == 'D') return evaluate(formula, A, B, C, x, E, F, G);
        if (uv == 'E') return evaluate(formula, A, B, C, D, x, F, G);
        if (uv == 'F') return evaluate(formula, A, B, C, D, E, x, G);
        if (uv == 'G') return evaluate(formula, A, B, C, D, E, F, x);
        return 0.0;
    };
    // 粗遍历找最优区间
    for (int i = 0; i <= steps; ++i)
    {
        double x = minX + (maxX - minX) * i / steps;
        double val = func(x);
        double err = fabs(val - target);
        if (err < minErr)
        {
            minErr = err;
            bestX = x;
        }
    }
    // 在最优点附近再精细局部求精，锁定小数
    double left = bestX - 0.05;
    double right = bestX + 0.05;
    if (left < minX) left = minX;
    if (right > maxX) right = maxX;
    for (int i = 0; i <= 500; ++i)
    {
        double x = left + (right - left) * i / 500;
        double val = func(x);
        double err = fabs(val - target);
        if (err < minErr)
        {
            minErr = err;
            bestX = x;
        }
    }
    return bestX;
}

void PreReverseFormulaSolver::solve(const QString& formula, double targetValue,
    double A, double B, double C, double D, double E, double F, double G,
    char unknownVar)
{
    vector<double> res;
    double ans = findBestMatch(formula, targetValue, A, B, C, D, E, F, G, unknownVar);
    res.push_back(ans);
    emit solveFinished(res);
}