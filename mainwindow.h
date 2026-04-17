#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QPainter>
#include <QKeyEvent>
#include <QList>
#include <QTime>
#include <QRandomGenerator>
#include "entities.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;    // 绘图事件：负责渲染画面
    void keyPressEvent(QKeyEvent *event) override;   // 按键事件：控制移动和技能选择

private slots:
    void gameLoop(); // 游戏主循环：每一帧执行一次

private:
    void spawnZombie(bool boss = false); // 生成僵尸
    void showLevelUpMenu();               // 显示升级菜单
    void gameOver();                      // 游戏结束处理

    QTimer *timer;          // 驱动游戏循环的定时器
    QPointF playerPos;      // 玩家（植物）的位置
    QList<Zombie*> zombies; // 场景中所有的僵尸集合
    QList<Bullet*> bullets; // 场景中所有的子弹集合

    // 游戏数值状态
    int score = 0;          // 分数
    int level = 1;          // 等级
    int exp = 0;            // 当前经验
    int expToNextLevel = 5; // 升级所需经验
    bool isPaused = false;  // 是否暂停（升级时）
    bool isGameOver = false;// 是否游戏结束

    // 玩家属性
    int playerHp = 100;     // 玩家当前血量
    int playerMaxHp = 100;  // 玩家最大血量
    int shieldCount = 0;    // 护盾层数
    int invulnerableTicks = 0; // 无敌时间计数（受伤后的闪烁时间）

    // 武器属性
    int fireRate = 800;     // 射击间隔（毫秒）
    int bulletCount = 1;    // 每次射出的子弹数量
    int bulletDamage = 1;   // 每颗子弹的伤害
    bool hasPenetration = false; // 是否获得穿透技能
    bool hasIce = false;         // 是否获得冰冻技能
    QTime lastShootTime;    // 上次射击的时间记录

    QList<Skill> currentOptions; // 当前升级选单中的技能
};

#endif