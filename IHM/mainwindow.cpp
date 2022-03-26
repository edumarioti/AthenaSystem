#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "threadscandatabase.h"


static bool WARNING = true;
static bool NORMAL = false;

static int COLUMN_ID = 0;
static int COLUMN_PRODUCT = 1;
static int COLUMN_QUALITY = 3;
static int COLUMN_ORDER = 2;
static int COLUMN_POINT = 4;
static int COLUMN_ENTRY = 5;
static int COLUMN_EXIT = 6;


static int MANUAL_AUTO = 0;
static int ACTUAL_POSITION_X = 1;
static int ACTUAL_POSITION_Y = 2;
static int ACTUAL_POSITION_Z = 3;
static int MACHINE_STATUS = 4;
static int IMA_ACTIVE = 5;
static int UPDATE_PRODUCTS = 6;

static QString HOST = "127.0.0.1";
static QString USER = "hmi";
static QString PASSWORD = "HOSThmi2021!";
static QString DB_NAME = "AthenaSystem";

static float CONVERSION_FACTOR_BETWEEN_MM_AND_PX = 2;

static int LABEL_MANIPULATOR_ZISE = 40;

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow){

    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);
    this->showFullScreen();


    dataBase.setHostName(HOST);
    dataBase.setUserName(USER);
    dataBase.setPassword(PASSWORD);
    dataBase.setDatabaseName(DB_NAME);

    if(!dataBase.open()){
        messageInBottomFrame(WARNING, dataBase.lastError().text());
    } else{
        mThread = new threadScanDatabase(this);
        connect(mThread, SIGNAL(scanDatabase()), this, SLOT(onScanDatabase()));
        mThread->start();

        messageInBottomFrame(NORMAL, "Banco de dados aberto com sucesso!");
    }

    ui->lineEditOrderEntry->setValidator(new QIntValidator(0, 100000, this));
    ui->lineEditGotoCoordinateX->setValidator(new QIntValidator(-300, 300, this));
    ui->lineEditGotoCoordinateY->setValidator(new QIntValidator(-190, 190, this));
    ui->lineEditGotoCoordinateZ->setValidator(new QIntValidator(-100, 100, this));
    ui->lineEditManualDistance->setValidator(new QIntValidator(10, 100, this));

    ui->tableCurrentProducts->setColumnCount(6);

    QStringList header_current_products = {"ID", "Produto", "Ordem", "Qualidade", "Ponto", "Entrada"};
    ui->tableCurrentProducts->setHorizontalHeaderLabels(header_current_products);

    ui->tableCurrentProducts->setColumnWidth(COLUMN_ID, 25);
    ui->tableCurrentProducts->setColumnWidth(COLUMN_PRODUCT, 400);
    ui->tableCurrentProducts->setColumnWidth(COLUMN_QUALITY, 100);
    ui->tableCurrentProducts->setColumnWidth(COLUMN_ORDER, 150);
    ui->tableCurrentProducts->setColumnWidth(COLUMN_POINT, 80);
    ui->tableCurrentProducts->setColumnWidth(COLUMN_ENTRY, 200);


    ui->tableProductsHistory->setColumnCount(7);

    QStringList header_products_history = {"ID", "Produto", "Ordem", "Qualidade", "Ponto", "Entrada"};
    ui->tableProductsHistory->setHorizontalHeaderLabels(header_products_history);
    ui->tableProductsHistory->setColumnWidth(COLUMN_ID, 40);
    ui->tableProductsHistory->setColumnWidth(COLUMN_PRODUCT, 400);
    ui->tableProductsHistory->setColumnWidth(COLUMN_QUALITY, 100);
    ui->tableProductsHistory->setColumnWidth(COLUMN_ORDER, 150);
    ui->tableProductsHistory->setColumnWidth(COLUMN_POINT, 80);
    ui->tableProductsHistory->setColumnWidth(COLUMN_ENTRY, 200);
    ui->tableProductsHistory->setColumnWidth(COLUMN_EXIT, 200);

    updateCurrentProductsBatabase();
    updateProductsHistoryBatabase();

}


