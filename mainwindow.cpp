#include "mainwindow.h"
#include <QApplication>
#include <algorithm>
#include <random>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setFixedSize(800, 600);
    setWindowTitle("植物大战僵尸：肉鸽幸存者 - 标准多文件版");
    playerPos = QPointF(100, 300);
    lastShootTime = QTime::currentTime();

    // 初始化定时器，约 60 FPS (16ms 一帧)
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::gameLoop);
    timer->start(16);
}

MainWindow::~MainWindow() {
    qDeleteAll(zombies); // 清理内存
    qDeleteAll(bullets);
}

void MainWindow::gameOver() {
    isGameOver = true;
    timer->stop();
    update();
}

// 处理按键：包括玩家移动和升级技能选择
void MainWindow::keyPressEvent(QKeyEvent *event) {
    // 如果处于暂停选技能状态
    if(isPaused) {
        int choice = -1;
        if(event->key() == Qt::Key_1) choice = 0;
        else if(event->key() == Qt::Key_2) choice = 1;
        else if(event->key() == Qt::Key_3) choice = 2;

        if(choice >= 0 && choice < currentOptions.size()) {
            Skill s = currentOptions[choice];
            // 应用技能效果
            if(s.type == 0) fireRate = qMax(80, fireRate - 120); // 提升射速
            else if(s.type == 1) bulletCount++;                // 增加子弹数
            else if(s.type == 2) bulletDamage += 1;            // 增加伤害
            else if(s.type == 3) playerHp = qMin(playerMaxHp, playerHp + 40); // 回血
            else if(s.type == 4) hasPenetration = true;        // 获得穿透
            else if(s.type == 5) hasIce = true;                // 获得冰冻
            else if(s.type == 6) shieldCount += 3;             // 增加护盾

            isPaused = false;
            timer->start(); // 继续游戏
        }
        return;
    }

    // 游戏结束按空格退出
    if(isGameOver && event->key() == Qt::Key_Space) qApp->exit();

    // 玩家上下移动逻辑
    float moveStep = 30.0f;
    if(event->key() == Qt::Key_W && playerPos.y() > 50) playerPos.ry() -= moveStep;
    if(event->key() == Qt::Key_S && playerPos.y() < 550) playerPos.ry() += moveStep;
}

// 僵尸生成逻辑
void MainWindow::spawnZombie(bool boss) {
    // 限制普通僵尸数量随等级增加
    if(!boss && zombies.size() >= (5 + level * 2)) return;

    Zombie* z = new Zombie();
    z->isBoss = boss;
    // Boss 固定在中间，普通僵尸随机高度
    z->pos = QPointF(850, boss ? 300 : QRandomGenerator::global()->bounded(50, 550));
    z->isFast = !boss && (QRandomGenerator::global()->bounded(10) > 8);
    // 动态调整血量
    z->maxHp = boss ? (30 + level * 5) : (2 + level / 2);
    z->hp = z->maxHp;
    zombies.append(z);
}

// 升级菜单：随机抽取3个技能
void MainWindow::showLevelUpMenu() {
    isPaused = true;
    timer->stop();
    currentOptions.clear();

    // 技能池
    QList<Skill> pool = {
        {"神速豆", "射速大幅提升", 0}, {"多重影", "子弹数量+1", 1},
        {"肌肉豌豆", "子弹伤害+1", 2}, {"治愈果实", "回复40点生命", 3},
        {"防御墙", "获得3层防御盾", 6}
    };
    // 排除已拥有的唯一技能
    if(!hasPenetration) pool.append({"穿透箭", "子弹可穿透敌人", 4});
    if(!hasIce) pool.append({"冰冻气息", "击中可减速敌人", 5});

    // 随机打乱并取前三个
    std::shuffle(pool.begin(), pool.end(), std::default_random_engine(QRandomGenerator::global()->generate()));
    for(int i=0; i<qMin(3, (int)pool.size()); i++) currentOptions.append(pool[i]);
    update();
}

