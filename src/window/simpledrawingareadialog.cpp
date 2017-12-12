#include "simpledrawingareadialog.h"
#include "ui_simpledrawingareadialog.h"

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	SimpleDrawingAreaDialog::SimpleDrawingAreaDialog(QWidget *parent, MaestroController* maestro_controller) :
		QDialog(parent),
		ui(new Ui::SimpleDrawingAreaDialog) {
		ui->setupUi(this);

		this->maestro_controller_ = maestro_controller;
		QLayout* layout = this->findChild<QLayout*>("maestroLayout");
		drawing_area_ = std::unique_ptr<SimpleDrawingArea>(new SimpleDrawingArea(layout->widget(), maestro_controller_));
		layout->addWidget(drawing_area_.get());
	}

	SimpleDrawingAreaDialog::~SimpleDrawingAreaDialog() {
		delete ui;
	}
}