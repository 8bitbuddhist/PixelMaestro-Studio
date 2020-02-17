#include <QApplication>
#include <QSettings>
#include "uiutility.h"

namespace PixelMaestroStudio {
	UIUtility::UIUtility() { }

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
