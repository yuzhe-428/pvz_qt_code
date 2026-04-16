#include "mainwindow.h"
#include <QFont>
#include <QPainter>
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setFixedSize(800, 600);
    setWindowTitle("植物大战僵尸：肉鸽幸存者");
    playerPos = QPointF(50, 300);
    lastShootTime = QTime::currentTime();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::gameLoop);
    timer->start(16);
}

MainWindow::~MainWindow() {
    // 防止内存泄漏
    qDeleteAll(zombies);
    qDeleteAll(bullets);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if(isPaused) {
        int choice = -1;
        if(event->key() == Qt::Key_1) choice = 0;
        else if(event->key() == Qt::Key_2) choice = 1;
        else if(event->key() == Qt::Key_3) choice = 2;

        if(choice >= 0 && choice < currentOptions.size()) {
            Skill s = currentOptions[choice];
            if(s.type == 0) fireRate = qMax(150, fireRate - 150);
            else if(s.type == 1) bulletCount++;
            else if(s.type == 2) bulletDamage += 2;
            isPaused = false;
            timer->start();
        }
        return;
    }

    if(event->key() == Qt::Key_W && playerPos.y() > 50) playerPos.ry() -= 30;
    if(event->key() == Qt::Key_S && playerPos.y() < 550) playerPos.ry() += 30;
}

void MainWindow::spawnZombie() {
    Zombie* z = new Zombie();
    z->pos = QPointF(850, QRandomGenerator::global()->bounded(50, 550));
    z->isFast = QRandomGenerator::global()->bounded(10) > 8;
    z->hp = 2 + (level / 2);
    zombies.append(z);
}

void MainWindow::showLevelUpMenu() {
    isPaused = true;
    timer->stop();
    currentOptions.clear();
    currentOptions.append({"超级射速", "发射频率大幅提升", 0});
    currentOptions.append({"多重射击", "一次发射更多子弹", 1});
    currentOptions.append({"重型弹药", "子弹伤害翻倍", 2});
    update();
}

void MainWindow::gameLoop() {
    if(lastShootTime.msecsTo(QTime::currentTime()) > fireRate) {
        for(int i=0; i<bulletCount; ++i) {
            Bullet* b = new Bullet();
            b->pos = playerPos + QPointF(30, 0);
            b->damage = bulletDamage;
            float spread = (i - (bulletCount-1)/2.0f) * 1.0f;
            b->velocity = QVector2D(6.0f, spread);
            bullets.append(b);
        }
        lastShootTime = QTime::currentTime();
    }

    // 使用 std::as_const 或 const 引用遍历指针列表，解决 detach 警告
    for(auto z : std::as_const(zombies)) z->move();
    for(auto b : std::as_const(bullets)) b->move();

    for(int i = bullets.size()-1; i>=0; --i) {
        if(bullets[i]->pos.x() > 800) {
            delete bullets.takeAt(i);
            continue;
        }
        for(int j = zombies.size()-1; j>=0; --j) {
            if(QLineF(bullets[i]->pos, zombies[j]->pos).length() < 30) {
                zombies[j]->hp -= bullets[i]->damage;
                delete bullets.takeAt(i);
                if(zombies[j]->hp <= 0) {
                    delete zombies.takeAt(j);
                    score += 10;
                    exp++;
                    if(exp >= expToNextLevel) {
                        level++;
                        exp = 0;
                        expToNextLevel += 3;
                        showLevelUpMenu();
                    }
                }
                break;
            }
        }
    }

    for(int i = zombies.size()-1; i>=0; --i) {
        if(zombies[i]->pos.x() < -50) {
            delete zombies.takeAt(i);
            score = qMax(0, score - 20);
        }
    }

    if(QRandomGenerator::global()->bounded(100) < 5) spawnZombie();
    update();
}

void MainWindow::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 1. 绘制背景图 (填满整个窗口)
    QPixmap bg(":/images/background.png");
    if(!bg.isNull()) {
        p.drawPixmap(rect(), bg);
    } else {
        // 如果图片加载失败，备用深绿色背景
        p.fillRect(rect(), QColor(34, 139, 34));
    }

    // 2. 画玩家 (植物)
    // 根据你的 mainwindow.h，变量名是 playerPos
    p.drawPixmap(playerPos.x() - 30, playerPos.y() - 30, 60, 60, QPixmap(":/images/plant.png"));

    // 3. 画僵尸
    for(auto z : std::as_const(zombies)) {
        // 使用 -> 访问指针，路径对应 qrc
        p.drawPixmap(z->pos.x() - 35, z->pos.y() - 35, 70, 70, QPixmap(":/images/zombie.png"));
    }

    // 4. 画子弹
    for(auto b : std::as_const(bullets)) {
        p.drawPixmap(b->pos.x() - 10, b->pos.y() - 10, 20, 20, QPixmap(":/images/bullet.png"));
    }

    // 5. 绘制 UI 文字
    p.setPen(Qt::white);
    p.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    p.drawText(20, 30, QString("等级: %1  经验: %2/%3  分数: %4")
                           .arg(level).arg(exp).arg(expToNextLevel).arg(score));

    // 6. 升级菜单逻辑
    if(isPaused) {
        p.fillRect(rect(), QColor(0, 0, 0, 180)); // 半透明遮罩
        p.setFont(QFont("Arial", 20, QFont::Bold));
        p.drawText(rect(), Qt::AlignTop | Qt::AlignHCenter, "\n\nLEVEL UP! CHOOSE A BUFF");
        p.setFont(QFont("Arial", 14));
        for(int i=0; i<currentOptions.size(); i++) {
            p.drawText(250, 250 + i*60, QString("按 [%1]: %2 (%3)")
                                              .arg(i+1).arg(currentOptions[i].name).arg(currentOptions[i].desc));
        }
    }
}