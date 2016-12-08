#include <QMessageBox>
#include <QTimer>
#include <QMouseEvent>
#include <QDebug>
#include <QRectF>
#include <QPainter>
#include <qmath.h>
#include "gamewidget.h"


//Constructor:
GameWidget::GameWidget(QWidget *parent) :
    QWidget(parent),
    timer(new QTimer(this)),
    generations(0),
    universeHeight(50),
    universeWidth(50),
    neighMode('m'),
    edgeMode('p'),
    population(0)
{

    timer->setInterval(100);
    m_masterColor = "#000";
    resetUniverse();
    connect(timer, SIGNAL(timeout()), this, SLOT(newGeneration()));
    setMouseTracking(true);
    BirthStates <<  3;
    SurvStates << 2;
    SurvStates << 3;
}

//Destructor:
GameWidget::~GameWidget()
{
    delete [] universe;
    delete [] next;
}


//Methods:
void GameWidget::startGame()
{
    timer->start();
    emit info("Game started.");
}

void GameWidget::stopGame()
{
    if(timer->isActive()){
        timer->stop();
        emit info("Game paused.");
    }
}

void GameWidget::clear()
{
    generations = 0;
    emit sendGen(generations);
    if(timer->isActive()){
        stopGame();
        emit gameStops(true);
    }
    for(int k = 1; k <= universeHeight; k++) {
        for(int j = 1; j <= universeWidth; j++) {
            universe[k][j] = false;
        }
    }
    update();
    emit info("Board cleared");
    population = 0;
    emit sendPop(population);
}

int GameWidget::getUniverseHeight()
{
    return universeHeight;
}


int GameWidget::getUniverseWidth()
{
    return universeWidth;
}


void GameWidget::setUniverseHeight(const int &s)
{
    universeHeight = s;
    resetUniverse();
    update();
}


void GameWidget::setUniverseWidth(const int &s)
{
    universeWidth = s;
    resetUniverse();
    update();
}


void GameWidget::setNeighMode(char mode)
{
    neighMode = mode;
}

void GameWidget::setEdgeMode(char mode)
{
    edgeMode = mode;
}

void GameWidget::resetUniverse()
{
    //Empty universe building algorithm.
    delete [] universe;
    delete [] next;
    // +2 to array dimension to give a buffer zone.
    universe = new bool*[universeHeight + 2];
    for(int k = 0; k < universeHeight + 2; k++){
       universe[k] = new bool[universeWidth + 2];
       for(int j = 0; j < universeWidth + 2; j++){
           universe[k][j] = false;
       }
    }
    next = new bool*[universeHeight + 2];
    for(int k = 0; k < universeHeight + 2; k++){
       next[k] = new bool[universeWidth + 2];
       for(int j = 0; j < universeWidth + 2; j++){
           next[k][j] = false;
       }
    }
    population = 0;
    emit sendPop(population);
}

void GameWidget::invert()
{
    for(int k = 1; k <= universeHeight; k++){
     for(int j = 1; j <= universeWidth; j++){
         universe[k][j] = !universe[k][j];
     }
    }
    update();
}



QString GameWidget::dump()
{
    char temp;
    QString master = "";
    for(int k = 1; k <= universeHeight; k++) {
        for(int j = 1; j <= universeWidth; j++) {
            if(universe[k][j] == true) {
                temp = '*';
            } else {
                temp = 'o';
            }
            master.append(temp);
        }
        master.append("\n");
    }
    return master;
}

void GameWidget::setDump(const QString &data)
{
    int current = 0;
    for(int k = 1; k <= universeHeight; k++) {
        for(int j = 1; j <= universeWidth; j++) {
            universe[k][j] = (data[current] == '*');
            current++;
        }
        current++;
    }
    update();
}

int GameWidget::interval()
{
    return timer->interval();
}

void GameWidget::setInterval(int msec)
{
    timer->setInterval(msec);
}

void GameWidget::setBirthStates(QList<int> states)
{
    BirthStates = states;
}


void GameWidget::setSurvStates(QList<int> states)
{
    SurvStates = states;
}

