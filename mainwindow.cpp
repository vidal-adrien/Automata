#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qmath.h>
#include <QColor>
#include <QColorDialog>
#include <QTextStream>
#include <QFileDialog>
#include <QChar>
#include <QList>
#include <QValidator>
#include <QInputDialog>
#include <QStandardItemModel>

#include <QDebug>

//=====================================================================================~~Constructor~~=====================================================================================
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    state(false),
    ui(new Ui::MainWindow),
    currentColor(QColor("#000")), //Cells are black by default.
    game(new GameWidget(this)), //Create custom widget instance.
    reg8(QRegExp("[0-8]{0,9}")), //For 8 neighbours rules.
    reg4(QRegExp("[0-4]{0,5}")), //For 4 neighbours rules.
    gridRatio(10),
    infoDialog(new InfoDialog(this)),
    intervalMin(25),
    intervalMax(1000)
{
    ui->setupUi(this);

    readDefaults();
    readRulesets();

    //Make a color square to show current color.
    QPixmap icon(12, 12);
    icon.fill(currentColor);
    ui->colorButton->setIcon( QIcon(icon) );

    connect(ui->StartStopBut, SIGNAL(clicked()), this,SLOT(startStopGame()));
    connect(game,SIGNAL(gameStops(bool)), this,SLOT(startStopGame()));
    connect(ui->saveBut, SIGNAL(clicked()), this,SLOT(saveGame()));
    connect(ui->loadBut, SIGNAL(clicked()), this,SLOT(loadGame()));
    connect(ui->rootBut, SIGNAL(clicked()), this,SLOT(setTreeRoot()));
    connect(ui->ClearBut, SIGNAL(clicked()), game,SLOT(clear()));
    connect(ui->stepBut, SIGNAL(clicked()), game, SLOT(step()));
    connect(ui->infoBut, SIGNAL(clicked()), this, SLOT(showInfo()));
    connect(ui->intervalSlider, SIGNAL(valueChanged(int)), this, SLOT(setInterval(int)));
    connect(ui->heightControl, SIGNAL(valueChanged(int)), game, SLOT(setUniverseHeight(int)));
    connect(ui->widthControl, SIGNAL(valueChanged(int)), game, SLOT(setUniverseWidth(int)));
    connect(ui->heightControl, SIGNAL(valueChanged(int)), this, SLOT(gridResize()));
    connect(ui->widthControl, SIGNAL(valueChanged(int)), this, SLOT(gridResize()));
    connect(ui->modeBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setNeighMode(int)));
    connect(ui->edgeRadio, SIGNAL(toggled(bool)), this, SLOT(setEdgeMode(bool)));
    connect(ui->Bstates, SIGNAL(textChanged(QString)), this, SLOT(setBStates(QString)));
    connect(ui->Sstates, SIGNAL(textChanged(QString)), this, SLOT(setSStates(QString)));
    connect(game,SIGNAL(info(QString)), ui->labelInfo, SLOT(setText(QString)));
    connect(ui->rulesetsBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectRuleset(int)));
    connect(ui->addBut, SIGNAL(clicked()), this, SLOT(addRuleset()));
    connect(ui->removeBut, SIGNAL(clicked()), this, SLOT(removeRuleset()));
    connect(ui->invBut,  SIGNAL(clicked()), game, SLOT(invert()));

    connect(game, SIGNAL(sendGen(int)), ui->lcdG, SLOT(display(int)));
    connect(game, SIGNAL(sendPop(int)), ui->lcdP, SLOT(display(int)));
    connect(game, SIGNAL(sendXY(int,int)), this, SLOT(showCoord(int, int)));

    connect(ui->colorButton, SIGNAL(clicked()), this, SLOT(selectMasterColor()));
    connect(ui->zoomInBut, SIGNAL(clicked()), this, SLOT(zoomIn()));
    connect(ui->zoomOutBut, SIGNAL(clicked()), this, SLOT(zoomOut()));

    connect(game, SIGNAL(crwheelup()), this, SLOT(zoomIn()));
    connect(game, SIGNAL(crwheeldw()), this, SLOT(zoomOut()));
    connect(game, SIGNAL(wheelup()), this, SLOT(scrollUp()));
    connect(game, SIGNAL(wheeldw()), this, SLOT(scrollDw()));
    connect(game, SIGNAL(stwheelup()), this, SLOT(scrollRt()));
    connect(game, SIGNAL(stwheeldw()), this, SLOT(scrollLt()));

    connect(ui->SaveMenu, SIGNAL(triggered()), this, SLOT(saveGame()));
    connect(ui->LoadMenu, SIGNAL(triggered()), this, SLOT(loadGame()));

    game->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    ui->GameArea->setBackgroundRole(QPalette::Dark);
    ui->GameArea->setWidget(game); //Custom widget, has to be added manually.
    ui->GameArea->setWidgetResizable(false);
    ui->GameArea->setAlignment(Qt::AlignHCenter);
    ui->GameArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->GameArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    game->setBackgroundRole(QPalette::Base);
    game->resize(game->getUniverseWidth() * gridRatio, game->getUniverseHeight() * gridRatio);
    game->setToolTip("¤ Left click to draw \n¤ Right click to erase");

    treeModel = new QFileSystemModel(this);
    ui->treeView->setModel(treeModel);
    ui->treeView->setRootIndex(treeModel->setRootPath(treeRoot));
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);

    ui->Bstates->setValidator( new QRegExpValidator(reg8, this) );
    ui->Sstates->setValidator( new QRegExpValidator(reg8, this) );

    ui->lcdG->setPalette(Qt::green);
    ui->lcdP->setPalette(Qt::green);
    ui->lcdX->setPalette(Qt::red);
    ui->lcdY->setPalette(Qt::red);
    ui->labelInfo->setText("...");

}

