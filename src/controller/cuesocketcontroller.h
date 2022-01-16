#ifndef CUESOCKETCONTROLLER_H
#define CUESOCKETCONTROLLER_H

#include <QObject>
#include <QTcpServer>
#include "../widget/maestrocontrolwidget.h"

namespace PixelMaestroStudio {
	class CueServerController : public QObject {
		Q_OBJECT

		public:
			CueServerController(QWidget* parent, MaestroControlWidget& maestro_control_widget);

		public slots:
			void new_connection();
			void read_data();

		private:
			MaestroControlWidget* maestro_control_widget_ = nullptr;
			QSharedPointer<QTcpServer> tcp_server_;
	};
}

#endif // CUESOCKETCONTROLLER_H
