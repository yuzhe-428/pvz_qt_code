#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <QPointF>
#include <QVector2D>
#include <QSet>
#include <QString>

// 基础类：所有游戏物体（僵尸、子弹）的父类
class GameObject {
public:
    QPointF pos;       // 物体在屏幕上的坐标 (x, y)
    int hp;            // 当前生命值
    int maxHp;         // 最大生命值

    // 纯虚函数：要求子类必须实现自己的移动逻辑
    virtual void move() = 0;

    // 虚析构函数：确保子类对象被正确销毁，防止内存泄漏
    virtual ~GameObject() {}
};

// 技能结构体：用于升级时显示的技能信息
struct Skill {
    QString name;      // 技能名称
    QString desc;      // 技能描述
    int type;          // 技能类型标识（用于逻辑分支判断）
};

#endif