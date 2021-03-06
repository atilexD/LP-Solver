#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDesktopWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_removeIcon.addFile(tr(":/images/remove.png"));

    QLabel *r1Label = new QLabel("R 1 :", ui->gridLayoutWidget);
    r1Label->setObjectName(tr("vRLabel(1)"));
    ui->gridLayout->addWidget(r1Label, 2, 0, 1, 1);
    m_vRLabel.append(r1Label);

    QLineEdit *r1LineEdit = new QLineEdit(ui->gridLayoutWidget);
    r1LineEdit->setObjectName(tr("vRLineEdit(1)"));
    r1LineEdit->setPlaceholderText(tr("a + b <= 50"));
    ui->gridLayout->replaceWidget(ui->addRestPushButton, r1LineEdit);
    m_vRLineEdit.append(r1LineEdit);

    QPushButton *r1PushButton = new QPushButton(ui->gridLayoutWidget);
    r1PushButton->setObjectName(QStringLiteral("vRPushButton(1)"));
    r1PushButton->setMaximumSize(QSize(16, 16));
    r1PushButton->setIcon(m_removeIcon);
    r1PushButton->setIconSize(QSize(16, 16));
    r1PushButton->setFlat(true);
    connect(r1PushButton, SIGNAL(clicked()), this, SLOT(removeRest()));

    ui->gridLayout->addWidget(r1PushButton, 2, 2, 1, 1);
    m_vRPushButton.append(r1PushButton);

    ui->gridLayout->addWidget(ui->addRestPushButton, 3, 1, 1, 1);

    poblacion = new Poblacion();
    poblacion->setGenerations(ui->generacionLineEdit->text().toUInt());
    poblacion->setSize(ui->poblacionLineEdit->text().toUInt());
    poblacion->setProbSele(0.85);
    poblacion->setProbMuta(0.1);
    poblacion->setPrecision(ui->precisionLineEdit->text().toUInt());
    poblacion->setTipoSeleccion(TipoSeleccion::TORNEO);

    this->setFixedSize(450, 450);
    QDesktopWidget *dsk = QApplication::desktop();
    QRect rec = dsk->screenGeometry(dsk->screenNumber(this));

    this->move(rec.width()/2  - 225, 50);

    // Debug
    ui->zLineEdit->setText("0.1A + 0.07B");
    m_vRLineEdit.at(0)->setText("A + B <= 500");
    ui->addRestPushButton->click();
    m_vRLineEdit.at(1)->setText("A <= 300");
    ui->addRestPushButton->click();
    m_vRLineEdit.at(2)->setText("B >= 100");
    ui->addRestPushButton->click();
    m_vRLineEdit.at(3)->setText("A >= B");
/*
    ui->zLineEdit->setText("2a + b");
    m_vRLineEdit.at(0)->setText("40 a + 30b <= 600");
    ui->addRestPushButton->click();
    m_vRLineEdit.at(1)->setText("a >= 3");
    ui->addRestPushButton->click();
    m_vRLineEdit.at(2)->setText("2b >= a");
    */
}

MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::on_addRestPushButton_clicked(){
    int row = m_vRLabel.size() + 2;

    if( row >= 12 )
        return;

    // Add new row
    QLabel *rLabel = new QLabel(tr("R %1 :").arg(row-1), ui->gridLayoutWidget);
    rLabel->setObjectName(tr("vRLabel(%1)").arg(row-1));
    ui->gridLayout->addWidget(rLabel, row, 0, 1, 1);
    m_vRLabel.append(rLabel);

    QLineEdit *rLineEdit = new QLineEdit(ui->gridLayoutWidget);
    rLineEdit->setObjectName(tr("vRLineEdit(%1)").arg(row-1));
    rLineEdit->setPlaceholderText(tr("a + b <= 50"));
    ui->gridLayout->replaceWidget(ui->addRestPushButton, rLineEdit);
    m_vRLineEdit.append(rLineEdit);

    QPushButton *rPushButton = new QPushButton(ui->gridLayoutWidget);
    rPushButton->setObjectName(tr("vRPushButton(%1)").arg(row-1));
    rPushButton->setMaximumSize(QSize(16, 16));
    rPushButton->setIcon(m_removeIcon);
    rPushButton->setIconSize(QSize(16, 16));
    rPushButton->setFlat(true);
    connect(rPushButton, SIGNAL(clicked()), this, SLOT(removeRest()));

    ui->gridLayout->addWidget(rPushButton, row, 2, 1, 1);
    m_vRPushButton.append(rPushButton);

    ui->gridLayout->addWidget(ui->addRestPushButton, row + 1, 1, 1, 1);

    // Resize window, move everything else
    int offset = 29;
    this->setFixedHeight(this->height() + offset);
    ui->runPushButton->move(ui->runPushButton->x(), ui->runPushButton->y() + offset);
    ui->geneticGroupBox->move(ui->geneticGroupBox->x(), ui->geneticGroupBox->y() + offset);
    ui->PLGroupBox->resize(ui->PLGroupBox->width(), ui->PLGroupBox->height() + offset);
    ui->gridLayoutWidget->resize(ui->gridLayoutWidget->width(), ui->gridLayoutWidget->height() + offset);
}