//Destructor:
MainWindow::~MainWindow()
{
    delete ui;
}

//====================================================================================~~File_Handling~~====================================================================================


void MainWindow::readDefaults()
{
    //Reads .ini default files and initializes UI variables.

    // Root Path:
    QFile file_r(".." + QString(QDir::separator()) + "rootpath.ini");
    if(!file_r.open(QIODevice::ReadOnly)){
        //if no default.ini, write it.
        file_r.open(QIODevice::WriteOnly | QIODevice::Truncate);
        treeRoot = ".." + QString(QDir::separator()) + "patterns";
        file_r.write(treeRoot.toUtf8());
        file_r.close();
     } else {
        // .ini found, read path
        QTextStream in_r(&file_r);
        in_r >> treeRoot;
        treeRoot.replace("/", QDir::separator());
        treeRoot.replace("\\", QDir::separator());
        if(QDir(treeRoot).exists()){
            curPath = treeRoot;
            file_r.close();
        } else {
            //Revert to default if the path doesn't exist.
            //"./pattern" is the go-to default but if it would be removed, the treeModel would go to the system root until a new valid path is added using setTreeRoot().
            file_r.close();
            file_r.open(QIODevice::WriteOnly | QIODevice::Truncate);
            treeRoot = "." + QString(QDir::separator()) + "patterns";
            file_r.write(treeRoot.toUtf8());
            file_r.close();
        } }

    // Startup game settings:
    QFile file_d(".." + QString(QDir::separator())+ "default.ini");
    if(!file_d.open(QIODevice::ReadOnly)){
        //if no default.ini, write it.
        file_d.open(QIODevice::WriteOnly | QIODevice::Truncate);
        QString def = "#grid:\nheight:50\nwidth:50\n\n#game:\ninterval:100\nmode:m\nruleB:3\nruleS:23\n\n#color:\nr:0\ng:0\nb:0";
        file_d.write(def.toUtf8());
        defBstates = "3";
        defSstates = "23";
    }
    //read:
    int r = 0;  int g = 0; int b = 0;
    QTextStream in_d(&file_d);
    QString buf;
    while( ! in_d.atEnd()){
        in_d >> buf;
        if(buf != "" || buf.startsWith("#")){
            //ignore comments and empty lines.
            QList<QString> var = buf.split(":");
            //Look for identifiers:
            if(var[0] == "height"){
               ui->heightControl->setValue(var[1].toInt());
               game->setUniverseHeight(var[1].toInt());
            } else if(var[0] == "width"){
               ui->widthControl->setValue(var[1].toInt());
               game->setUniverseWidth(var[1].toInt());
            } else if(var[0] == "interval"){
               ui->intervalSlider->setValue(var[1].toInt());
               setInterval(var[1].toInt());
            } else if(var[0] == "mode"){
                if(var[1] == "m"){
                    ui->modeBox->setCurrentIndex(0);
                    game->setNeighMode('m');
               } else {
                    ui->modeBox->setCurrentIndex(1);
                    game->setNeighMode('v');
               }
            } else if(var[0] == "ruleB") {
                ui->Bstates->setText(var[1]);
                defBstates = var[1];
                setBStates(var[1]);
            } else if(var[0] == "ruleS"){
                ui->Sstates->setText(var[1]);
                defSstates = var[1];
                setSStates(var[1]);
            } else if(var[0] == "r"){
                r = var[1].toInt();
            } else if(var[0] == "g"){
               g = var[1].toInt();
            } else if(var[0] == "b"){
               b = var[1].toInt();
            }
         }
     }
    //Set Color
    currentColor = QColor(r,g,b);
    game->setMasterColor(currentColor); // sets color of the cells
    QPixmap icon(12, 12); // icon on the button
    icon.fill(currentColor); // fill with new color
    ui->colorButton->setIcon( QIcon(icon) );
    file_d.close();
}