MainWindow::~MainWindow(){
    delete ui;
}

void MainWindow::onScanDatabase(){

    if (dataBase.isOpen()){

        QSqlQuery query;

        query.prepare("SELECT * FROM system;");

        if (query.exec()){

            while (query.next()) {

                changeStateManualAuto(query.value(MANUAL_AUTO).toBool());

                ui->lcdPositionX->display(query.value(ACTUAL_POSITION_X).toInt());
                ui->lcdPositionY->display(query.value(ACTUAL_POSITION_Y).toInt());
                ui->lcdPositionZ->display(query.value(ACTUAL_POSITION_Z).toInt());


                positionXInPx = query.value(ACTUAL_POSITION_X).toFloat() * CONVERSION_FACTOR_BETWEEN_MM_AND_PX;
                positionYInPx = query.value(ACTUAL_POSITION_Y).toFloat() * CONVERSION_FACTOR_BETWEEN_MM_AND_PX;

                QSize sizeLabel = ui->labelPointManipulatorImaAtive->size();
                QSize sizeFrame = ui->framePoints->size();

                int intPositionXInPx = sizeFrame.rwidth() - (int(positionXInPx) + sizeLabel.rwidth());
                int intPositionYInPx = sizeFrame.rheight() - (int(positionYInPx) + sizeLabel.rheight());

                ui->labelPointManipulatorImaAtive->setGeometry(intPositionXInPx, intPositionYInPx, LABEL_MANIPULATOR_ZISE, LABEL_MANIPULATOR_ZISE);
                ui->labelPointManipulatorImaInative->setGeometry(intPositionXInPx, intPositionYInPx, LABEL_MANIPULATOR_ZISE, LABEL_MANIPULATOR_ZISE);

                ui->labelPointManipulatorImaAtive->setHidden(!query.value(IMA_ACTIVE).toBool());
                ui->labelPointManipulatorImaInative->setHidden(query.value(IMA_ACTIVE).toBool());

                int machineState =  query.value(MACHINE_STATUS).toInt();

                switch (machineState) {

                    case 0:
                        ui->labelMachineReady->setHidden(true);
                        ui->labelMachineRun->setHidden(true);
                        ui->labelMachineAlarm->setHidden(true);
                        ui->labelMachineUndefined->setHidden(false);
                    break;

                    case 1:
                        ui->labelMachineReady->setHidden(false);
                        ui->labelMachineRun->setHidden(true);
                        ui->labelMachineAlarm->setHidden(true);
                        ui->labelMachineUndefined->setHidden(true);
                    break;

                    case 2:
                        ui->labelMachineReady->setHidden(true);
                        ui->labelMachineRun->setHidden(false);
                        ui->labelMachineAlarm->setHidden(true);
                        ui->labelMachineUndefined->setHidden(true);
                    break;

                    case 3:
                        ui->labelMachineReady->setHidden(true);
                        ui->labelMachineRun->setHidden(true);
                        ui->labelMachineAlarm->setHidden(false);
                        ui->labelMachineUndefined->setHidden(true);
                    break;

                }

                if (query.value(UPDATE_PRODUCTS).toBool()){
                    updateCurrentProductsBatabase();
                    updateProductsHistoryBatabase();
                    SQLInsertComandFromHMI("currentProductsUpdated", "True");

                }
            }
        }
    }
}

