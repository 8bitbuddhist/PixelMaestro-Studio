#include <QAbstractButton>
#include <QMessageBox>
#include "sectionmapdialog.h"
#include "ui_sectionmapdialog.h"
#include "widget/devicecontrolwidget.h"
#include "widget/maestrocontrolwidget.h"

namespace PixelMaestroStudio {
	SectionMapDialog::SectionMapDialog(SerialDeviceController& device, QWidget *parent) : QDialog(parent), ui(new Ui::SectionMapDialog), device_(device) {

		setWindowIcon(QIcon("qrc:/../../../docsrc/images/logo.png"));

		ui->setupUi(this);

		// If the model hasn't been initialized, initialize it
		if (device.section_map_model == nullptr) {
			MaestroControlWidget* mcw = dynamic_cast<MaestroControlWidget*>(parent->parentWidget()->parentWidget()->parentWidget()->parentWidget()->parentWidget());
			Maestro& maestro = mcw->get_maestro_controller()->get_maestro();
			device.section_map_model = new SectionMapModel();
			for (int i = 0; i < maestro.get_num_sections(); i++) {
				device.section_map_model->add_section();
			}
		}

		initialize();
	}

	void SectionMapDialog::initialize() {
		ui->mapTableView->setModel(device_.section_map_model);
		ui->mapTableView->resizeColumnsToContents();
		ui->mapTableView->resizeRowsToContents();
		ui->mapTableView->horizontalHeader()->stretchLastSection();
		ui->mapTableView->show();
	}

	void SectionMapDialog::on_buttonBox_clicked(QAbstractButton *button) {
		if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole) {
			// Trigger a save of all devices so we guarantee the device's model gets saved
			dynamic_cast<DeviceControlWidget*>(parentWidget()->parentWidget())->save_devices();
		}
		else if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole) {
			QMessageBox::StandardButton confirm;
			confirm = QMessageBox::question(this, "Clear Section Mappings", "Are you sure you want to clear your mappings and revert back to the defaults?", QMessageBox::Yes|QMessageBox::No);
			if (confirm == QMessageBox::Yes) {
				// Reinitialize the Section's model and reset the table view
				delete device_.section_map_model;
				MaestroControlWidget* mcw = dynamic_cast<MaestroControlWidget*>(parentWidget()->parentWidget()->parentWidget()->parentWidget()->parentWidget()->parentWidget());
				Maestro& maestro = mcw->get_maestro_controller()->get_maestro();
				device_.section_map_model = new SectionMapModel();
				for (int i = 0; i < maestro.get_num_sections(); i++) {
					device_.section_map_model->add_section();
				}
				initialize();
			}
		}
	}

	SectionMapDialog::~SectionMapDialog() {
		delete ui;
	}
}
