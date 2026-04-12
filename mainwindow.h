#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QList>
#include <QTime>
#include <QRandomGenerator>

struct Zombie {
    QPoint pos;
    int hp;
    bool isRoadBlock;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void updateGame();

private:
    QTimer* gameTimer;
    QList<QPoint> plants;
    QList<Zombie> zombies;
    QList<QPoint> bullets;

    QList<QPoint> suns;
    int sunCount;
    QTime lastSunTime;
    QTime lastShootTime;
};

#endif