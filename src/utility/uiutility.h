#ifndef UIUTILITY_H
#define UIUTILITY_H

#include <QWidget>

namespace PixelMaestroStudio {
	class UIUtility {

		public:
			UIUtility();

			static void highlight_widget(QWidget* button, bool highlight);
	};
}

#endif // UIUTILITY_H
