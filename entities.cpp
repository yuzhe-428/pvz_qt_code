#include "entities.h"

// 僵尸的移动实现
void Zombie::move() {
    // 根据类型确定基础速度：Boss慢，快速僵尸快，普通居中
    float baseSpeed = isBoss ? 0.8f : (isFast ? 3.2f : 1.6f);
    // 坐标向左移动（僵尸从右往左进攻）
    pos.rx() -= baseSpeed * speedMultiplier;
}

// 子弹的移动实现
void Bullet::move() {
    // 根据速度矢量更新当前位置
    pos += velocity.toPointF();
}