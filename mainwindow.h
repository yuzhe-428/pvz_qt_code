#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPainter>
#include <QKeyEvent>
#include <QList>
#include <QTime>
#include <QRandomGenerator>
#include <QVector2D>

// 1. 基础类：体现 OOP 思想
class GameObject {
public:
    QPointF pos;
    int hp;
    virtual void move() = 0;
    virtual ~GameObject() {}
};

// 2. 僵尸子类
class Zombie : public GameObject {
public:
    bool isFast;
    void move() override { pos.rx() -= (isFast ? 3.0 : 1.5); }
};

// 3. 子弹子类
class Bullet : public GameObject {
public:
    int damage;
    QVector2D velocity;
    void move() override { pos += velocity.toPointF(); }
};

// 4. 技能结构
struct Skill {
    QString name;
    QString desc;
    int type;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow(); // 必须析构释放内存

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();

private:
    void spawnZombie();
    void showLevelUpMenu();

    QTimer *timer;
    QPointF playerPos;
    QList<Zombie*> zombies;
    QList<Bullet*> bullets;

    // 游戏数值
    int score = 0;
    int level = 1;
    int exp = 0;
    int expToNextLevel = 5;
    bool isPaused = false;
    QList<Skill> currentOptions;

    // 强化属性
    int fireRate = 800;
    int bulletCount = 1;
    int bulletDamage = 1;
    QTime lastShootTime;
};

#endif