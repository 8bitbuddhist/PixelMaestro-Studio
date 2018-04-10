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
			static QString last_cuefile_directory;
			static QString main_window_option;
			static QString num_sections;
			static QString output_enabled;
			static QString pause_on_start;
			static QString pixel_padding;
			static QString pixel_shape;
			static QString refresh_rate;
			static QString save_session;
			static QString separate_window_option;
			static QString device_capacity;
			static QString device_port;
			static QString devices;
			static QString device_real_time_refresh;

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