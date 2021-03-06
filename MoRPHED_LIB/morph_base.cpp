/*
This file is part of MoRPHED

MoRPHED is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MoRPHED is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

(c)2015 MoRPHED Development team
*/
#include "morph_base.h"

MORPH_Base::MORPH_Base(QString xmlPath)
{
    XmlInit.loadDocument(xmlPath, 1);
    qsXmlPath = xmlPath;
    init();
}

MORPH_Base::~MORPH_Base()
{
    GDALDestroyDriverManager();
}

int MORPH_Base::getCurrentIteration()
{
    return nCurrentIteration;
}

int MORPH_Base::getIterations()
{
    return nIterations;
}

int MORPH_Base::getUsBoundary()
{
    return nDirUSbound;
}

void MORPH_Base::setCurrentIteration(int iter)
{
    nCurrentIteration = iter;
    qsFloodName = MORPH_FileManager::getFloodName(iter+1);
}

double MORPH_Base::findMaxVector(QVector<double> vector)
{
    double max =0;

    for (int i=0; i<vector.size(); i++)
    {
        if (i == 0)
        {
            max = vector[i];
        }

        else
        {
            if (max < vector[i])
            {
                max = vector[i];
            }
        }
    }
    return max;
}

void MORPH_Base::loadInputText(QString filename, QVector<double> &dates, QVector<double> &discharge, QVector<double> &waterElev, QVector<double> &sediment)
{
    dates.clear();
    discharge.clear();
    waterElev.clear();
    sediment.clear();

    //declare temp variables to hold stream data
    QString qsDate, qsQ, qsDSWE, qsImport;
    QDateTime tempDate;
    int count = 0;

    //load file
    QFile in(filename);
    if (in.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&in);
        while (!stream.atEnd())
        {
            //read elements from stream to temp variable, convert to double, and store in a QVector
            stream >> qsDate;
            tempDate = QDateTime::fromString(qsDate, "MM/dd/yyyy,hh:mm");
            dates.append(tempDate.toTime_t());
            stream >> qsQ;
            discharge.append(qsQ.toDouble());
            stream >> qsDSWE;
            waterElev.append(qsDSWE.toDouble());
            stream >> qsImport;
            sediment.append(qsImport.toDouble());

            //Each line represents 1 model iteration, increment model iterations after each line is read
            //qDebug()<<date[nIterations]<<" "<<q[nIterations]<<" "<<dswe[nIterations]<<" "<<import[nIterations];
            count++;
        }
    }
}

