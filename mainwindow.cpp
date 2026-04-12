#include "mainwindow.h"
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setFixedSize(900, 600);
    gameTimer = new QTimer(this);
    gameTimer->start(30);
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::updateGame);

    zombies.push_back({{800, 200}, 3, false});
    lastShootTime = QTime::currentTime();
    lastSunTime = QTime::currentTime();
    sunCount = 50;
}

MainWindow::~MainWindow()
{
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(34, 139, 34));

    for(int i=0; i<9; i++){
        for(int j=0; j<5; j++){
            p.drawRect(i*100, j*120, 100, 120);
        }
    }

    p.setBrush(Qt::green);
    for(auto& plant : plants)
        p.drawEllipse(plant, 30, 30);

    for(auto& z : zombies) {
        if(z.isRoadBlock) {
            p.setBrush(Qt::darkGray);
        } else {
            if(z.hp >= 5) p.setBrush(Qt::red);
            else if(z.hp >= 3) p.setBrush(QColor(255,150,0));
            else p.setBrush(QColor(139,69,19));
        }
        p.drawEllipse(z.pos, 35, 35);
    }

    p.setBrush(Qt::white);
    for(auto& b : bullets)
        p.drawEllipse(b, 8, 8);

    p.setBrush(Qt::yellow);
    for(auto& sun : suns)
        p.drawEllipse(sun, 20, 20);

    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 20));
    p.drawText(20, 40, "阳光: " + QString::number(sunCount));
}

void MainWindow::mousePressEvent(QMouseEvent *e)
{
    QPointF pos = e->position();
    int x = pos.x();
    int y = pos.y();

    for(int i=0; i<suns.size(); i++){
        if(qAbs(x - suns[i].x())<25 && qAbs(y - suns[i].y())<25){
            suns.removeAt(i);
            sunCount +=25;
            return;
        }
    }

    if(sunCount >=50){
        plants.append(pos.toPoint());
        sunCount -=50;
    }
}

void MainWindow::updateGame()
{
    for(int i=0; i<zombies.size(); i++)
        zombies[i].pos.setX(zombies[i].pos.x()-1);

    for(int i=0; i<zombies.size(); i++){
        if(zombies[i].pos.x() <0){
            zombies.removeAt(i);
            break;
        }
    }

    if(zombies.isEmpty()){
        int row = QRandomGenerator::global()->bounded(0,5);
        bool road = QRandomGenerator::global()->bounded(2) == 0;
        zombies.push_back({{800, row*120+60}, road ? 6 : 3, road});
    }

    if(QTime::currentTime().msecsTo(lastSunTime) <=-3000){
        int x = QRandomGenerator::global()->bounded(100,800);
        int y = QRandomGenerator::global()->bounded(50,500);
        suns.append({x,y});
        lastSunTime = QTime::currentTime();
    }

    if(QTime::currentTime().msecsTo(lastShootTime) <=-2000){
        for(auto& plant : plants)
            bullets.append({plant.x()+30, plant.y()});
        lastShootTime = QTime::currentTime();
    }

    for(int i=0; i<bullets.size(); i++)
        bullets[i].setX(bullets[i].x()+3);

    for(int i=0; i<bullets.size(); i++){
        if(bullets[i].x()>width()){
            bullets.removeAt(i);
            break;
        }
    }

    for(int i=0; i<bullets.size(); i++){
        for(int j=0; j<zombies.size(); j++){
            auto& b = bullets[i];
            auto& z = zombies[j];
            if(qAbs(b.x()-z.pos.x())<40 && qAbs(b.y()-z.pos.y())<40){
                bullets.removeAt(i);
                z.hp -=1;
                if(z.hp <=0) zombies.removeAt(j);
                return;
            }
        }
    }

    update();
}