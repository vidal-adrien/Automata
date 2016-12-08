#include "infodialog.h"
#include "ui_infodialog.h"
#include <QFile>
#include <QTextStream>

/**
  *
  * Custom dialog window to display html about page.
  * Associated with ui form.
 */

InfoDialog::InfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    ui->setupUi(this);

    connect(ui->okBut, SIGNAL(clicked()), this,SLOT(hide()));

    QFile file(":/html/about.html");
    if(!file.open(QIODevice::ReadOnly))
        return;
    QTextStream in(&file);
    ui->htmlField->setText(in.readAll());
    ui->htmlField->setOpenExternalLinks(true);
    file.close();
}

InfoDialog::~InfoDialog()
{
    delete ui;
}
