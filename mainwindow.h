#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "gamewidget.h"
#include "infodialog.h"
#include <QFileSystemModel>
#include <QScrollBar>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void startStopGame(); //Start/stop button + other
    void selectMasterColor(); //Color button
    void saveGame(); //Save button
    void loadGame(); //Load button
    void  readGame(QString filename); //Load button + Tree view
    void gridResize(); //X/Y dimention selectors
    void setBStates(QString b); //Rule input
    void setSStates(QString s); //Rule input
    void setNeighMode(int index); //Mode selector
    void setEdgeMode(bool state); //Mode selector
    void adjustScrollBar(QScrollBar *scrollBar, double factor, double offset); //Adjuster to follow cursor point on zoom.
    //Mouse wheel / arrow key on grid responses:
    void zoomIn();
    void zoomOut();
    void scrollUp();
    void scrollDw();
    void scrollRt();
    void scrollLt();
    void scaleGame(double factor); //Zoom in/out
    void setInterval(int ms);
    //---------------
    void showCoord(int x, int y);//Display
    void setTreeRoot();//Root path button
    void selectRuleset(int index); //Ruleset selector
    void addRuleset(); //Add ruleset button
    void removeRuleset(); //Remove ruleset button
    void showInfo(); //Status label
    void readDefaults(); //.ini file handler
    void readRulesets(); //.ini file handler
    void writeRulesets(); //.ini file handler
    QList<QString> getRuleSet(); //Reads active ruleset.
    int rulesetExists(QList<QString> rule); //Active ruleset vs saved rulesets.
    void ruleSwich(); //Ruleset auto compare.


private slots:
    void on_treeView_doubleClicked(const QModelIndex &index);

private:
    bool state; //Is game running?
    Ui::MainWindow *ui; //Form
    QColor currentColor; //Cells color
    GameWidget* game; //Custom widget
    QFileSystemModel *treeModel; //Tree's model
    QRegExp reg8; //Regex filter for rule input
    QRegExp reg4; //Regex filter for rule input
    float gridRatio; //Scaling ratio nb(cells) -> size(px)
    QString treeRoot; //Root of the tree model
    QString curPath; //Save/Load path
    InfoDialog infoDialog; //"About" window handler
    int intervalMin; //ms interval limit
    int intervalMax; //ms interval limit
    QString defBstates; //default string for rule
    QString defSstates; //default string for rule
    QList<QList<QString> > rulesets; //Rulesets collection
};

#endif // MAINWINDOW_H