void MainWindow::updateCurrentProductsBatabase(){

    QSqlQuery query;

    query.prepare("SELECT * FROM `current_products`");

    if (query.exec()){

        int line = 0;

        ui->tableCurrentProducts->clear();
        ui->tableCurrentProducts->setRowCount(0);

        QStringList header_current_products = {"ID", "Produto", "Ordem", "Qualidade", "Ponto", "Entrada"};
        ui->tableCurrentProducts->setHorizontalHeaderLabels(header_current_products);

        ui->comboBoxIDExit->clear();
        ui->comboBoxOrderExit->clear();

        QStringList points = {};

        while (query.next()) {

            ui->tableCurrentProducts->insertRow(line);

            QString id = query.value(COLUMN_ID).toString();
            ui->tableCurrentProducts->setItem(line, COLUMN_ID, new QTableWidgetItem(id));
            ui->tableCurrentProducts->item(line,COLUMN_ID)->setTextAlignment(Qt::AlignCenter);
            ui->comboBoxIDExit->addItem(id);

            QString product = query.value(COLUMN_PRODUCT).toString();
            ui->tableCurrentProducts->setItem(line, COLUMN_PRODUCT, new QTableWidgetItem(product));

            QString quality = query.value(COLUMN_QUALITY).toString();
            ui->tableCurrentProducts->setItem(line, COLUMN_QUALITY, new QTableWidgetItem(quality));
            ui->tableCurrentProducts->item(line,COLUMN_QUALITY)->setTextAlignment(Qt::AlignCenter);

            QString order = query.value(COLUMN_ORDER).toString();
            ui->tableCurrentProducts->setItem(line, COLUMN_ORDER, new QTableWidgetItem(order));
            ui->tableCurrentProducts->item(line,COLUMN_ORDER)->setTextAlignment(Qt::AlignCenter);

            int index = ui->comboBoxOrderExit->findText(order);

            if (index < 0){
                ui->comboBoxOrderExit->addItem(order);
            }

            QString point = query.value(COLUMN_POINT).toString();
            ui->tableCurrentProducts->setItem(line, COLUMN_POINT, new QTableWidgetItem(point));
            ui->tableCurrentProducts->item(line,COLUMN_POINT)->setTextAlignment(Qt::AlignCenter);
            points.append(point);


            QString entry = query.value(COLUMN_ENTRY).toString();
            ui->tableCurrentProducts->setItem(line, COLUMN_ENTRY, new QTableWidgetItem(entry));


            line++;
        }

        updateStatePoints(points);
    }
}

void MainWindow::updateProductsHistoryBatabase(){

    QSqlQuery query;

    query.prepare("SELECT * FROM `products_history`");

    if (query.exec()){

        int line = 0;

        ui->tableProductsHistory->clear();
        ui->tableProductsHistory->setRowCount(0);

        QStringList header_current_products = {"ID", "Produto", "Ordem", "Qualidade", "Ponto", "Entrada", "Saída"};
        ui->tableProductsHistory->setHorizontalHeaderLabels(header_current_products);

        QStringList points = {};

        while (query.next()) {

            ui->tableProductsHistory->insertRow(line);

            QString id = query.value(COLUMN_ID).toString();
            ui->tableProductsHistory->setItem(line, COLUMN_ID, new QTableWidgetItem(id));
            ui->tableProductsHistory->item(line,COLUMN_ID)->setTextAlignment(Qt::AlignCenter);

            QString product = query.value(COLUMN_PRODUCT).toString();
            ui->tableProductsHistory->setItem(line, COLUMN_PRODUCT, new QTableWidgetItem(product));

            QString quality = query.value(COLUMN_QUALITY).toString();
            ui->tableProductsHistory->setItem(line, COLUMN_QUALITY, new QTableWidgetItem(quality));
            ui->tableProductsHistory->item(line,COLUMN_QUALITY)->setTextAlignment(Qt::AlignCenter);


            QString order = query.value(COLUMN_ORDER).toString();
            ui->tableProductsHistory->setItem(line, COLUMN_ORDER, new QTableWidgetItem(order));
            ui->tableProductsHistory->item(line,COLUMN_ORDER)->setTextAlignment(Qt::AlignCenter);

            QString point = query.value(COLUMN_POINT).toString();
            ui->tableProductsHistory->setItem(line, COLUMN_POINT, new QTableWidgetItem(point));
            ui->tableProductsHistory->item(line,COLUMN_POINT)->setTextAlignment(Qt::AlignCenter);

            QString entry = query.value(COLUMN_ENTRY).toString();
            ui->tableProductsHistory->setItem(line, COLUMN_ENTRY, new QTableWidgetItem(entry));

            QString exit = query.value(COLUMN_EXIT).toString();
            ui->tableProductsHistory->setItem(line, COLUMN_EXIT, new QTableWidgetItem(exit));

            line++;
        }
    }
}