/*
bool GameWidget::isAlive(int k, int j)
{
    int power = 0;
    if(neighMode == 'm'){
        //Diagonals only in moore mode.
        power += universe[k+1][ j-1];
        power += universe[k-1][j+1];
        power += universe[k-1][j-1];
        power += universe[k+1][j+1];
    }
    //Cardinals always:
    power += universe[k+1][j];
    power += universe[k-1][j];
    power += universe[k][j+1];
    power += universe[k][j-1];
    if(universe[k][j] == false){
        foreach( int b, BirthStates){
            if(power == b){ return true; }
        }
    }
    if(universe[k][j] == true){
        foreach( int s, SurvStates){
            if(power == s){ return true; }
        }
    }
    return false;
}
*/

bool GameWidget::isAlive(int k, int j)
/**
          0     1       ...     ...       uw      uw+1
          __|___________________|__
  0          |                    | k              |
              |                    |                 |
  1          |                    |                 |
   .          |                    |                 |
   :          |__________|_______> |
              |  j                 |                 |
   .          |                    |                 |
   :          |                    |                 |
 uh     __|__________v________|__
 uh+1     |                                      |
 The universe can have to shapes (edgeMode):
    -bounded plane(b): the cells at the edge have no neighbour beyond it.
        A buffer zone of 'dead' cells that are never calculated marks the limit
    -toroidal plane(t): The edges are connected, top to bottom and left to right.
        Which means the neighbour of a cell at the edge is the cell facing it at the oposite edge.
        This could be modelized as the surface of a torus (donut shaped solid).
*/
{
    int power = 0;
    for(int x = j-1; x <= j+1; x++ ){
        for(int y = k-1; y <= k+1; y++){
            int x2 = x;
            int y2 = y;
            if(edgeMode == 't'){
                //Handle edge limits by looping around in toroidal mode.
                if( x2 == 0){ x2 = universeWidth; } //<-- skip over 0 and uw+1 buffer columns. --
                else
                if( x2 == universeWidth + 1){ x2 = 1;} //-- skip over uw+1 and 0 buffer columns -->

                if( y2 == 0){ y2 = universeHeight; } //<-- skip over 0 and uh buffer rows. --
                else
                if( y2 == universeHeight + 1){ y2 = 1;} //-- skip over uh+1 and 0 buffer rows -->
            }
            if( (x2 == j || y2 == k || neighMode == 'm')
                 && !(x2 == j && y2 == k)) // don't count u[k][j] itself in its neighbours.
            {
                power += universe[y2][x2]; //count live neighbours.
            }
        }  }
    //Rule application
    if(universe[k][j] == false){
        //If dead, look if a birth state is satisfied.
        foreach( int b, BirthStates){
            // Birth state found => cell becomes alive.
            if(power == b){ return true; }
        }
    }
    if(universe[k][j] == true){
        //If alive, look if a survival state is satisfied
        foreach( int s, SurvStates){
            // Survival state found => cell remains alive.
            if(power == s){ return true; }
        }
    }
    //Nothing found => cell dies / remains dead.
    return false;
}


void GameWidget::step()
{
    newGeneration();
}


void GameWidget::newGeneration()
{
    int notChanged=0;
    for(int k=1; k <= universeHeight; k++) {
        for(int j=1; j <= universeWidth; j++) {
            next[k][j] = isAlive(k, j);
            if(next[k][j] == universe[k][j])
                notChanged++;
        }
    }
    if(notChanged == universeHeight*universeWidth) {
        emit gameStops(true);
        emit info("Game stopped: all the next generations will be the same.");
        return;
    }
    for(int k=1; k <= universeHeight; k++) {
        for(int j=1; j <= universeWidth; j++) {
            universe[k][j] = next[k][j];
        }
    }
    update();
    generations++;
    emit sendGen(generations);
    emit sendPop(population);
}


//Events:
void GameWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    paintGrid(p);
    paintUniverse(p);
}

void GameWidget::mousePressEvent(QMouseEvent *e)
{
    double cellHeight = (double)height()/universeHeight;
    double cellWidth = (double)width()/universeWidth;
    int k = floor(e->y()/cellHeight)+1;
    int j = floor(e->x()/cellWidth)+1;
    if( e->buttons() == Qt::LeftButton){
        universe[k][j] = true;
    }
    if( e->buttons() == Qt::RightButton){
        universe[k][j] = false;
    }
    update();
}

void GameWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(e->x() >= width() || e->y() >= height()){
        //Wihout this, it crashes the program when the mouse is dragged out of the game area.
        return;
    }
    double cellHeight = (double)height()/universeHeight;
    double cellWidth = (double)width()/universeWidth;
    int k = floor(e->y()/cellHeight)+1;
    int j = floor(e->x()/cellWidth)+1;
    if(e->buttons() == Qt::LeftButton){
        if(timer->isActive()){
            stopGame();
            emit gameStops(true);
            interupted = true;
        }
        universe[k][j] = true;
        update();
    }
    if(e->buttons() == Qt::RightButton){
        if(timer->isActive()){
            stopGame();
            emit gameStops(true);
            interupted = true;
        }
        universe[k][j] = false;
        update();
    }
    sendXY(j, k);
}

void GameWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if(interupted){
        startGame();
        interupted = false;
    }
}

void GameWidget::wheelEvent(QWheelEvent * e){
    if( e->modifiers() & Qt::ControlModifier ){
        if( e->delta() > 0)
        {
            emit crwheelup();
        } else {
            emit crwheeldw();
        }
    }
    else if(e->modifiers() & Qt::ShiftModifier ){
        if( e->delta() > 0)
        {
            emit stwheelup();
        } else {
            emit stwheeldw();
        }
    }
    else
    {
        if( e->delta() > 0)
        {
            emit wheelup();
        } else {
            emit wheeldw();
        }
    }
    double cellHeight = (double)height()/universeHeight;
    double cellWidth = (double)width()/universeWidth;
    int k = floor(e->y()/cellHeight)+1;
    int j = floor(e->x()/cellWidth)+1;
    emit sendXY(j, k);
}

void GameWidget::keyPressEvent(QKeyEvent *e)
{
    if(e->type() == Qt::UpArrow)
    {
        emit wheelup();
    }
    else if(e->type() == Qt::DownArrow)
    {
        emit wheeldw();
    }
    else if(e->type() == Qt::LeftArrow)
    {
        emit stwheeldw();
    }
    else if(e->type() == Qt::RightArrow)
    {
        emit stwheelup();
    }
}

//Painting methods:
void GameWidget::paintGrid(QPainter &p)
{
    QRect borders(0, 0, width()-1, height()-1); // borders of the universe
    QColor gridColor = "#000"; // color of the grid
    double cellWidth = (double)width()/universeWidth; // width of the widget / number of cells at one row
    int n = 1;
    for(double k = cellWidth; k <= width(); k += cellWidth)
    {
        if( n % 10 == 0){
            //every 10n line is thicker.
            gridColor.setAlpha(100);
            p.setPen(QPen(QBrush(gridColor), 2.0));
            p.drawLine(k, 0, k, height());
        } else {
            gridColor.setAlpha(50);
            p.setPen(QPen(QBrush(gridColor), 1.0));
            p.drawLine(k, 0, k, height());
        }
        n++;
    }
    double cellHeight = (double)height()/universeHeight; // height of the widget / number of cells at one row
    n = 1;
    for(double k = cellHeight; k <= height(); k += cellHeight)
    {
        if(n % 10 == 0){
            //every 10n line is thicker.
            gridColor.setAlpha(100);
            p.setPen(QPen(QBrush(gridColor), 2.0));
            p.drawLine(0, k, width(), k);
        } else {
            gridColor.setAlpha(50);
            p.setPen(QPen(QBrush(gridColor), 1.0));
            p.drawLine(0, k, width(), k);
        }
        n++;
    }
    p.drawRect(borders);
}

void GameWidget::paintUniverse(QPainter &p)
{
    population = 0;
    double cellWidth = (double)width()/universeWidth;
    double cellHeight = (double)height()/universeHeight;
    for(int k=1; k <= universeHeight; k++) {
        for(int j=1; j <= universeWidth; j++) {
            if(universe[k][j] == true) { // if there is any sense to paint it
                qreal left = (qreal)(cellWidth*(j-1) + 1); // margin from left
                qreal top  = (qreal)(cellHeight*(k-1) + 1); // margin from top
                QRectF r(left, top, (qreal)(cellWidth) - 1.5, (qreal)(cellHeight) - 1.5);
                p.fillRect(r, QBrush(m_masterColor)); // fill cell with brush of main color
                population++;
            }
        }
    }
    emit sendPop(population);
}

QColor GameWidget::masterColor()
{
    return m_masterColor;
}

void GameWidget::setMasterColor(const QColor &color)
{
    m_masterColor = color;
    update();
}