void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    //Open double clicked file.
     QString filename = treeModel->fileInfo(index).filePath();
     readGame(filename);
}


void MainWindow::readRulesets(){
    //Reads ruleset codes and add them to the list.
    QFile file(".." + QString(QDir::separator()) + "rulesets.ini");
    disconnect(ui->rulesetsBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectRuleset(int))); //Needs to be disconected to clear or causes crash (index error).
    ui->rulesetsBox->clear();
    connect(ui->rulesetsBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectRuleset(int)));
    rulesets.clear();
    if(!file.open(QIODevice::ReadOnly)){
        //If no file, write it.
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);
        QString r = "3|23|m|p|Life"; //Let's at leat have Conway's game.
        file.write(r.toUtf8());
        QList<QString> token;
        rulesets.append(token);
        rulesets[0].append("3");
        rulesets[0].append("23");
        rulesets[0].append("m");
        rulesets[0].append("Life");
        ui->rulesetsBox->addItem(rulesets[0][3]);
    }
    QTextStream in(&file);
    QString buf;
    int i = 0;
    //Read rulesets:
    while( !in.atEnd()){
        in >> buf;
        QList<QString> tmp = buf.split("|");
        if(tmp.size() >= 4){
            rulesets.append(tmp);
            ui->rulesetsBox->addItem(rulesets[i][3]);
            i++;
        }

    }
    file.close();
    ui->rulesetsBox->addItem(" "); //Placeholder Item.
    qobject_cast<QStandardItemModel *>(ui->rulesetsBox->model())->item(  ui->rulesetsBox->count() -1 )->setEnabled( false );
}


void MainWindow::writeRulesets(){
    //Rulesets save to file.
    QFile file(".." + QString(QDir::separator()) + "rulesets.ini");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {return;}
    QString s;
    for( int i = 0; i < rulesets.size(); i++)
    {
        s = "";
        for(int j=0; j<rulesets[i].size(); j++)
        {
        s += rulesets[i][j] + "|";
        }
        s.chop(1); //No "|" after last element.
        s += "\n";
        file.write(s.toUtf8());
    }
    file.close();
}


void MainWindow::saveGame()
//Game dump handler to file. Saves on prompted path.
{
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save current game"),
                                                    curPath,
                                                    tr("Life automaton Files (*.laut)"));
    if(filename.length() < 1){
        return;}
    if(!filename.endsWith(".laut")){ filename += ".laut"; }
        //extension used to filter files. Files without it will not be loaded.
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {return;}
    curPath =  QFileInfo(file).absolutePath(); //Save path for next time.
    QString n;
    if(ui->modeBox->currentIndex() == 0){ n = 'm';} else { n = 'v';}
    QString s = ui->Bstates->text() + "|" + ui->Sstates->text() + "|" + n + "\n"; //Ruleset for the pattern.
    file.write(s.toUtf8());
    s = QString::number(game->getUniverseHeight()) + "|" + QString::number(game->getUniverseWidth()) +"\n"; //Grid dimensions.
    file.write(s.toUtf8());
    if(ui->edgeRadio->isChecked()){ s = "t\n"; } else { s = "p\n"; } //connected edges?
    file.write(s.toUtf8());
    file.write(game->dump().toUtf8()); //Game "o"/"*" dump.
    QColor color = game->masterColor();
    s = QString::number(color.red())+" "+  //RGB setting for cells.
                  QString::number(color.green())+" "+
                  QString::number(color.blue())+"\n";
    file.write(s.toUtf8());
    s = QString::number(ui->intervalSlider->value())+"\n";   //Timer setting.
    file.write(s.toUtf8());
    file.close();
    ui->labelInfo->setText("Pattern saved: " + QFileInfo(file).fileName());
}


