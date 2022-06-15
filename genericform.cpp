#include "genericform.h"
#include "ui_genericform.h"
#include "QLabel"
#include "mainwindow.h"

GenericForm::GenericForm(QJsonObject *formdata, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GenericForm)
{

    ui->setupUi(this);

    for (int i=0; i<formdata->keys().size(); i++)
    {
        if (formdata->value(formdata->keys()[i]).isObject())
        {
            QJsonObject object = formdata->value(formdata->keys()[i]).toObject();
            formdata->value(formdata->keys()[i]).toObject();
            parameter_property parameter_prop;
            parameter_prop.Discription = formdata->keys()[i];
            parameter_prop.DefaultValue = object.value("default").toString();
            if (object.contains("type"))
            {

                QLabel *label = new QLabel(parameter_prop.Discription, this);
                QString typeval = object.value("type").toString();
                if (object.value("type").toString()=="spinBox")
                {
                    parameter_prop.Type = delegate_type::SpinBox;
                    QSpinBox *spinbox = new QSpinBox(this);
                    spinbox->setValue(object.value("default").toString().toInt());
                    ui->formLayout->addRow(label,spinbox);
                    parameter_prop.InputWidget = spinbox;
                }
                if (object.value("type").toString()=="lineEdit")
                {
                    parameter_prop.Type = delegate_type::LineEdit;
                    QLineEdit *lineedit = new QLineEdit(this);
                    lineedit->setText(object.value("default").toString());
                    ui->formLayout->addRow(label,lineedit);
                    parameter_prop.InputWidget = lineedit;
                }
                if (object.value("type").toString()=="comboBox")
                {
                    parameter_prop.Type = delegate_type::ComboBox;
                    QComboBox *combobox = new QComboBox(this);
                    combobox->setCurrentText(object.value("default").toString());
                    ui->formLayout->addRow(label,combobox);
                    parameter_prop.InputWidget = combobox;
                    {
                        if (object.value("source").toString()=="TargetSamplesList")
                        {
                            vector<string> names = mainwindow()->Data()->SampleNames(mainwindow()->Data()->TargetGroup());
                            for (unsigned int i=0; i<names.size(); i++)
                                combobox->addItem(QString::fromStdString(names[i]));
                        }
                    }
                }
                if (object.value("type").toString()=="checkBox")
                {
                    parameter_prop.Type = delegate_type::CheckBox;
                    QCheckBox *checkbox = new QCheckBox(this);
                    if (object.value("default").toString().toLower()=="true")
                        checkbox->setCheckState(Qt::CheckState::Checked);
                    else
                        checkbox->setCheckState(Qt::CheckState::Unchecked);
                    ui->formLayout->addRow(label,checkbox);
                    parameter_prop.InputWidget = checkbox;
                }

                Parameter_Properties.append(parameter_prop);

            }
        }

    }

    buttonOk = new QPushButton("Ok",this);
    QIcon iconOk = QIcon(qApp->applicationDirPath()+"/../../resources/Icons/Proceed.png");
    buttonOk->setIcon(iconOk);
    ui->horizontalLayout->addWidget(buttonOk);
    connect(buttonOk,SIGNAL(clicked()),this,SLOT(onProceed()));
    buttonCancel = new QPushButton("Cancel",this);
    QIcon iconCancel = QIcon(qApp->applicationDirPath()+"/../../resources/Icons/Cancel.png");
    buttonCancel->setIcon(iconCancel);
    ui->horizontalLayout->addWidget(buttonCancel);
    connect(buttonCancel,SIGNAL(clicked()),this,SLOT(onCancel()));

}

GenericForm::~GenericForm()
{
    Parameter_Properties.clear();
    delete ui;
}

void GenericForm::onProceed()
{
    map<string,string> arguments;
    for (int i=0; i<Parameter_Properties.count(); i++)
    {
        arguments[Parameter_Properties[i].Discription.toStdString()] = Parameter_Properties[i].value().toStdString();
    }
    mainwindow()->Execute(command.toStdString(),arguments);

}
void GenericForm::onCancel()
{
    this->close();
}

MainWindow *GenericForm::mainwindow()
{
    if (parent()!=nullptr)
    return dynamic_cast<MainWindow*>(parent());
}

bool GenericForm::SetCommand(const QString &cmd)
{
    command = cmd;
    return true;
}