void MainWindow::on_buttonCloseWindow_clicked(){
    this->close();
}

void MainWindow::on_buttonMinimizeWindow_clicked(){
    this->showMinimized();
}

void MainWindow::changeStateManualAuto(bool newState){

    ui->buttonSystemInAuto->setChecked(newState);
    ui->buttonSystemInAuto_2->setChecked(newState);

    ui->buttonSystemInManual->setChecked(!newState);
    ui->buttonSystemInManual_2->setChecked(!newState);

    ui->labelSystemInAuto->setVisible(newState);
    ui->labelSystemInAuto_2->setVisible(newState);
    ui->labelSystemInAuto_3->setVisible(newState);

    ui->labelSystemInManual->setVisible(!newState);
    ui->labelSystemInManual_2->setVisible(!newState);
    ui->labelSystemInManual_3->setVisible(!newState);

    ui->groupBoxEntryProduct->setEnabled(newState);
    ui->groupBoxExitProduct->setEnabled(newState);

    ui->groupControlManual->setEnabled(!newState);
    ui->groupGoToCoordinate->setEnabled(!newState);
    ui->groupGoToPosition->setEnabled(!newState);

}

void MainWindow::SQLInsertComandFromHMI(QString comand, QString value){

    if (!dataBase.isOpen()){
        QMessageBox::warning(this, "Atenção!", "Sem conexão com o banco de dados!");
    }
    QSqlQuery query;

    query.prepare("INSERT INTO `hmi_commands` (command, value) values (\'"+comand+"\', \'"+value+"\');");
    if (!query.exec()){

        QMessageBox::warning(this, "Atenção!", query.lastError().text());
    }

}

void MainWindow::messageInBottomFrame(bool isWarning, QString message){

    ui->labelInfoMessage->setText(message);

    if (isWarning){
        ui->labelInfoMessage->setStyleSheet("color: #EF3535");
    } else {
        ui->labelInfoMessage->setStyleSheet("color: #DCDCDC");
    }
}

void MainWindow::on_buttonSystemInManual_clicked(){
    SQLInsertComandFromHMI("manualAuto", "False");
}

void MainWindow::on_buttonSystemInManual_2_clicked(){
    SQLInsertComandFromHMI( "manualAuto", "False");
}

void MainWindow::on_buttonSystemInAuto_2_clicked(){
    SQLInsertComandFromHMI("manualAuto", "True");
}

void MainWindow::on_buttonSystemInAuto_clicked(){
    SQLInsertComandFromHMI("manualAuto", "True");
}

void MainWindow::on_buttonUpManual_pressed(){
    SQLInsertComandFromHMI("manualDirectionMovement", "1");
}

void MainWindow::on_buttonUpManual_released(){
    SQLInsertComandFromHMI("manualDirectionMovement", "0");
}

void MainWindow::on_buttonUpRightManual_pressed(){
    SQLInsertComandFromHMI("manualDirectionMovement", "2");
}

void MainWindow::on_buttonUpRightManual_released(){
    SQLInsertComandFromHMI("manualDirectionMovement", "0");
}

void MainWindow::on_buttonRightManual_pressed(){
    SQLInsertComandFromHMI("manualDirectionMovement", "3");
}

void MainWindow::on_buttonRightManual_released(){
    SQLInsertComandFromHMI("manualDirectionMovement", "0");
}

void MainWindow::on_buttonDownRightManual_pressed(){
    SQLInsertComandFromHMI("manualDirectionMovement", "4");
}

void MainWindow::on_buttonDownRightManual_released(){
    SQLInsertComandFromHMI("manualDirectionMovement", "0");
}

void MainWindow::on_buttonDownManual_pressed(){
    SQLInsertComandFromHMI("manualDirectionMovement", "5");
}

void MainWindow::on_buttonDownManual_released(){
    SQLInsertComandFromHMI("manualDirectionMovement", "0");
}

