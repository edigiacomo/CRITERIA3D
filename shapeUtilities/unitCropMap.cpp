#include "unitCropMap.h"
#include "zonalStatistic.h"
#include "shapeToRaster.h"
#include "shapeUtilities.h"
#include "formInfo.h"
#include "computationUnitsDb.h"

#include <QFile>


bool computeUcmPrevailing(Crit3DShapeHandler &ucm, Crit3DShapeHandler &crop, Crit3DShapeHandler &soil, Crit3DShapeHandler &meteo,
                 std::string idCrop, std::string idSoil, std::string idMeteo, double cellSize,
                 QString ucmFileName, std::string &error, bool showInfo)
{

    // make a copy of shapefile and return cloned shapefile complete path
    QString refFileName = QString::fromStdString(crop.getFilepath());
    QString ucmShapeFileName = cloneShapeFile(refFileName, ucmFileName);

    if (!ucm.open(ucmShapeFileName.toStdString()))
    {
        error = "Load shapefile failed: " + ucmShapeFileName.toStdString();
        return false;
    }

    // create reference and value raster
    gis::Crit3DRasterGrid rasterRef;
    gis::Crit3DRasterGrid rasterVal;
    initializeRasterFromShape(ucm, rasterRef, cellSize);
    initializeRasterFromShape(ucm, rasterVal, cellSize);

    FormInfo formInfo;

    // CROP (reference shape)
    if (showInfo) formInfo.start("[1/8] Rasterize crop (reference)...", 0);
    fillRasterWithShapeNumber(rasterRef, ucm);

    // meteo grid
    if (showInfo) formInfo.setText("[2/8] Rasterize meteo grid...");
    fillRasterWithShapeNumber(rasterVal, meteo);

    if (showInfo) formInfo.setText("[3/8] Compute matrix crop/meteo...");
    std::vector <int> vectorNull;
    std::vector <std::vector<int> > matrix = computeMatrixAnalysis(ucm, meteo, rasterRef, rasterVal, vectorNull);

    if (showInfo) formInfo.setText("[4/8] Zonal statistic crop/meteo...");
    bool isOk = zonalStatisticsShapeMajority(ucm, meteo, matrix, vectorNull, idMeteo, "ID_METEO", error);

    // zonal statistic on soil map
    if (isOk)
    {
        if (showInfo) formInfo.setText("[5/8] Rasterize soil...");
        fillRasterWithShapeNumber(rasterVal, soil);

        if (showInfo) formInfo.setText("[6/8] Compute matrix crop/soil...");
        matrix = computeMatrixAnalysis(ucm, soil, rasterRef, rasterVal, vectorNull);

        if (showInfo) formInfo.setText("[7/8] Zonal statistic crop/soil...");
        isOk = zonalStatisticsShapeMajority(ucm, soil, matrix, vectorNull, idSoil, "ID_SOIL", error);
    }

    if (! isOk)
    {
        error = "ZonalStatisticsShape: " + error;
    }

    rasterRef.clear();
    rasterVal.clear();
    matrix.clear();
    vectorNull.clear();

    if (! isOk)
    {
        if (showInfo) formInfo.close();
        return false;
    }

    if (showInfo) formInfo.setText("[8/8] Write UCM...");

    // add ID CASE
    ucm.addField("ID_CASE", FTString, 20, 0);
    int idCaseIndex = ucm.getFieldPos("ID_CASE");

    // add ID CROP
    bool existIdCrop = ucm.existField("ID_CROP");
    if (! existIdCrop) ucm.addField("ID_CROP", FTString, 5, 0);
    int idCropIndex = ucm.getFieldPos("ID_CROP");

    // read indexes
    int nShape = ucm.getShapeCount();
    int cropIndex = ucm.getFieldPos(idCrop);
    int soilIndex = ucm.getFieldPos(idSoil);
    int meteoIndex = ucm.getFieldPos(idMeteo);

    // FILL ID_CROP and ID_CASE
    for (int shapeIndex = 0; shapeIndex < nShape; shapeIndex++)
    {
        std::string cropStr = ucm.readStringAttribute(shapeIndex, cropIndex);
        if (cropStr == "-9999") cropStr = "";

        std::string soilStr = ucm.readStringAttribute(shapeIndex, soilIndex);
        if (soilStr == "-9999") soilStr = "";

        std::string meteoStr = ucm.readStringAttribute(shapeIndex, meteoIndex);
        if (meteoStr == "-9999") meteoStr = "";

        std::string caseStr = "";
        if (meteoStr != "" && soilStr != "" && cropStr != "")
            caseStr = "M" + meteoStr + "S" + soilStr + "C" + cropStr;

        if (! existIdCrop) ucm.writeStringAttribute(shapeIndex, idCropIndex, cropStr.c_str());
        ucm.writeStringAttribute(shapeIndex, idCaseIndex, caseStr.c_str());

        if (caseStr == "")
            ucm.deleteRecord(shapeIndex);
    }

    if (showInfo) formInfo.close();
    cleanShapeFile(ucm);

    return isOk;
}