void MainWindow::loadGame()
//Prompt for file loading.
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Open saved game"),
                                                    curPath,
                                                    tr("Life automaton Files (*.laut)"));
    if(filename.length() < 1)
        return;
    readGame(filename);

}


void MainWindow::readGame(QString filename)
{
    //Game dump handler from file. Can be called from prompt or from the treeview.
    game->clear();
    gridRatio = 10; //Ratio reset is necessary because apparently loading a small pattern when zoomed far out crashes the widget.
    scaleGame(1.0);
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly) || !filename.endsWith(".laut")){
        //extension used to filter files. Files without it will not be loaded.
        return;}
    curPath =  QFileInfo(file).absolutePath(); //Save path for next time.
    QTextStream in(&file);

    //Setup ruleset:
    QString tmp;
    in >> tmp;
    QList<QString> tmpl =  tmp.split('|');
    ui->Bstates->setText(tmpl[0]);
    ui->Sstates->setText(tmpl[1]);
    if(tmpl[2] == "m"){
        ui->modeBox->setCurrentIndex(0);
    }else{
        ui->modeBox->setCurrentIndex(1);
    }
    //Setup grid:
    in >> tmp;
    tmpl =  tmp.split('|');
    int h = tmpl[0].toInt();
    int w = tmpl[1].toInt();
    ui->heightControl->setValue(h);
    ui->widthControl->setValue(w);
    //Edge mode:
    in >> tmp;
    if( tmp == "t"){ ui->edgeRadio->setChecked(true);}
    else { ui->edgeRadio->setChecked(false);}
    //Load dump:
    QString dump="";
    for(int k=0; k != h ; k++) {
        QString t;
        in >> t;
        dump.append(t+"\n");
    }
    game->setDump(dump);
    //Setup RGB setting.
    int r,g,b;
    in >> r >> g >> b;
    currentColor = QColor(r,g,b);
    game->setMasterColor(currentColor); // sets color of the cells
    QPixmap icon(12, 12); // icon on the button
    icon.fill(currentColor); // fill with new color
    ui->colorButton->setIcon( QIcon(icon) ); // set icon for button
    in >> r; // r will be interval number
    //Setup speed increment:
    ui->intervalSlider->setValue(r);
    setInterval(r);
   //End:
    file.close();
    ui->labelInfo->setText("Pattern loaded: " + QFileInfo(file).fileName());
    game->update();
}

//====================================================================================~~Action-Slots~~====================================================================================

void MainWindow::startStopGame()
{
    //Handler for start/pause button and other interruptions.
    if(state == false)
    {
        state = true;
        game->startGame();
        ui->StartStopBut->setIcon(QIcon(":/icons/icons/pause.png"));
        ui->StartStopBut->setToolTip("Pause game (space)");
    } else {
        state = false;
        game->stopGame();
        ui->StartStopBut->setIcon(QIcon(":/icons/icons/play.png"));
        ui->StartStopBut->setToolTip("Start game (space)");
    }
}

void MainWindow::setInterval(int ms){
    game->setInterval(ms);
    ui->intervalSlider->setToolTip("<html>Turn interval ("  + QString::number(ms) + "ms<sup>-1</sup>)</html>");
}

void MainWindow::setTreeRoot()
//Prompt for the tree root button. Saves path in ini file.
{
    QString tmp = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                treeRoot,
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);
    if(tmp != ""){
        tmp = QDir("." + QString(QDir::separator())).relativeFilePath(tmp);
        treeRoot = tmp;
        ui->treeView->setRootIndex(treeModel->setRootPath(treeRoot));
        QFile file(".." + QString(QDir::separator()) + "rootpath.ini");
        if(!file.open(QIODevice::WriteOnly| QIODevice::Truncate)){
            return;}
        file.write(treeRoot.toUtf8());
        file.close();
    }
}


void MainWindow::selectRuleset(int index)
{
    //Handler for the ruleset comboBox.
    QList<QString> rule = rulesets[index];
    if(rule[2] == "v"){
        ui->modeBox->setCurrentIndex(1);
        setNeighMode(1);
    } else {
        ui->modeBox->setCurrentIndex(0);
        setNeighMode(0);
    }
    ui->Bstates->setText(rule[0]);
    setBStates(rule[0]);
    ui->Sstates->setText(rule[1]);
    setSStates(rule[1]);
    ui->labelInfo->setText("Ruleset \"" + rule[3] + "\"(" + "B" + rule[0] + "/S" + rule[1] + ") selected.");
}

