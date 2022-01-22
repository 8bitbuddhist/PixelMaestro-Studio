#include <QAbstractButton>
#include <QCheckBox>
#include <QMessageBox>
#include <QSettings>
#include "sectionmapdialog.h"
#include "ui_sectionmapdialog.h"
#include "dialog/preferencesdialog.h"
#include "utility/uiutility.h"
#include "widget/devicecontrolwidget.h"
#include "widget/maestrocontrolwidget.h"

namespace PixelMaestroStudio {
	SectionMapDialog::SectionMapDialog(DeviceController& device, QWidget *parent) : QDialog(parent), ui(new Ui::SectionMapDialog), device_(device) {

		setWindowIcon(QIcon("qrc:/../../../docsrc/images/logo.png"));

		ui->setupUi(this);

		// If the model hasn't been initialized, initialize it
		if (device.section_map_model == nullptr) {
			MaestroControlWidget* mcw = dynamic_cast<MaestroControlWidget*>(parent->parentWidget()->parentWidget()->parentWidget()->parentWidget()->parentWidget()->parentWidget());
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
			int confirm = UIUtility::show_confirm_message_box(PreferencesDialog::msgbox_hide_clear_section_mappings, QString("Clear Section mappings"), QString("Are you sure you want to clear your mappings and revert back to the defaults?"), this);

			if (confirm == QMessageBox::Yes) {
				// Reinitialize the Section's model and reset the table view
				delete device_.section_map_model;

				device_.section_map_model = new SectionMapModel();
				device_.section_map_model->add_section();

				MaestroControlWidget* mcw = dynamic_cast<MaestroControlWidget*>(parentWidget()->parentWidget()->parentWidget()->parentWidget()->parentWidget()->parentWidget());

				if (mcw) {
					Maestro& maestro = mcw->get_maestro_controller()->get_maestro();
					device_.section_map_model = new SectionMapModel();
					for (int i = 1; i < maestro.get_num_sections(); i++) {
						device_.section_map_model->add_section();
					}
				}
				initialize();
			}
		}
	}

	SectionMapDialog::~SectionMapDialog() {
		delete ui;
	}
}
