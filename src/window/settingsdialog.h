#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QSettings>

namespace Ui {
	class SettingsDialog;
}

class SettingsDialog : public QDialog
{
		Q_OBJECT

	public:
		static QString num_sections;
		static QString pixel_padding;
		static QString pixel_shape;
		static QString output_devices;
		static QString output_enabled;
		static QString output_name;
		static QString refresh_rate;
		static QString screen_option;
		static QString serial_enabled;
		static QString serial_port;
		static QString virtual_device_option;
		static QString virtual_device_width;
		static QString virtual_device_height;

		explicit SettingsDialog(QWidget *parent = 0);
		~SettingsDialog();

	private slots:
		void on_buttonBox_accepted();
		void on_outputListWidget_itemChanged(QListWidgetItem *item);

	private:
		QSettings settings_;
		Ui::SettingsDialog *ui;

		void set_simulated_device_options_enabled(bool output_enabled);
};

#endif // SETTINGSDIALOG_H
