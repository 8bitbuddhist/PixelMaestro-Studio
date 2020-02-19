#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QComboBox>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QSettings>
#include "uiutility.h"

namespace PixelMaestroStudio {
	UIUtility::UIUtility() { }

	QSharedPointer<QPixmap> UIUtility::generate_palette_thumbnail(PaletteController::PaletteWrapper& palette) {
		QSharedPointer<QPixmap> thumbnail = QSharedPointer<QPixmap>(new QPixmap(100, 20));
		QPainter painter(thumbnail.data());
		painter.setRenderHint(QPainter::Antialiasing);

		int i = 0;
		while (i < 5 || i < palette.palette.get_num_colors()) {
			Colors::RGB rgb = palette.palette.get_color_at_index(i);
			// Generate thumbnail
			QColor qcolor;
			QBrush brush;
			QRect rect;

			qcolor.setRgb(rgb.r, rgb.g, rgb.b);
			brush.setColor(qcolor);
			brush.setStyle(Qt::BrushStyle::SolidPattern);

			rect.setRect(i * 20, 0, 20, 20);
			painter.setBrush(brush);
			painter.setPen(Qt::PenStyle::NoPen);

			painter.drawRect(rect);

			++i;
		}

		return thumbnail;
	}

	void UIUtility::highlight_widget(QWidget* widget, bool highlight) {
		if (highlight) {
			QColor highlight_color = qApp->palette().highlight().color();
			widget->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(highlight_color.red()).arg(highlight_color.green()).arg(highlight_color.blue()));
		}
		else {
			widget->setStyleSheet(QString());
		}
	}
}