void MORPH_Base::init()
{
    loadDrivers();

    qsInputText = XmlInit.readNodeData("Inputs", "HydroSediPath");
    loadInputText();

    qsRootPath = XmlInit.readNodeData("ProjectDirectory");
    qsTempPath = qsRootPath + "/temp";
    qsOldDemPath = qsTempPath + "/olddem.tif";
    qsNewDemPath = qsTempPath + "/newdem.tif";
    qsDelftDemPath = qsTempPath + "/delftdem.tif";

    qsOrigDemPath = XmlInit.readNodeData("Inputs", "DEMPath");
    GDALDataset *pTemp, *pTemp2, *pTemp3, *pTemp4;
    GDALAllRegister();
    pTemp = (GDALDataset*) GDALOpen(qsOrigDemPath.toStdString().c_str(), GA_ReadOnly);
    pTemp->GetGeoTransform(transform);
    nRows = pTemp->GetRasterBand(1)->GetYSize();
    nCols = pTemp->GetRasterBand(1)->GetXSize();
    noData = pTemp->GetRasterBand(1)->GetNoDataValue();

    //create copies of original dataset
    pDriverTIFF = GetGDALDriverManager()->GetDriverByName("GTiff");
    pTemp2 = pDriverTIFF->CreateCopy(qsOldDemPath.toStdString().c_str(), pTemp, FALSE, NULL, NULL, NULL);
    pTemp3 = pDriverTIFF->CreateCopy(qsNewDemPath.toStdString().c_str(), pTemp, FALSE, NULL, NULL, NULL);
    pTemp4 = pDriverTIFF->CreateCopy(qsDelftDemPath.toStdString().c_str(), pTemp, FALSE, NULL, NULL, NULL);
    GDALClose(pTemp);
    GDALClose(pTemp2);
    GDALClose(pTemp3);
    GDALClose(pTemp4);

    cellWidth = transform[1];
    cellHeight = transform[5];
    topLeftX = transform[0];
    topLeftY = transform[3];

    //nIterations = XmlInit.readNodeData("Inputs", "ModelIterations").toInt();
    nDirUSbound = XmlInit.readNodeData("Delft3DParameters", "USBoundLocation").toInt();
    nDirDSbound = XmlInit.readNodeData("Delft3DParameters", "DSBoundLocation").toInt();
    nImportType = XmlInit.readNodeData("Inputs", "ImportType").toInt();
    nCurrentIteration = 0;

    nPlDistType1 = XmlInit.readNodeData("MorphedParameters", "PathLength1","DistributionType").toInt();
    nPlDistType2 = XmlInit.readNodeData("MorphedParameters", "PathLengthImport","DistributionType").toInt();
    nDirDSbound = XmlInit.readNodeData("Delft3DParameters", "DSBoundLocation").toInt();
    nDirUSbound = XmlInit.readNodeData("Delft3DParameters", "USBoundLocation").toInt();


    sigA1 = XmlInit.readNodeData("MorphedParameters", "PathLength1","SigA").toDouble();
    muB1 = XmlInit.readNodeData("MorphedParameters", "PathLength1","MuB").toDouble();
    plDistLength1 = XmlInit.readNodeData("MorphedParameters", "PathLength1", "DistributionLength").toDouble();
    sigA2 = XmlInit.readNodeData("MorphedParameters", "PathLengthImport","SigA").toDouble();
    muB2 = XmlInit.readNodeData("MorphedParameters", "PathLengthImport","MuB").toDouble();
    plDistLength2 = XmlInit.readNodeData("MorphedParameters", "PathLengthImport", "DistributionLength").toDouble();
    bankSlopeThresh = XmlInit.readNodeData("MorphedParameters", "BankSlopeThresh").toDouble();
    bankShearThresh = XmlInit.readNodeData("MorphedParameters", "BankShearThresh").toDouble();
    areaThresh = XmlInit.readNodeData("MorphedParameters", "BankAreaThresh").toDouble();
    erosionFactor = XmlInit.readNodeData("MorphedParameters", "ErosionFactor").toDouble();
    gsActiveSize = XmlInit.readNodeData("MorphedParameters", "GrainSize").toDouble();

    simTime = XmlInit.readNodeData("Delft3DParameters", "SimTime").toDouble();
    timeStep = XmlInit.readNodeData("Delft3DParameters", "TimeStep").toDouble();
    roughness = XmlInit.readNodeData("Delft3DParameters", "Roughness").toDouble();
    hev = XmlInit.readNodeData("Delft3DParameters", "HEV").toDouble();

    qsInputPath = qsRootPath + "/Inputs";
    qsOutputPath = qsRootPath + "/Outputs";


    qsFloodName = MORPH_FileManager::getFloodName(nCurrentIteration);
}

void MORPH_Base::loadDrivers()
{
    pDriverTIFF = GetGDALDriverManager()->GetDriverByName("GTiff");
}

void MORPH_Base::loadInputText()
{
    date.clear();
    q.clear();
    dswe.clear();
    import.clear();

    //declare temp variables to hold stream data
    QString qsDate, qsQ, qsDSWE, qsImport;
    QDateTime tempDate;
    nIterations = 0;

    //load file
    QFile in(qsInputText);
    if (in.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&in);
        while (!stream.atEnd())
        {
            //read elements from stream to temp variable, convert to double, and store in a QVector
            stream >> qsDate;
            tempDate = QDateTime::fromString(qsDate, "MM/dd/yyyy,hh:mm");
            date.append(tempDate.toTime_t());
            stream >> qsQ;
            q.append(qsQ.toDouble());
            stream >> qsDSWE;
            dswe.append(qsDSWE.toDouble());
            stream >> qsImport;
            import.append(qsImport.toDouble());

            //Each line represents 1 model iteration, increment model iterations after each line is read
            //qDebug()<<date[nIterations]<<" "<<q[nIterations]<<" "<<dswe[nIterations]<<" "<<import[nIterations];
            nIterations++;
        }
    }
}
