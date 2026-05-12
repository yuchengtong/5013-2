#ifndef INREVERSEFORMULASOLVER_H
#define INREVERSEFORMULASOLVER_H

#include <QObject>
#include <vector>
#include <QMap>
#include <QString>

class InReverseFormulaSolver : public QObject
{
    Q_OBJECT
public:
    explicit InReverseFormulaSolver(QObject* parent = nullptr);

    void solveReverse(const QString& formula, double targetValue,
        const QMap<char, double>& knownVars, char unknownVar,
        double guessValue = 0.5);  // Ôö¼Ó²Â²âÖµ

signals:
    void solveCompleted(const std::vector<double>& resultList);

private:
    double parseFactor(const QString& factor, const QMap<char, double>& varMap);
    double parseTerm(const QString& term, const QMap<char, double>& varMap);
    double calculate(const QString& expr, const QMap<char, double>& varMap);
    double searchBestValue(const QString& formula, double target,
        const QMap<char, double>& knownVars, char unknownVar,
        double guessValue);
};

#endif