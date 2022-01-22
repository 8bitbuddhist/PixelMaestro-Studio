#ifndef UIUTILITY_H
#define UIUTILITY_H

#include <QPixmap>
#include <QWidget>
#include "controller/palettecontroller.h"

namespace PixelMaestroStudio {
	class UIUtility {

		public:
			UIUtility();

			static QSharedPointer<QPixmap> generate_palette_thumbnail(PaletteController::PaletteWrapper& palette);
			static void highlight_widget(QWidget* button, bool highlight);
			static int show_confirm_message_box(QString preferencesdialog_msgbox, QString title, QString text, QWidget* parent = nullptr);
	};
}

#endif // UIUTILITY_H