// 核心游戏主循环
void MainWindow::gameLoop() {
    if(isPaused || isGameOver) return;
    if(invulnerableTicks > 0) invulnerableTicks--; // 更新无敌时间

    // 1. 发射逻辑：检查冷却时间和屏幕子弹上限
    if(bullets.size() < 120 && lastShootTime.msecsTo(QTime::currentTime()) > fireRate) {
        for(int i=0; i<bulletCount; ++i) {
            Bullet* b = new Bullet();
            b->pos = playerPos + QPointF(30, 0);
            b->damage = bulletDamage;
            b->isPenetrating = hasPenetration;
            b->isIce = hasIce;
            // 扇形射击角度计算
            float angle = (i - (bulletCount-1)/2.0f) * 0.15f;
            b->velocity = QVector2D(9.5f, angle * 4.0f);
            bullets.append(b);
        }
        lastShootTime = QTime::currentTime();
    }

    // 2. 移动物体
    for(auto z : zombies) z->move();
    for(auto b : bullets) b->move();

    // 3. 碰撞判定（子弹 vs 僵尸）
    for(int i = bullets.size() - 1; i >= 0; --i) {
        Bullet* b = bullets[i];
        bool bulletDestroyed = false;
        // 移除越界子弹
        if(b->pos.x() > 850 || b->pos.x() < -50) { delete bullets.takeAt(i); continue; }

        for(int j = zombies.size() - 1; j >= 0; --j) {
            Zombie* z = zombies[j];
            float hitRange = z->isBoss ? 60 : 40; // 根据体型判断碰撞范围

            // 距离检测判断是否击中
            if(QLineF(b->pos, z->pos).length() < hitRange) {
                // 如果是穿透子弹且已打过该僵尸，则跳过
                if(b->isPenetrating && b->hitZombies.contains(z)) continue;

                z->hp -= b->damage;
                if(b->isIce) z->speedMultiplier = 0.4f; // 冰冻减速

                if(b->isPenetrating) b->hitZombies.insert(z);
                else { delete bullets.takeAt(i); bulletDestroyed = true; } // 非穿透子弹消失

                // 僵尸死亡处理
                if(z->hp <= 0) {
                    bool wasBoss = z->isBoss;
                    delete zombies.takeAt(j);
                    score += wasBoss ? 100 : 10;
                    exp += wasBoss ? 5 : 1;

                    // 升级判断
                    if(exp >= expToNextLevel) {
                        level++; exp = 0; expToNextLevel += 5;
                        if(level % 5 == 0) spawnZombie(true); // 每5级出一次Boss
                        showLevelUpMenu(); return;
                    }
                }
                if(bulletDestroyed) break;
            }
        }
    }

    // 4. 玩家受伤判定（僵尸 vs 玩家）
    for(int i = zombies.size() - 1; i >= 0; --i) {
        float playerHitRange = zombies[i]->isBoss ? 70 : 45;
        if(QLineF(zombies[i]->pos, playerPos).length() < playerHitRange && invulnerableTicks == 0) {
            if(shieldCount > 0) {
                shieldCount--; // 优先消耗护盾
                delete zombies.takeAt(i);
            }
            else {
                playerHp -= 20; // 扣血
                invulnerableTicks = 45; // 开启短暂无敌
            }
        } else if(zombies[i]->pos.x() < -50) {
            delete zombies.takeAt(i); // 僵尸走到屏幕左侧外移除
        }
        if(playerHp <= 0) { gameOver(); return; }
    }

    // 随机生成僵尸
    if(QRandomGenerator::global()->bounded(100) < qMin(15, 3 + level)) spawnZombie();
    update(); // 触发重绘
}

// 绘图逻辑
void MainWindow::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    p.drawPixmap(rect(), QPixmap(":/images/background.png"));

    // 绘制玩家（无敌状态下会闪烁）
    if(invulnerableTicks % 2 == 0)
        p.drawPixmap(playerPos.x()-30, playerPos.y()-30, 60, 60, QPixmap(":/images/plant.png"));

    // 绘制玩家血条
    p.setBrush(Qt::black); p.drawRect(playerPos.x()-30, playerPos.y()-45, 60, 5);
    p.setBrush(Qt::green); p.drawRect(playerPos.x()-30, playerPos.y()-45, 60 * (playerHp/(float)playerMaxHp), 5);

    // 绘制护盾特效
    if(shieldCount > 0) {
        p.setPen(QPen(Qt::cyan, 3, Qt::DotLine));
        p.drawEllipse(playerPos, 45, 45);
    }

    // 遍历绘制僵尸及其血条
    for(auto z : zombies) {
        int size = z->isBoss ? 140 : 70;
        p.drawPixmap(z->pos.x() - size/2, z->pos.y() - size/2, size, size, QPixmap(":/images/zombie.png"));
        float hr = qMax(0.0f, (float)z->hp / z->maxHp);
        p.fillRect(z->pos.x()-size/4, z->pos.y() - size/2 - 10, size/2, 6, Qt::black);
        p.fillRect(z->pos.x()-size/4, z->pos.y() - size/2 - 10, (size/2) * hr, 6, z->isBoss ? Qt::magenta : Qt::red);
    }

    // 绘制子弹
    for(auto b : bullets)
        p.drawPixmap(b->pos.x()-10, b->pos.y()-10, 20, 20, QPixmap(":/images/bullet.png"));

    // 绘制顶栏 UI 信息
    p.setPen(Qt::white);
    p.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    p.drawText(20, 30, QString("等级: %1  经验: %2/%3  分数: %4").arg(level).arg(exp).arg(expToNextLevel).arg(score));

    // 绘制暂停/结束遮罩层
    if(isPaused || isGameOver) {
        p.fillRect(rect(), QColor(0, 0, 0, 180));
        p.setPen(Qt::yellow);
        p.setFont(QFont("Arial", 25, QFont::Bold));
        if(isGameOver) {
            p.drawText(rect(), Qt::AlignCenter, "GAME OVER\nScore: " + QString::number(score));
        } else {
            p.drawText(rect(), Qt::AlignTop | Qt::AlignHCenter, "\n\nUPGRADE!");
            // 绘制升级选项列表
            for(int i=0; i<currentOptions.size(); i++)
                p.drawText(250, 250 + i*60, QString("[%1] %2: %3").arg(i+1).arg(currentOptions[i].name).arg(currentOptions[i].desc));
        }
    }
}