void MainWindow::on_runPushButton_clicked(){
    // check Z Function
    // "  0.4 a  + -1.2 b -4c + d - 10e  "
    QString str = ui->zLineEdit->text();
    QRegExp reNum("-?(?:\\d*\\.)?\\d+");
    QRegExp reVar("[a-y]", Qt::CaseInsensitive);
    QRegExp reTerm("(?:(?:(?:" + reNum.pattern() + " *)?" + reVar.pattern() + ")|"
                   "(?:" + reNum.pattern() + "))",
                   Qt::CaseInsensitive);
    QRegExp reSumRes("[+\\-]");
    QRegExp rePoly(reTerm.pattern() + " *"
                   "(?:" + reSumRes.pattern() + " *" +
                   reTerm.pattern() + " *)*",
                   Qt::CaseInsensitive);

    QRegExp reZ("^ *" + rePoly.pattern() + " *$",
                Qt::CaseInsensitive);

    if( !str.contains(reZ) ){
        qWarning() << "Error en la funcion Z.";
        return;
    }

    QStringList Z;
    QString variable = "";
    int pos = reTerm.indexIn(str);
    QString sub = reTerm.cap();
    int posV = reVar.indexIn(sub);
    if( posV != -1 && !variable.contains(reVar.cap()) )
        variable += reVar.cap();
    Z << sub;
    pos += reTerm.matchedLength();
    while( (pos = reSumRes.indexIn(str, pos)) != -1 ){
        Z << reSumRes.cap();
        pos += reSumRes.matchedLength();
        pos = reTerm.indexIn(str, pos);
        sub = reTerm.cap();
        posV = reVar.indexIn(sub);
        if( posV != -1 && !variable.contains(reVar.cap()) )
            variable += reVar.cap();
        Z << sub;
        pos += reTerm.matchedLength();
    }
    //qInfo() << "Z:" << Z;
    qInfo() << "Variables:" << variable;
    poblacion->setZ(Z);

    // Check restriction
    //
    QRegExp reDesig("<|<=|>|>=|=");
    QRegExp reRestr("^ *(" + rePoly.pattern() + ") *"
                    "(" + reDesig.pattern() + ") *"
                    "(" + rePoly.pattern() + ") *$",
                    Qt::CaseInsensitive);

    QVector<QStringList> R;

    for(int i=0;i<m_vRLineEdit.size();i++){
        str = m_vRLineEdit.at(i)->text();

        if( reRestr.indexIn(str) < 0 ){
            qWarning() << "Error en la restriccion" << (i+1) << ".";
            return;
        }

        QStringList cap = reRestr.capturedTexts();

        R.append(QStringList());
        pos = reTerm.indexIn(cap.at(1));
        sub = reTerm.cap();
        posV = reVar.indexIn(sub);
        if( posV != -1 && !variable.contains(reVar.cap()) ){
            qWarning() << "Error en la restriccion" << (i+1) << ".";
            qWarning() << "Variable" << reVar.cap() << "no presente en Z.";
            return;
        }
        R[i] << sub;
        pos += reTerm.matchedLength();
        while( (pos = reSumRes.indexIn(cap.at(1), pos)) != -1 ){
            R[i] << reSumRes.cap();
            pos += reSumRes.matchedLength();
            pos = reTerm.indexIn(cap.at(1), pos);
            sub = reTerm.cap();
            posV = reVar.indexIn(sub);
            if( posV != -1 && !variable.contains(reVar.cap()) ){
                qWarning() << "Error en la restriccion" << (i+1) << ".";
                qWarning() << "Variable" << reVar.cap() << "no presente en Z.";
                return;
            }
            R[i] << sub;
            pos += reTerm.matchedLength();
        }

        R[i] << cap.at(2);

        pos = reTerm.indexIn(cap.at(3));
        sub = reTerm.cap();
        posV = reVar.indexIn(sub);
        if( posV != -1 && !variable.contains(reVar.cap()) ){
            qWarning() << "Error en la restriccion" << (i+1) << ".";
            qWarning() << "Variable" << reVar.cap() << "no presente en Z.";
            return;
        }
        R[i] << sub;
        pos += reTerm.matchedLength();
        while( (pos = reSumRes.indexIn(cap.at(3), pos)) != -1 ){
            R[i] << reSumRes.cap();
            pos += reSumRes.matchedLength();
            pos = reTerm.indexIn(cap.at(3), pos);
            sub = reTerm.cap();
            posV = reVar.indexIn(sub);
            if( posV != -1 && !variable.contains(reVar.cap()) ){
                qWarning() << "Error en la restriccion" << (i+1) << ".";
                qWarning() << "Variable" << reVar.cap() << "no presente en Z.";
                return;
            }
            R[i] << sub;
            pos += reTerm.matchedLength();
        }

        //qInfo() << "Restriccion" << (i+1) << ":" << R[i];
    }

    poblacion->setR(R);
    if( ui->maxRadioButton->isChecked() )
        poblacion->setMaximizar(true);
    else if( ui->minRadioButton->isChecked() )
        poblacion->setMaximizar(false);

    VarLimDialog dial(variable, this);
    if( dial.exec() == QDialog::Rejected )
        return;

    poblacion->setVariables(variable);
    poblacion->setLimits(dial.getData());
    poblacion->evolve();

    if( ui->maxRadioButton->isChecked() )
        qInfo() << "Maximo Z:" << poblacion->getMaximo();
    else if( ui->minRadioButton->isChecked() )
        qInfo() << "Minimo Z:" << poblacion->getMinimo();
}

