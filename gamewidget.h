#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QColor>
#include <QWidget>
#include <QList>

class GameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GameWidget(QWidget *parent = 0);
    ~GameWidget();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    void keyPressEvent(QKeyEvent *e);

signals:
    void gameStops(bool ok);
    void sendGen(int g);
    void sendXY(int x, int y);
    void sendPop(int p);
    void wheelup();
    void wheeldw();
    void crwheelup();
    void crwheeldw();
    void stwheelup();
    void stwheeldw();
    void info(QString);

public slots:
    void startGame(); // start
    void stopGame(); // finish
    void clear(); // clear
    void step();
    void invert();

    int getUniverseHeight();
    int getUniverseWidth();
    void setUniverseHeight(const int &s); // set number of the cells in one row
    void setUniverseWidth(const int &s);
    void setNeighMode(char mode);
    void setEdgeMode(char mode);

    void setBirthStates( QList<int> states);
    void setSurvStates( QList<int> states);

    int interval(); // interval between generations
    void setInterval(int msec); // set interval between generations

    QColor masterColor(); // color of the cells
    void setMasterColor(const QColor &color); // set color of the cells

    QString dump(); // dump of current universe
    void setDump(const QString &data); // set current universe from it's dump

private slots:
    void paintGrid(QPainter &p);
    void paintUniverse(QPainter &p);
    void newGeneration();

private:
    QColor m_masterColor;
    QTimer* timer;
    int generations;
    QList<int> BirthStates;
    QList<int> SurvStates;
    int universeHeight;
    int universeWidth;
    char neighMode;
    char edgeMode;
    bool** universe; // map
    bool** next; // map
    bool interupted;
    int population;

    bool isAlive(int k, int j); // return true if universe[k][j] accept rules
    void resetUniverse();// reset the size of universe
};

#endif // GAMEWIDGET_H
