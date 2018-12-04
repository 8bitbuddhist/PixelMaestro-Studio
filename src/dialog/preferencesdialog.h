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
			static QString delimiter;
			static QString sub_delimiter;

			static QString palettes;
			static QString palette_base_color;
			static QString palette_colors;
			static QString palette_mirror;
			static QString palette_name;
			static QString palette_num_colors;
			static QString palette_target_color;
			static QString palette_type;

			static QString device_capacity;
			static QString device_port;
			static QString devices;
			static QString device_real_time_refresh;
			static QString event_history_max;
			static QString last_cuefile_directory;
			static QString last_session;
			static QString main_window_option;
			static QString num_sections;
			static QString output_enabled;
			static QString pause_on_start;
			static QString pixel_padding;
			static QString pixel_shape;
			static QString refresh_rate;
			static QString save_session;
			static QString separate_window_option;
			static QString splitter_position;
			static QString window_geometry;
			static QString window_state;

			static QString section_map;
			static QString section_map_local;
			static QString section_map_remote;

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
