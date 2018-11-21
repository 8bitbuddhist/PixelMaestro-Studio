#include "maestrodrawingareadialog.h"
#include "ui_maestrodrawingareadialog.h"
#include <QEvent>
#include <QKeyEvent>

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	MaestroDrawingAreaDialog::MaestroDrawingAreaDialog(QWidget *parent, MaestroController* maestro_controller) :
		QDialog(parent),
		ui(new Ui::MaestroDrawingAreaDialog) {
		// Capture button key presses
		qApp->installEventFilter(this);

		ui->setupUi(this);

		this->maestro_controller_ = maestro_controller;
		QLayout* layout = this->findChild<QLayout*>("maestroLayout");
		drawing_area_ = QSharedPointer<MaestroDrawingArea>(new MaestroDrawingArea(layout->widget(), maestro_controller_));
		drawing_area_->set_maestro_control_widget(dynamic_cast<MaestroControlWidget*>(parent));
		layout->addWidget(drawing_area_.data());

		this->setWindowFlags(Qt::Window);
	}

	/**
	 * Handle keypress events.
	 * @param watched Object that the keypress occurred in.
	 * @param event Keypress event.
	 * @return True on success.
	 */
	bool MaestroDrawingAreaDialog::eventFilter(QObject *watched, QEvent *event) {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* key_event = dynamic_cast<QKeyEvent*>(event);
			if (key_event->key() == Qt::Key_F11) {
				if (this->isFullScreen()) {
					this->showNormal();
				}
				else {
					this->showFullScreen();
				}
				return true;
			}
		}

		return QObject::eventFilter(watched, event);
	}

	/**
	 * Returns the Dialog's drawing area.
	 * @return Drawing area.
	 */
	MaestroDrawingArea* MaestroDrawingAreaDialog::get_maestro_drawing_area() {
		return this->drawing_area_.get();
	}

	MaestroDrawingAreaDialog::~MaestroDrawingAreaDialog() {
		delete ui;
	}
}
