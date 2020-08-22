#ifndef SHAPEFROMCSV_H
#define SHAPEFROMCSV_H

    #ifndef SHAPEHANDLER_H
        #include "shapeHandler.h"
    #endif
    #include <QString>

    bool shapeFromCsv(Crit3DShapeHandler &refShapeFile, Crit3DShapeHandler &outputShapeFile, QString csvFileName,
                      QString fieldListFileName, QString outputFileName, QString &error);

#endif // SHAPEFROMCSV_H