void MainWindow::addRuleset()
{
    //Handler for add button. Adds ruleset with prompted name. Overwrites duplicates.
    QList<QString> rule =getRuleSet();
    QString name = QInputDialog::getText(this ,"Name your ruleset.",
                           "Ruleset name:", QLineEdit::Normal);
    name.replace(" ", "_");
    if(name == ""){
        return;}
    int r = rulesetExists(rule); //Detect if that ruleset is already present
    if(r > -1){
        //This rule already exists, we just change the name.
        rulesets[r][3] = name;
        ui->labelInfo->setText("Ruleset (B" + rule[0] + "/S" + rule[1] + ") renamed to \"" + name + "\"." );
    } else {
        rule[3] = name;
        rulesets.append(rule);
        ui->labelInfo->setText("Ruleset \"" + rule[3] + "\"(" + "B" + rule[0] + "/S" + rule[1] + ") added.");
    }
    writeRulesets();
    readRulesets();
    setBStates(rule[0]);
    setSStates(rule[1]);
}


QList<QString> MainWindow::getRuleSet()
{
//Method to retrieve the current settings in the same format as ruleset elements.
    QList<QString> rule;
    for(int i = 0; i <= 3; i++){ rule.append(""); }
    rule[0] = ui->Bstates->text();
    rule[1] = ui->Sstates->text();
    if(ui->modeBox->currentIndex() == 0){
        rule[2] = "m";
    } else {
        rule[2] = "v";
    }
    rule[3].append("");
    return rule;
}

int MainWindow::rulesetExists(QList<QString> rule){
    //Compare a ruleset to saved rulesets, ignores name.
    for(int r = 0; r < rulesets.size(); r++){
        if(rulesets[r][0] == rule[0] && rulesets[r][1] == rule[1] && rulesets[r][2] == rule[2])
        { return r; }
     }
    return -1;
}

void MainWindow::removeRuleset()
{
    //deletes selected ruleset.
    rulesets.removeAt(ui->rulesetsBox->currentIndex());
    writeRulesets();
    readRulesets();
    ruleSwich();
}


void MainWindow::ruleSwich()
{
    //Detects if input ruleset already exists and switches combobox to it or placeholder.
    disconnect(ui->rulesetsBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectRuleset(int)));
    //Disconect before switching to avoid endless feedback loop.
    int r = rulesetExists(getRuleSet());
    if( r > -1) {
        ui->rulesetsBox->setCurrentIndex(r); //Place on corresponding item;
        ui->removeBut->setEnabled(true); //Rule can be deleted.
    } else {
        ui->rulesetsBox->setCurrentIndex(ui->rulesetsBox->count() - 1); //Or put on placeholder.
        ui->removeBut->setDisabled(true); //Nothing to delete.
    }
    connect(ui->rulesetsBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectRuleset(int))); //reconnect
}


void MainWindow::selectMasterColor()
{
    //Color dialog handler for the button.
    QColor color = QColorDialog::getColor(currentColor, this, tr("Select color of figures"));
    if(!color.isValid())
        return;
    currentColor = color;
    game->setMasterColor(color);
    QPixmap icon(12, 12);
    icon.fill(color);
    ui->colorButton->setIcon( QIcon(icon) );
}


void MainWindow::setNeighMode(int index)
//Swiches neighbourhood mode in the game instance.
//   'm' mode = Moore -> 8 neighbours considered.
//   'm' mode = Von Neumann -> Only the 4 direct cardinal neighbours are considered.
{
    if(state)
    {
        startStopGame();
    }
    if(index == 0){
        game->setNeighMode('m');
        //Filter characters from regex:
        ui->Bstates->setValidator( new QRegExpValidator(reg8, this) );
        ui->Sstates->setValidator( new QRegExpValidator(reg8, this) );
    }
    if(index == 1){
        game->setNeighMode('v');
        //Filter characters from regex:
        ui->Bstates->setValidator( new QRegExpValidator(reg4, this) );
        ui->Sstates->setValidator( new QRegExpValidator(reg4, this) );
    }
    //reset states to avoid conflict:
    ui->Bstates->setText(defBstates);
    ui->Sstates->setText(defSstates);
    ruleSwich(); //Compare new ruleset.
}

void MainWindow::setEdgeMode(bool state)
{
    if(state){game->setEdgeMode('t');}
    else{game->setEdgeMode('p');}
}


