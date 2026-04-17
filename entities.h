#ifndef ENTITIES_H
#define ENTITIES_H

#include "gameobject.h"

// 僵尸类：继承自 GameObject
class Zombie : public GameObject {
public:
    bool isFast;             // 是否为快速移动类型
    bool isBoss = false;     // 是否为 Boss
    float speedMultiplier = 1.0f; // 速度倍率（受冰冻效果影响）
    void move() override;    // 实现具体的移动逻辑
};

// 子弹类：继承自 GameObject
class Bullet : public GameObject {
public:
    int damage;              // 子弹伤害值
    bool isPenetrating = false; // 是否具备穿透属性
    bool isIce = false;      // 是否具备冰冻（减速）效果
    QSet<Zombie*> hitZombies; // 记录已击中的僵尸（用于穿透逻辑，避免单颗子弹对同一目标重复伤害）
    QVector2D velocity;      // 子弹的速度矢量（包含方向和大小）
    void move() override;    // 实现具体的移动逻辑
};

#endif