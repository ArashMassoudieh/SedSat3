#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QDebug"
#include "QFileDialog"
#include "QDir"
#include "sourcesinkdata.h"
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
#include "indicatesheetsdialog.h"
#include "QMessageBox"
using namespace QXlsx;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionImport_Elemental_Profile_from_Excel,SIGNAL(triggered()),this,SLOT(on_import_excel()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_import_excel()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open"), "",
            tr("Excel files (*.xlsx);; All files (*.*)"),nullptr,QFileDialog::DontUseNativeDialog);


    if (fileName!="")
    {
        ReadExcel(fileName);
    }

}

bool MainWindow::ReadExcel(const QString &filename)
{
    Document xlsxR(filename);
    QStringList sheetnames;
    if ( xlsxR.load() ) // load excel file
    {
        qDebug() << "[debug] success to load xlsx file.";
        sheetnames = xlsxR.sheetNames();
    }

    for (int i=0; i<sheetnames.count(); i++)
    {
        qDebug() << sheetnames[i];
    }

    IndicateSheetsDialog *indicatesheetdialog = new IndicateSheetsDialog(this);
    indicatesheetdialog->Populate_Table(sheetnames);
    indicatesheetdialog->exec();
    qDebug()<<"Sink Sheet is: " << Sink_Sheet;

    QList<QStringList> element_names;
    for (int sheetnumber=0; sheetnumber<sheetnames.count(); sheetnumber++)
    {
        xlsxR.selectSheet(sheetnames[sheetnumber]);
        QStringList elem_names;
        QString elem="none";
        int col = 2;
        while (elem!="" && xlsxR.cellAt(1,col))
        {
            elem = xlsxR.cellAt(1,col)->readValue().toString();
            if (elem!="")
                elem_names.append(elem);
            qDebug()<<elem;
            col++;
        }
        if (sheetnumber>0)
        {   if (elem_names!=element_names.last())
            {
                QMessageBox::warning(this, "CMBSource", tr("Element names and their order in all sheets must be identical\n"), QMessageBox::Ok);
                WriteMessageOnScreen("Name of elements in sheet '" + sheetnames[sheetnumber-1] + "' and '" + sheetnames[sheetnumber] + "' are different",Qt::red);
                return false;
            }
        }
        element_names.append(elem_names);

    }


    /*bool endoftable = false;
        int rownum = 8;
        while (!endoftable)
        {
            _row Row;
            Row.CourseCode = xlsxR.cellAt(rownum,2)->readValue().toString() + " " + xlsxR.cellAt(rownum,3)->readValue().toString();
            Row.CourseName = xlsxR.cellAt(rownum,5)->readValue().toString();
            Row.Instructor = xlsxR.cellAt(rownum,9)->readValue().toString();
            Row.section = xlsxR.cellAt(rownum,4)->readValue().toString();
            QString solefilename = filename.split("/")[filename.split("/").count()-1];
            Row.Semester = solefilename.split(" ")[0] + solefilename.split(" ")[1];
            Row.EvaluationScore = xlsxR.cellAt(rownum,31)->readValue().toDouble();
            qDebug()<< Row.CourseCode << ", " << Row.CourseName << ", " << Row.Instructor << ", " << Row.EvaluationScore << ", " << Row.Semester;
            data.append(Row);
            rownum++;
            if (!xlsxR.cellAt(rownum,2))
                endoftable = true;
            else if (xlsxR.cellAt(rownum,2)->readValue().toString()=="")
                endoftable = true;

        }
    }
    else
    {
        qDebug() << "[debug][error] failed to load xlsx file.";
    }
    return data;*/
    return true;
}

void MainWindow::WriteMessageOnScreen(const QString &text, QColor color)
{
    ui->textBrowser->setTextColor(color);
    ui->textBrowser->append(text);
}