void MainWindow::on_buttonDownLeftManual_pressed(){
    SQLInsertComandFromHMI("manualDirectionMovement", "6");
}

void MainWindow::on_buttonDownLeftManual_released(){
    SQLInsertComandFromHMI("manualDirectionMovement", "0");
}

void MainWindow::on_buttonLeftManual_pressed(){
    SQLInsertComandFromHMI("manualDirectionMovement", "7");
}

void MainWindow::on_buttonLeftManual_released(){
    SQLInsertComandFromHMI("manualDirectionMovement", "0");
}

void MainWindow::on_buttonUpLeftManual_pressed(){
    SQLInsertComandFromHMI("manualDirectionMovement", "8");
}

void MainWindow::on_buttonUpLeftManual_released(){
    SQLInsertComandFromHMI("manualDirectionMovement", "0");
}

void MainWindow::on_buttonIncrementCoordinate_clicked(){

    QString x = ui->lineEditGotoCoordinateX->text();
    QString y = ui->lineEditGotoCoordinateY->text();
    QString z = ui->lineEditGotoCoordinateZ->text();

    SQLInsertComandFromHMI("incrementCoordinate", "("+x+","+y+","+z+")");

    ui->lineEditGotoCoordinateX->setText("0");
    ui->lineEditGotoCoordinateY->setText("0");
    ui->lineEditGotoCoordinateZ->setText("0");
}

void MainWindow::on_buttonGoToPosition_clicked(){
    QString position = ui->lineEditGoToPosition->text();

    if (position != ""){
        SQLInsertComandFromHMI("goToPosition", "\""+position+"\"");
        messageInBottomFrame(NORMAL, "Comando enviado!");
    } else {
        messageInBottomFrame(WARNING, "Verifique os campos informados.");
    }
}

void MainWindow::on_buttonConfirmEntry_clicked(){

    QString product = ui->lineEditProductEntry->text();
    QString order = ui->lineEditOrderEntry->text();
    QString quality = ui->comboBoxQualityEntry->currentText();

    //QString value =
    if (product != "" and order != ""){
        SQLInsertComandFromHMI("buttonConfirmEntry", "(\""+product+"\","+order+", \""+quality+"\")");
        messageInBottomFrame(NORMAL, "Comando enviado!");
    } else {
        messageInBottomFrame(WARNING, "Verifique os campos informados.");
    }
}

void MainWindow::on_buttonConfirmManualDistance_clicked()
{
    QString milimeters = ui->lineEditManualDistance->text();
    if (milimeters != "" ){
        SQLInsertComandFromHMI("manualMovementDistance", milimeters);
        messageInBottomFrame(NORMAL, "Comando enviado!");
    } else {
        messageInBottomFrame(WARNING, "Verifique os campos informados.");
    }
}


void MainWindow::on_buttonPickUpAndDrop_clicked(bool checked)
{
    QString value = "False";

    if (checked){
        value = "True";
    }

    SQLInsertComandFromHMI("pickUpAndDrop", value);
}

void MainWindow::on_buttonUpZManual_clicked(){
    SQLInsertComandFromHMI("EixoZManual", "True");
}

void MainWindow::on_buttonDownZManual_clicked(){
    SQLInsertComandFromHMI("EixoZManual", "False");
}

void MainWindow::on_buttonHomePosition_clicked(){
    SQLInsertComandFromHMI("goToHome", "True");
}

void MainWindow::on_buttonResetMachine_clicked(){
    SQLInsertComandFromHMI("resetMachine", "True");
}

void MainWindow::on_buttonConfirmIDExit_clicked(){

    if (ui->comboBoxIDExit->currentIndex() >= 0){
        SQLInsertComandFromHMI("exitProductID", ui->comboBoxIDExit->currentText());
        messageInBottomFrame(NORMAL, "Comando enviado!");
    } else {
        messageInBottomFrame(WARNING, "Verifique os campos informados.");
    }
}

