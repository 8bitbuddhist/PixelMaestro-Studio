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
	};
}

#endif // UIUTILITY_H