void MainWindow::setBStates(QString b)
//handler for rule lineedit input. Converts string to a sorted number array without duplicates.
{
    if(state)
    {
        startStopGame();
    }
    //Read Regex validated string.
    QList<int> array;
    foreach(QChar c, b){
        int n = c.digitValue();
        if(!array.contains(n)){
            array.append(n);
        }
    }
    // Change Game rules.
    game->setBirthStates(array);
    // Don't allow duplicates + sort
    qSort(array);
    QString nText;
    for(int i=0; i<array.size(); i++)
    {
    nText += QString::number(array[i]);
    }
    ui->Bstates->setText(nText);
    ruleSwich(); //Compare new ruleset.
}


void MainWindow::setSStates(QString s)
//handler for rule lineedit input. Converts string to a sorted number array without duplicates.
{
    if(state)
    {
        startStopGame();
    }
    game->stopGame();
    //Read Regex validated string.
    QList<int> array;
    foreach(QChar c, s){
        int n = c.digitValue();
        if(!array.contains(n)){
            array.append(n);
        }
    }
    // Change Game rules.
    game->setSurvStates(array);
    // Don't allow duplicates + sort
    qSort(array);
    QString nText;
    for(int i=0; i<array.size(); i++)
    {
    nText += QString::number(array[i]);
    }
    ui->Sstates->setText(nText);
    ruleSwich(); //Compare new ruleset.
}


void MainWindow::showInfo()
{
    //Reciever for various status update messages.
    infoDialog.show();
    infoDialog.activateWindow();
}


//====================================================================================~~Graphical-slots~~====================================================================================


void MainWindow::scaleGame(double factor)
{
    //Grid scaler for zoom actions.
    gridRatio = ( factor * gridRatio); //Modify ratio.
    //Limits for resizing:
    if( gridRatio <= 1.5){
        gridRatio = 1.5;
        factor = 1;
    }
    if(gridRatio > 300){
        gridRatio = 300;
        factor = 1;
    }
    //Apply new ratio:
    game->resize(game->getUniverseWidth() * gridRatio, game->getUniverseHeight() * gridRatio);
    // Relative (-0.8:0.8) offsets to center axes of the game area:
    //0.8 gives a softer transition.
    float y = 0.8 * ((ui->GameArea->mapFromGlobal(QCursor::pos()).y() - ((float)ui->GameArea->height() / 2) ) / (float)ui->GameArea->height());
    float x = 0.8 * ((ui->GameArea->mapFromGlobal(QCursor::pos()).x() - ((float)ui->GameArea->width() / 2) ) / (float)ui->GameArea->width());
    //go to cursor position:
    adjustScrollBar(ui->GameArea->verticalScrollBar(), factor, y);
    adjustScrollBar(ui->GameArea->horizontalScrollBar(), factor, x);

}

void MainWindow::showCoord(int x, int y)
{
 //Coordinates reciever for display.
    ui->lcdX->display(x);
    ui->lcdY->display(y);
}

void MainWindow::gridResize()
{
    //Used only for resize when the grid dimensions change.
    if( gridRatio <= 1.5){
        gridRatio = 1.5;
    }
    if(gridRatio > 300){
        gridRatio = 300;
    }
    game->clear();
    game->resize(game->getUniverseWidth() * gridRatio, game->getUniverseHeight() * gridRatio);
}

void MainWindow::adjustScrollBar(QScrollBar *scrollBar, double factor, double offset)
{
    //Allows to Zoom on the point under the cursor.
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2) //focus on center of the grid.
                            + (offset * scrollBar->maximum() ))); //Offset towards the cursor.
}


void MainWindow::zoomIn(){
    //Zoom in act in response to ctrl+wheel signal.
    scaleGame(1.10);
}


void MainWindow::zoomOut(){
     //Zoom out act  in response to ctrl+wheel signal.
    scaleGame(0.9);
}

//Scrolling navigation:
void MainWindow::scrollUp(){
    ui->GameArea->verticalScrollBar()->setValue(ui->GameArea->verticalScrollBar()->value() - (game->getUniverseHeight()/10 * gridRatio) );
}

void MainWindow::scrollDw(){
    ui->GameArea->verticalScrollBar()->setValue(ui->GameArea->verticalScrollBar()->value() + (game->getUniverseHeight()/10 * gridRatio));
}

void MainWindow::scrollRt(){
    ui->GameArea->horizontalScrollBar()->setValue(ui->GameArea->horizontalScrollBar()->value() + (game->getUniverseWidth()/10 * gridRatio));
}

void MainWindow::scrollLt(){
    ui->GameArea->horizontalScrollBar()->setValue(ui->GameArea->horizontalScrollBar()->value() - (game->getUniverseWidth()/10 * gridRatio));
}