bool fillUcmIdCase(Crit3DShapeHandler &ucm, std::string idCrop, std::string idSoil, std::string idMeteo)
{
    if (!ucm.existField("ID_CASE"))
    {
        return false;
    }
    // read indexes
    int nShape = ucm.getShapeCount();
    int cropIndex = ucm.getFieldPos(idCrop);
    int soilIndex = ucm.getFieldPos(idSoil);
    int meteoIndex = ucm.getFieldPos(idMeteo);
    int idCaseIndex = ucm.getFieldPos("ID_CASE");

    if (cropIndex == -1 || soilIndex == -1 || meteoIndex == -1)
    {
        return false;
    }
    for (int shapeIndex = 0; shapeIndex < nShape; shapeIndex++)
    {
        std::string cropStr = ucm.readStringAttribute(shapeIndex, cropIndex);
        if (cropStr == "-9999") cropStr = "";

        std::string soilStr = ucm.readStringAttribute(shapeIndex, soilIndex);
        if (soilStr == "-9999") soilStr = "";

        std::string meteoStr = ucm.readStringAttribute(shapeIndex, meteoIndex);
        if (meteoStr == "-9999") meteoStr = "";

        std::string caseStr = "";
        if (meteoStr != "" && soilStr != "" && cropStr != "")
            caseStr = "M" + meteoStr + "S" + soilStr + "C" + cropStr;

        ucm.writeStringAttribute(shapeIndex, idCaseIndex, caseStr.c_str());

        if (caseStr == "")
            ucm.deleteRecord(shapeIndex);
    }
    return true;
}


bool writeUcmListToDb(Crit3DShapeHandler &shapeHandler, QString dbName, std::string &error)
{
    QStringList idCase, idCrop, idMeteo, idSoil;
    QList<double> ha;

    int nShape = shapeHandler.getShapeCount();

    for (int i = 0; i < nShape; i++)
    {
        QString key = QString::fromStdString(shapeHandler.getStringValue(signed(i), "ID_CASE"));
        if (key.isEmpty()) continue;

        if ( !idCase.contains(key) )
        {
            idCase << key;
            idCrop << QString::fromStdString(shapeHandler.getStringValue(signed(i), "ID_CROP"));
            idMeteo << QString::fromStdString(shapeHandler.getStringValue(signed(i), "ID_METEO"));
            idSoil << QString::fromStdString(shapeHandler.getStringValue(signed(i), "ID_SOIL"));
            ha << shapeHandler.getNumericValue(signed(i), "HA");
        }
        else
        {
            // TODO search id_case and sum ectars
        }
    }

    ComputationUnitsDB unitsDb(dbName);

    bool writeResult = unitsDb.writeListToUnitsTable(idCase, idCrop, idMeteo, idSoil, ha);
    error = unitsDb.getError().toStdString();
    unitsDb.clear();

    return writeResult;
}
