#include "rotator.h"
#include <qnamespace.h>
#include <qpaintdevice.h>
#include <qpainter.h>

void RotationEffect::draw(QPainter *painter) {
  QPixmap pixmap;
  QPoint offset;

  pixmap = sourcePixmap(Qt::LogicalCoordinates, &offset);
  painter->save();

  painter->setRenderHint(QPainter::SmoothPixmapTransform);

  qreal cx = offset.x() + pixmap.width() / 2.0;
  qreal cy = offset.y() + pixmap.height() / 2.0;

  painter->translate(cx, cy);
  painter->rotate(angle);
  painter->translate(-cx, -cy);

  painter->drawPixmap(offset, pixmap);
  painter->restore();
}

QRectF RotationEffect::boundingRectFor(const QRectF &rect) const {
  QPolygonF polygon(rect);
  QTransform transform;

  transform.translate(rect.width() / 2.0, rect.height() / 2.0);
  transform.rotate(angle);
  transform.translate(-rect.width() / 2.0, -rect.height() / 2.0);

  return transform.map(polygon).boundingRect();
}
