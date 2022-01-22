#include <QApplication>
#include <QBrush>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QImage>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QSettings>
#include "uiutility.h"
#include "dialog/preferencesdialog.h"

namespace PixelMaestroStudio {
	UIUtility::UIUtility() { }

	/**
	 * Generates a small thumbnail preview of a Palette. Used as an icon in ComboBoxes.
	 * @param palette The Palette to generate a thumbnail for.
	 * @return Thumbnail pixmap.
	 */
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

	/**
	 * Highlights the QWidget using the theme's highlight color.
	 * @param widget Widget to highlight.
	 * @param highlight Whether to add or remove the highlight.
	 */
	void UIUtility::highlight_widget(QWidget* widget, bool highlight) {
		if (highlight) {
			QColor highlight_color = qApp->palette().highlight().color();
			widget->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(highlight_color.red()).arg(highlight_color.green()).arg(highlight_color.blue()));
		}
		else {
			widget->setStyleSheet(QString());
		}
	}

	int UIUtility::show_confirm_message_box(QString preferencesdialog_msgbox, QString title, QString text, QWidget* parent) {
		QSettings settings;
		bool hide_msgbox = settings.value(preferencesdialog_msgbox, false).toBool();

		if (!hide_msgbox) {
			QCheckBox* do_not_show = new QCheckBox("Don't show this message again");
			QMessageBox msg_box(QMessageBox::Icon::Question, title, text, QMessageBox::Yes | QMessageBox::No, parent);
			msg_box.setDefaultButton(QMessageBox::Yes);
			msg_box.setCheckBox(do_not_show);

			QObject::connect(do_not_show, &QCheckBox::stateChanged, [&](){
				settings.setValue(preferencesdialog_msgbox, do_not_show->isChecked());
			});

			return msg_box.exec();
		}
		else {
			return QMessageBox::Yes;
		}
	}
}
