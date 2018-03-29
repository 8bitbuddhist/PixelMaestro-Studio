#include <QSerialPortInfo>
#include <QSettings>
#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

namespace PixelMaestroStudio {

	QString PreferencesDialog::last_cuefile_directory = QStringLiteral("File/LastCuefileDirectory");
	QString PreferencesDialog::separate_window_option = QStringLiteral("Displays/SeparateWindow");
	QString PreferencesDialog::main_window_option = QStringLiteral("Displays/MainWindow");
	QString PreferencesDialog::num_sections = QStringLiteral("Maestro/NumSections");
	QString PreferencesDialog::output_enabled = QStringLiteral("Enabled");
	QString PreferencesDialog::pause_on_start = QStringLiteral("Interface/PauseOnStart");
	QString PreferencesDialog::pixel_padding = QStringLiteral("Interface/Padding");
	QString PreferencesDialog::pixel_shape = QStringLiteral("Interface/Shape");
	QString PreferencesDialog::refresh_rate = QStringLiteral("Maestro/Refresh");
	QString PreferencesDialog::serial_ports = QStringLiteral("SerialPorts");
	QString PreferencesDialog::serial_port_name = QStringLiteral("Port");
	QString PreferencesDialog::serial_real_time_refresh = QStringLiteral("RealTimeRefresh");

	PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent), ui(new Ui::PreferencesDialog) {
		ui->setupUi(this);

		// Interface settings
		ui->paddingComboBox->setCurrentIndex(settings_.value(pixel_padding, 0).toInt());		// Default to no padding
		ui->pixelShapeComboBox->setCurrentIndex(settings_.value(pixel_shape, 1).toInt());		// Default to square pixels

		// Maestro settings
		ui->numSectionsSpinBox->setValue(settings_.value(num_sections, 1).toInt());				// Default to 1 Section
		ui->refreshSpinBox->setValue(settings_.value(refresh_rate, 50).toInt());				// Default to 50 ms
		ui->pauseOnStartCheckBox->setChecked(settings_.value(pause_on_start, false).toBool());	// Default to run on start

		// Check off activated displays
		ui->separateWindowCheckBox->setChecked(settings_.value(separate_window_option, false).toBool());
		ui->mainWindowCheckBox->setChecked(settings_.value(main_window_option, true).toBool());
	}

	void PreferencesDialog::on_buttonBox_accepted() {
		// Save Maestro settings
		settings_.setValue(refresh_rate, ui->refreshSpinBox->value());
		settings_.setValue(num_sections, ui->numSectionsSpinBox->value());
		settings_.setValue(pause_on_start, ui->pauseOnStartCheckBox->isChecked());

		// Save interface settings
		settings_.setValue(pixel_padding, ui->paddingComboBox->currentIndex());
		settings_.setValue(pixel_shape, ui->pixelShapeComboBox->currentIndex());

		// Save display settings
		settings_.setValue(separate_window_option, ui->separateWindowCheckBox->isChecked());
		settings_.setValue(main_window_option, ui->mainWindowCheckBox->isChecked());
	}

	PreferencesDialog::~PreferencesDialog() {
		delete ui;
	}
}
