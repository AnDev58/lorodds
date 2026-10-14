#include <qgraphicseffect.h>
#include <qtmetamacros.h>
class RotationEffect : public QGraphicsEffect {
  Q_OBJECT
public:
  explicit RotationEffect(qreal angle, QObject *parent = nullptr)
      : QGraphicsEffect(parent), angle(angle) {}

  void draw(QPainter *painter) override;

  QRectF boundingRectFor(const QRectF &rect) const override;

private:
  qreal angle;
};
