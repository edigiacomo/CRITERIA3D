#ifndef CRITERIAOUTPUTELABORATION_H
#define CRITERIAOUTPUTELABORATION_H

#include <QString>
#include <QDate>
#include <QSqlDatabase>
#include <QFile>
#include "criteriaOutputVariable.h"

int writeCsvOutputUnit(QString idCase, QString idCropClass, QSqlDatabase dbData, QSqlDatabase dbCrop, QSqlDatabase dbDataHistorical,
                       QDate dateComputation, CriteriaOutputVariable outputVariable, QString csvFileName, QString* projectError);
int selectSimpleVar(QSqlDatabase db, QString idCase, QString varName, QString computation, QDate firstDate, QDate lastDate, float irriRatio, QVector<float>* resVector, QString *projectError);
int computeDTX(QSqlDatabase db, QString idCase, int period, QString computation, QDate firstDate, QDate lastDate, QVector<float>* resVector, QString *projectError);

#endif // CRITERIAOUTPUTELABORATION_H
