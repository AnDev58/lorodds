#include "main_window.h"
#include "rotator.h"
#include "ui_main_window.h"
#include <QGraphicsRotation>
#include <memory>
#include <qnamespace.h>
#include <qpixmap.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(std::make_unique<Ui::MainWindow>()) {
  ui->setupUi(this);

  ui->page_bg->lower();
  createPageBacking();

  rotateElements();

  auto *logoShadow = new QGraphicsDropShadowEffect(ui->page_logo);
  logoShadow->setBlurRadius(15);
  logoShadow->setColor(QColor(255, 0, 0, 150));
  logoShadow->setOffset(0, 0);
  ui->page_logo->setGraphicsEffect(logoShadow);
}

void MainWindow::createPageBacking() {
  QFrame *black_backing = new QFrame(ui->frame);

  black_backing->setGeometry(ui->page_bg->geometry());
  black_backing->setStyleSheet("background-color: #000000; border: none;");
  black_backing->lower();
}

void MainWindow::rotateElements() {
  QPixmap original = ui->page_logo->pixmap();
  if (!original.isNull()) {
    QPixmap scaled{original.scaled(ui->page_logo->size(), Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation)};
    QTransform transform;
    transform.rotate(kPageTiltAngle);
    QPixmap rotated = scaled.transformed(transform, Qt::SmoothTransformation);
    ui->page_logo->setFixedSize(rotated.size());
    ui->page_logo->setPixmap(rotated);
  }

  ui->dice_queue_small->setGraphicsEffect(
      new RotationEffect(kPageTiltAngle, this));
  ui->page_name->setGraphicsEffect(new RotationEffect(kPageTiltAngle, this));
}

MainWindow::~MainWindow() = default;
