#include <QSerialPortInfo>
#include <QSettings>
#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

namespace PixelMaestroStudio {

	QString PreferencesDialog::delimiter = QStringLiteral("|");
	QString PreferencesDialog::sub_delimiter = QStringLiteral(",");

	// "Files" section
	QString PreferencesDialog::last_cuefile_directory = QStringLiteral("Files/LastCuefileDirectory");

	// "Displays" section
	QString PreferencesDialog::separate_window_option = QStringLiteral("Displays/SeparateWindow");
	QString PreferencesDialog::main_window_option = QStringLiteral("Displays/MainWindow");

	// "Maestro" section
	QString PreferencesDialog::num_sections = QStringLiteral("Maestro/NumSections");
	QString PreferencesDialog::refresh_rate = QStringLiteral("Maestro/Refresh");
	QString PreferencesDialog::output_enabled = QStringLiteral("Enabled");

	// "Interface" section
	QString PreferencesDialog::pause_on_start = QStringLiteral("Interface/PauseOnStart");
	QString PreferencesDialog::pixel_padding = QStringLiteral("Interface/Padding");
	QString PreferencesDialog::pixel_shape = QStringLiteral("Interface/Shape");
	QString PreferencesDialog::save_session = QStringLiteral("Interface/SaveSessionOnClose");
	QString PreferencesDialog::last_session = QStringLiteral("Interface/LastSession");

	// "Devices" section
	QString PreferencesDialog::devices = QStringLiteral("Devices");
	QString PreferencesDialog::device_capacity = QStringLiteral("Capacity");
	QString PreferencesDialog::device_port = QStringLiteral("Port");
	QString PreferencesDialog::device_real_time_refresh = QStringLiteral("RealTimeRefresh");

	// "Palettes" section
	QString PreferencesDialog::palettes = QStringLiteral("Palettes");
	QString PreferencesDialog::palette_base_color = QStringLiteral("BaseColor");
	QString PreferencesDialog::palette_colors = QStringLiteral("Colors");
	QString PreferencesDialog::palette_mirror = QStringLiteral("Mirror");
	QString PreferencesDialog::palette_name = QStringLiteral("Name");
	QString PreferencesDialog::palette_num_colors = QStringLiteral("ColorCount");
	QString PreferencesDialog::palette_target_color = QStringLiteral("TargetColor");
	QString PreferencesDialog::palette_type = QStringLiteral("Type");

	PreferencesDialog::PreferencesDialog(QWidget *parent) : QDialog(parent), ui(new Ui::PreferencesDialog) {
		ui->setupUi(this);

		// Interface settings
		ui->paddingComboBox->setCurrentIndex(settings_.value(pixel_padding, 0).toInt());		// Default to no padding
		ui->pixelShapeComboBox->setCurrentIndex(settings_.value(pixel_shape, 1).toInt());		// Default to square pixels
		ui->saveSessionCheckBox->setChecked(settings_.value(save_session, false).toBool());		// Default to new session

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
		settings_.setValue(save_session, ui->saveSessionCheckBox->isChecked());

		// Save display settings
		settings_.setValue(separate_window_option, ui->separateWindowCheckBox->isChecked());
		settings_.setValue(main_window_option, ui->mainWindowCheckBox->isChecked());
	}

	PreferencesDialog::~PreferencesDialog() {
		delete ui;
	}
}
