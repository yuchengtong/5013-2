#ifndef PREREVERSEFORMULASOLVER_H
#define PREREVERSEFORMULASOLVER_H

#include <QObject>
#include <vector>
#include <QString>

class PreReverseFormulaSolver : public QObject
{
    Q_OBJECT
public:
    explicit PreReverseFormulaSolver(QObject* parent = nullptr);

public slots:
    void solve(const QString& formula, double targetValue,
        double A, double B, double C, double D, double E, double F,
        char unknownVar);

signals:
    void solveFinished(const std::vector<double>& resList);

private:
    double evaluate(const QString& expr, double A, double B, double C, double D, double E, double F);
    double findBestMatch(const QString& formula, double target,
        double A, double B, double C, double D, double E, double F, char uv);

    double parseTerm(const QString& term, double A, double B, double C, double D, double E, double F);
    double parseFactor(const QString& factor, double A, double B, double C, double D, double E, double F);
};

#endif