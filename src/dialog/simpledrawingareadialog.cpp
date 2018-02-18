#include "simpledrawingareadialog.h"
#include "ui_simpledrawingareadialog.h"
#include <QEvent>
#include <QKeyEvent>

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	SimpleDrawingAreaDialog::SimpleDrawingAreaDialog(QWidget *parent, MaestroController* maestro_controller) :
		QDialog(parent),
		ui(new Ui::SimpleDrawingAreaDialog) {
		// Capture button key presses
		qApp->installEventFilter(this);

		ui->setupUi(this);

		this->maestro_controller_ = maestro_controller;
		QLayout* layout = this->findChild<QLayout*>("maestroLayout");
		drawing_area_ = std::unique_ptr<SimpleDrawingArea>(new SimpleDrawingArea(layout->widget(), maestro_controller_));
		drawing_area_->set_maestro_control_widget((MaestroControlWidget*)parent);
		layout->addWidget(drawing_area_.get());

		this->setWindowFlags(Qt::Window);
	}

	/**
	 * Handle keypress events.
	 * @param watched Object that the keypress occurred in.
	 * @param event Keypress event.
	 * @return True on success.
	 */
	bool SimpleDrawingAreaDialog::eventFilter(QObject *watched, QEvent *event) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
			if (key_event->key() == Qt::Key_F11) {
				if (this->isFullScreen()) {
					this->showNormal();
					return true;
				}
				else {
					this->showFullScreen();
					return true;
				}
			}
		}

		return QObject::eventFilter(watched, event);
	}

	SimpleDrawingAreaDialog::~SimpleDrawingAreaDialog() {
		delete ui;
	}
}