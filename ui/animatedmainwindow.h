#ifndef ANIMATEDMAINWINDOW_H
#define ANIMATEDMAINWINDOW_H

#include"mainwindow.h"
#include <QPropertyAnimation>          // 添加这行
#include <QParallelAnimationGroup>     // 添加这行
#include <QGraphicsOpacityEffect>      // 添加这行
#include <QEasingCurve>               // 添加这行
#include <QTimer>
#include <QApplication>
class AnimatedMainWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void minimizeWindow()
    {
        if (m_animationGroup && m_animationGroup->state() == QAbstractAnimation::Running) {
            return;
        }

        // 创建缩放和淡出效果
        QParallelAnimationGroup *minimizeGroup = new QParallelAnimationGroup(this);

        // 缩放动画
        QPropertyAnimation *scaleAnimation = new QPropertyAnimation(this, "geometry");
        scaleAnimation->setDuration(400);
        scaleAnimation->setEasingCurve(QEasingCurve::InBack);

        QRect startRect = geometry();
        QRect endRect = QRect(startRect.center().x() - 10, startRect.center().y() - 10, 20, 20);
        scaleAnimation->setStartValue(startRect);
        scaleAnimation->setEndValue(endRect);

        // 透明度动画
        QPropertyAnimation *opacityAnimation = new QPropertyAnimation(this, "windowOpacity");
        opacityAnimation->setDuration(400);
        opacityAnimation->setStartValue(1.0);
        opacityAnimation->setEndValue(0.3);

        minimizeGroup->addAnimation(scaleAnimation);
        minimizeGroup->addAnimation(opacityAnimation);

        connect(minimizeGroup, &QParallelAnimationGroup::finished, this, [this]() {
            QMainWindow::showMinimized();
            // 恢复原状
            setWindowOpacity(1.0);
        });

        minimizeGroup->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void toggleFullscreen()
    {
        if (m_animationGroup && m_animationGroup->state() == QAbstractAnimation::Running) {
            return;
        }

        if (isFullScreen()) {
            // 退出全屏 - 缩放效果
            showNormal();

            QTimer::singleShot(50, this, [this]() {
                QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
                animation->setDuration(500);
                animation->setEasingCurve(QEasingCurve::OutBack);

                QRect screen = QApplication::primaryScreen()->geometry();
                animation->setStartValue(QRect(0, 0, screen.width(), screen.height()));
                animation->setEndValue(m_normalGeometry);

                connect(animation, &QPropertyAnimation::finished, this, [this]() {
                    ui->fullscreenButton->setIcon(QIcon(":/img/full-screen.svg"));
                });

                animation->start(QAbstractAnimation::DeleteWhenStopped);
            });
        } else {
            // 进入全屏 - 放大效果
            m_normalGeometry = geometry();

            QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
            animation->setDuration(500);
            animation->setEasingCurve(QEasingCurve::InOutQuad);

            QRect screen = QApplication::primaryScreen()->geometry();
            animation->setStartValue(geometry());
            animation->setEndValue(QRect(0, 0, screen.width(), screen.height()));

            connect(animation, &QPropertyAnimation::finished, this, [this]() {
                showFullScreen();
                ui->fullscreenButton->setIcon(QIcon(":/img/copy-document.svg"));
            });

            animation->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }

    void closeWindow()
    {
        if (m_animationGroup && m_animationGroup->state() == QAbstractAnimation::Running) {
            return;
        }

        // 创建渐隐和缩放关闭效果
        QParallelAnimationGroup *closeGroup = new QParallelAnimationGroup(this);

        // 渐隐动画
        QPropertyAnimation *fadeAnimation = new QPropertyAnimation(this, "windowOpacity");
        fadeAnimation->setDuration(350);
        fadeAnimation->setStartValue(1.0);
        fadeAnimation->setEndValue(0.0);

        // 轻微缩放动画
        QPropertyAnimation *scaleAnimation = new QPropertyAnimation(this, "geometry");
        scaleAnimation->setDuration(350);
        scaleAnimation->setEasingCurve(QEasingCurve::InCubic);

        QRect startRect = geometry();
        QRect endRect = startRect.adjusted(20, 20, -20, -20);
        scaleAnimation->setStartValue(startRect);
        scaleAnimation->setEndValue(endRect);

        closeGroup->addAnimation(fadeAnimation);
        closeGroup->addAnimation(scaleAnimation);

        connect(closeGroup, &QParallelAnimationGroup::finished, this, &QWidget::close);
        closeGroup->start(QAbstractAnimation::DeleteWhenStopped);
    }

private:
    QRect m_normalGeometry;
    QAnimationGroup *m_animationGroup = nullptr;
};
#endif // ANIMATEDMAINWINDOW_H