void MainWindow::on_buttonConfirmOrderExit_clicked(){
    if (ui->comboBoxOrderExit->currentIndex() >= 0){
        SQLInsertComandFromHMI("exitProductOrder", ui->comboBoxOrderExit->currentText());
        messageInBottomFrame(NORMAL, "Comando enviado!");
    } else {
        messageInBottomFrame(WARNING, "Verifique os campos informados.");
    }
}

void MainWindow::updateStatePoints(QStringList points){
    QString styleSheet = "border:2px solid #71ECCF;";


    if (points.indexOf("B") >= 0){
        ui->labelPointB->setStyleSheet(styleSheet);
    } else {
        ui->labelPointB->setStyleSheet("");
    }

    if (points.indexOf("C") >= 0){
        ui->labelPointC->setStyleSheet(styleSheet);
    } else {
        ui->labelPointC->setStyleSheet("");
    }

    if (points.indexOf("D") >= 0){
        ui->labelPointD->setStyleSheet(styleSheet);
    } else {
        ui->labelPointD->setStyleSheet("");
    }

    if (points.indexOf("E") >= 0){
        ui->labelPointE->setStyleSheet(styleSheet);
    } else {
        ui->labelPointE->setStyleSheet("");
    }

    if (points.indexOf("F") >= 0){
        ui->labelPointF->setStyleSheet(styleSheet);
    } else {
        ui->labelPointF->setStyleSheet("");
    }

    if (points.indexOf("G") >= 0){
        ui->labelPointG->setStyleSheet(styleSheet);
    } else {
        ui->labelPointG->setStyleSheet("");
    }

    if (points.indexOf("H") >= 0){
        ui->labelPointH->setStyleSheet(styleSheet);
    } else {
        ui->labelPointH->setStyleSheet("");
    }

    if (points.indexOf("I") >= 0){
        ui->labelPointI->setStyleSheet(styleSheet);
    } else {
        ui->labelPointI->setStyleSheet("");
    }

    if (points.indexOf("J") >= 0){
        ui->labelPointJ->setStyleSheet(styleSheet);
    } else {
        ui->labelPointJ->setStyleSheet("");
    }

    if (points.indexOf("K") >= 0){
        ui->labelPointK->setStyleSheet(styleSheet);
    } else {
        ui->labelPointK->setStyleSheet("");
    }

    if (points.indexOf("L") >= 0){
        ui->labelPointL->setStyleSheet(styleSheet);
    } else {
        ui->labelPointL->setStyleSheet("");
    }

    if (points.indexOf("M") >= 0){
        ui->labelPointM->setStyleSheet(styleSheet);
    } else {
        ui->labelPointM->setStyleSheet("");
    }

    if (points.indexOf("N") >= 0){
        ui->labelPointN->setStyleSheet(styleSheet);
    } else {
        ui->labelPointN->setStyleSheet("");
    }

    if (points.indexOf("O") >= 0){
        ui->labelPointO->setStyleSheet(styleSheet);
    } else {
        ui->labelPointO->setStyleSheet("");
    }

    if (points.indexOf("P") >= 0){
        ui->labelPointP->setStyleSheet(styleSheet);
    } else {
        ui->labelPointP->setStyleSheet("");
    }

    if (points.indexOf("Q") >= 0){
        ui->labelPointQ->setStyleSheet(styleSheet);
    } else {
        ui->labelPointQ->setStyleSheet("");
    }

    if (points.indexOf("R") >= 0){
        ui->labelPointR->setStyleSheet(styleSheet);
    } else {
        ui->labelPointR->setStyleSheet("");
    }

    if (points.indexOf("S") >= 0){
        ui->labelPointS->setStyleSheet(styleSheet);
    } else {
        ui->labelPointS->setStyleSheet("");
    }

    if (points.indexOf("T") >= 0){
        ui->labelPointT->setStyleSheet(styleSheet);
    } else {
        ui->labelPointT->setStyleSheet("");
    }

    if (points.indexOf("U") >= 0){
        ui->labelPointU->setStyleSheet(styleSheet);
    } else {
        ui->labelPointU->setStyleSheet("");
    }
}
