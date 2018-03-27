#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QSettings>

namespace Ui {
	class PreferencesDialog;
}

namespace PixelMaestroStudio {
	class PreferencesDialog : public QDialog {
			Q_OBJECT

		public:
			static QString separate_window_option;
			static QString main_window_option;
			static QString num_sections;
			static QString output_enabled;
			static QString pause_on_start;
			static QString pixel_padding;
			static QString pixel_shape;
			static QString refresh_rate;
			static QString serial_port;
			static QString serial_ports;

			explicit PreferencesDialog(QWidget *parent = 0);
			~PreferencesDialog();

		private slots:
			void on_buttonBox_accepted();

		private:
			QSettings settings_;
			Ui::PreferencesDialog *ui;
	};
}

#endif // SETTINGSDIALOG_H