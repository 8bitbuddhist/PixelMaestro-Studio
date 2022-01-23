#include "cueservercontroller.h"
#include "../dialog/preferencesdialog.h"
#include "../lib/PixelMaestro/src/cue/cuecontroller.h"
#include <QHostAddress>
#include <QMessageBox>
#include <QSettings>
#include <QTcpSocket>

namespace PixelMaestroStudio {
	CueServerController::CueServerController(QWidget* parent, MaestroControlWidget& maestro_control_widget) {
		this->maestro_control_widget_ = &maestro_control_widget;

		tcp_server_ = QSharedPointer<QTcpServer>(new QTcpServer(this));
		connect(tcp_server_.data(), &QTcpServer::newConnection, this, &CueServerController::new_connection);

		QSettings settings;
		if (!tcp_server_->listen(QHostAddress::Any, settings.value(PreferencesDialog::cue_server_port).toInt())) {
			QMessageBox::critical(parent, tr("Cue Server"),
								  tr("Unable to start the Cue server. PixelMaestro Studio will not be able to read Cues sent to this device. Error code: %1.")
								  .arg(tcp_server_->errorString()));
			return;
		}
	}

	void CueServerController::new_connection() {
		QTcpSocket* newSocket = tcp_server_->nextPendingConnection();
			if (newSocket) {
				connect(newSocket, &QTcpSocket::readyRead, this, &CueServerController::read_data);

				// Clean up the connection once it's disconnected
				connect(newSocket, &QTcpSocket::disconnected, newSocket, &QTcpSocket::deleteLater);
			}
	}

	void CueServerController::read_data() {
		// No point in continuing if nothing can read the cue
		CueController* controller = &maestro_control_widget_->get_maestro_controller()->get_maestro().get_cue_controller();
		if (controller == nullptr) return;

		QTcpSocket * senderSocket = dynamic_cast<QTcpSocket*>(sender());
			if (senderSocket) {
				bool updated = false;
				while (senderSocket->bytesAvailable()) {
					QByteArray data = senderSocket->readAll();
					for (uint8_t byte : data) {
						updated = controller->read(byte);
					}
				}
				maestro_control_widget_->set_maestro_modified(updated);
				maestro_control_widget_->set_refresh_needed(updated);
			}
	}
}