void MainWindow::on_generacionLineEdit_editingFinished(){
    int pos = 0;
    QString str = ui->generacionLineEdit->text();
    QIntValidator *v = new QIntValidator(this);
    v->setBottom(0);

    if( v->validate(str, pos) != QValidator::Acceptable ){
        ui->generacionLineEdit->setText(QString::number(poblacion->getGenerations()));
        return;
    }
    if( poblacion->getGenerations() == str.toUInt() )
        return;

    poblacion->setGenerations(str.toUInt());
}

void MainWindow::on_poblacionLineEdit_editingFinished(){
    int pos = 0;
    QString str = ui->poblacionLineEdit->text();
    QIntValidator *v = new QIntValidator(this);
    v->setBottom(2);

    if( v->validate(str, pos) != QValidator::Acceptable ){
        ui->poblacionLineEdit->setText(QString::number(poblacion->getSize()));
        return;
    }
    if( str.toUInt()%2 != 0 ){
        ui->poblacionLineEdit->setText(QString::number(poblacion->getSize()));
        return;
    }
    if( poblacion->getSize() == str.toUInt() )
        return;

    poblacion->setSize(str.toUInt());
}

void MainWindow::on_precisionLineEdit_editingFinished(){
    int pos = 0;
    QString str = ui->precisionLineEdit->text();
    QIntValidator *v = new QIntValidator(0, 15, this);

    if( v->validate(str, pos) != QValidator::Acceptable ){
        ui->precisionLineEdit->setText(QString::number(poblacion->getPrecision()));
        return;
    }
    if( poblacion->getPrecision() == str.toUInt() )
        return;

    poblacion->setPrecision(str.toUInt());
}

void MainWindow::removeRest(){
    if( m_vRLabel.size() == 1 )
        return;

    int num = sender()->objectName().midRef(13, 1).toInt();
    num = (sender()->objectName().length() == 16 ? 9 : num-1);

    // Remove widget from the layout
    ui->gridLayout->removeWidget(m_vRLabel.at(num));
    ui->gridLayout->removeWidget(m_vRLineEdit.at(num));
    ui->gridLayout->removeWidget(m_vRPushButton.at(num));

    // Move up next widgets one row
    for(int i=num+1;i<m_vRLabel.size();i++){
        ui->gridLayout->addWidget(m_vRLabel.at(i), i+1, 0, 1, 1);
        m_vRLabel.at(i)->setText(tr("R %1 :").arg(i));
        m_vRLabel.at(i)->setObjectName(tr("vRLabel(%1)").arg(i));
        ui->gridLayout->addWidget(m_vRLineEdit.at(i), i+1, 1, 1, 1);
        m_vRLineEdit.at(i)->setObjectName(tr("vRLineEdit(%1)").arg(i));
        ui->gridLayout->addWidget(m_vRPushButton.at(i), i+1, 2, 1, 1);
        m_vRPushButton.at(i)->setObjectName(tr("vRPushButton(%1)").arg(i));
    }

    // Move up the add restriction button
    ui->gridLayout->addWidget(ui->addRestPushButton, m_vRLabel.size() + 1, 1, 1, 1);

    // Delete the 3 widgets
    m_vRLabel.at(num)->deleteLater();
    m_vRLineEdit.at(num)->deleteLater();
    m_vRPushButton.at(num)->deleteLater();

    // Remove from the QVector
    m_vRLabel.removeAt(num);
    m_vRLineEdit.removeAt(num);
    m_vRPushButton.removeAt(num);

    // Resize window, move everything else
    int offset = 29;
    ui->gridLayoutWidget->resize(ui->gridLayoutWidget->width(), ui->gridLayoutWidget->height() - offset);
    ui->PLGroupBox->resize(ui->PLGroupBox->width(), ui->PLGroupBox->height() - offset);
    ui->geneticGroupBox->move(ui->geneticGroupBox->x(), ui->geneticGroupBox->y() - offset);
    ui->runPushButton->move(ui->runPushButton->x(), ui->runPushButton->y() - offset);
    this->setFixedHeight(this->height() - offset);
}
