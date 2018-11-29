#include <QSettings>
#include "sectionmapdialog.h"
#include "ui_sectionmapdialog.h"
#include "widget/devicecontrolwidget.h"

namespace PixelMaestroStudio {
	SectionMapDialog::SectionMapDialog(SerialDeviceController* device, QWidget *parent) : QDialog(parent), ui(new Ui::SectionMapDialog) {
		ui->setupUi(this);

		// If the model hasn't been initialized, initialize it
		if (device->section_map_model == nullptr) {
			MaestroControlWidget* mcw = dynamic_cast<MaestroControlWidget*>(parent->parentWidget()->parentWidget()->parentWidget()->parentWidget());
			Maestro* maestro = mcw->get_maestro_controller()->get_maestro();
			device->section_map_model = new SectionMapModel(maestro);
		}

		ui->mapTableView->setModel(device->section_map_model);
		ui->mapTableView->resizeColumnToContents(0);
		ui->mapTableView->show();

		this->model_ = device->section_map_model;
	}

	void SectionMapDialog::on_addSectionButton_clicked() {
		model_->add_section();

		ui->addSectionButton->setEnabled(model_->rowCount() != UINT8_MAX);
		ui->removeSectionButton->setEnabled(model_->rowCount() > 0);
	}

	void SectionMapDialog::on_buttonBox_accepted() {
		// Trigger a save of all devices so we guarantee the device's model gets saved
		dynamic_cast<DeviceControlWidget*>(parentWidget())->save_devices();
	}

	void SectionMapDialog::on_removeSectionButton_clicked() {
		model_->remove_section();

		ui->addSectionButton->setEnabled(model_->rowCount() != UINT8_MAX);
		ui->removeSectionButton->setEnabled(model_->rowCount() > 0);
	}

	SectionMapDialog::~SectionMapDialog() {
		delete ui;
	}
}
