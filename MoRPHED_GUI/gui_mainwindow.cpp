#include "gui_mainwindow.h"
#include "ui_gui_mainwindow.h"


GUI_MainWindow::GUI_MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GUI_MainWindow)
{
    ui->setupUi(this);

    GDALAllRegister();
}

GUI_MainWindow::~GUI_MainWindow()
{
    delete ui;
}

void GUI_MainWindow::on_btn_description_clicked()
{

}

void GUI_MainWindow::on_btn_inputs_clicked()
{
    dialog_inputs dialog(XmlGui, this);
    dialog.setModal(true);
    dialog.exec();
}

void GUI_MainWindow::on_btn_morphParam_clicked()
{
    dialog_morphParams dialog(this);
    dialog.setModal(true);
    dialog.exec();
}

void GUI_MainWindow::on_btn_delftParam_clicked()
{
    dialog_delftParams dialog(this);
    dialog.setModal(true);
    dialog.exec();
}

void GUI_MainWindow::on_btn_outputs_clicked()
{

}

void GUI_MainWindow::on_btn_run_clicked()
{
    QDateTime start, end, floodStart, floodEnd, delftStart, delftEnd, bankEStart, bankEEnd, bedEStart, bedEEnd;

    start = QDateTime::currentDateTime();

    QFileInfo file(filenameXml);

    MORPH_FileManager fm(file.absolutePath());
    MORPH_Delft3DIO *delft = new MORPH_Delft3DIO(filenameXml);
    MORPH_SedimentTransport *trans = new MORPH_SedimentTransport(filenameXml);

    for (int i=0; i<trans->getIterations(); i++)
    {

        floodStart = QDateTime::currentDateTime();
        if (i == 0)
        {
            fm.createFloodDirectories(i);
            qDebug()<<"running delft";
            delftStart = QDateTime::currentDateTime();
            delft->run();
            delftEnd = QDateTime::currentDateTime();
            qDebug()<<"DELFT TIME initial "<<delftStart.toString("hh:mm:ss")<< delftEnd.toString("hh:mm:ss")<< delftStart.secsTo(delftEnd)/60.0;
            qDebug()<<"delft done, load rasters";
            trans->loadRasters();
            qDebug()<<"rasters loaded";
        }

        fm.createFloodDirectories(i+1);
        delft->setCurrentIteration(i);
        trans->setCurrentIteration(i);

        qDebug()<<"running bank erosoin";
        bankEStart = QDateTime::currentDateTime();
        trans->runBankErode();
        bankEEnd = QDateTime::currentDateTime();
        qDebug()<<"BANK TIME "<<bankEStart.toString("hh:mm:ss")<< bankEEnd.toString("hh:mm:ss")<< bankEStart.secsTo(bankEEnd)/60.0;

        qDebug()<<"running delft "<<i;
        delftStart = QDateTime::currentDateTime();
        delft->run();
        delftEnd = QDateTime::currentDateTime();
        qDebug()<<"DELFT TIME "<<delftStart.toString("hh:mm:ss")<< delftEnd.toString("hh:mm:ss")<< delftStart.secsTo(delftEnd)/60.0;
        qDebug()<<"done running delft "<<i;

        trans->loadRasters();
        qDebug()<<"running bed erosion";
        bedEStart = QDateTime::currentDateTime();
        trans->runBedErode();
        bedEEnd = QDateTime::currentDateTime();
        qDebug()<<"BED TIME "<<bedEStart.toString("hh:mm:ss")<< bedEEnd.toString("hh:mm:ss")<< bedEStart.secsTo(bedEEnd)/60.0;
        qDebug()<<"importing";
        trans->importSediment();
        qDebug()<<"import done";
        qDebug()<<"flood done "<<i;
        floodEnd = QDateTime::currentDateTime();
        qDebug()<<"FLOOD TIME "<<floodStart.toString("hh:mm:ss")<< floodEnd.toString("hh:mm:ss")<< floodStart.secsTo(floodEnd)/60.0;
    }

    end = QDateTime::currentDateTime();
    qDebug()<<"SIM TIME "<<start.toString("hh:mm:ss")<< end.toString("hh:mm:ss")<< start.secsTo(end)/60.0;

    delete(delft);
    delete(trans);

    qDebug()<<"FINISHED!";
}

void GUI_MainWindow::on_actionNew_Project_triggered()
{
    QString filename, name;
    QStringList list;
    filename = QFileDialog::getExistingDirectory(this, "Select or Create Project Folder");

    if (filename.isNull() || filename.isEmpty())
    {
        QMessageBox::information(this, "Empty or Null Path", "The directory path is empty or null, you must select a valid directory to continue");
    }

    else
    {
        XmlGui.writeXMLdocGUI();
        XmlGui.writeNodeData("ProjectDirectory",filename);
        list = filename.split("/");
        name = list[list.size()-1];
        baseDir = filename;
        filenameXml = filename + "/" + name + ".morph";
        XmlGui.setDocumentFilename(filenameXml);
    }
}

void GUI_MainWindow::on_actionOpen_Project_triggered()
{
    QString filename;
    filename = QFileDialog::getOpenFileName(this, "Select *.morph file to open");
    filenameXml = filename;
    QFileInfo file(filename);

    if (filename.isNull() || filename.isEmpty())
    {
        QMessageBox::information(this, "Empty or Null Path", "The file path is null or empty, you must select a valid *.morph file to continue");
    }
    else
    {
        baseDir = file.absolutePath();
        XmlGui.loadDocument(filename, 1);
        XmlGui.writeNodeData("ProjectDirectory",baseDir);
    }
}
