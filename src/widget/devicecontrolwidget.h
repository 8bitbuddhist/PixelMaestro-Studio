/*
 * DeviceControlWidget - Widget for managing USB/serial devices.
 */

#ifndef DEVICECONTROLWIDGET_H
#define DEVICECONTROLWIDGET_H

#include <QBuffer>
#include <QSharedPointer>
#include <QThread>
#include <QVector>
#include <QWidget>
#include "controller/serialdevicecontroller.h"
#include "widget/maestrocontrolwidget.h"

namespace Ui {
	class DeviceControlWidget;
}

namespace PixelMaestroStudio {
	class MaestroControlWidget;
	class DeviceControlWidget : public QWidget {
		Q_OBJECT

		public:
			/**
			 * Defines a Cue that should be blocked from serialization.
			 */
			struct BlockedCue {
				/// The Handler that this Cue belongs to.
				CueController::Handler handler;
				/// The action being performed.
				uint8_t action;
			};

			explicit DeviceControlWidget(QWidget *parent = 0);
			~DeviceControlWidget();
			QByteArray* get_maestro_cue();
			void block_cue(CueController::Handler handler, uint8_t action);
			void run_cue(uint8_t* cue, int size);
			void save_devices();
			void update_cuefile_size();

		private slots:
			void on_connectPushButton_clicked();
			void on_previewButton_clicked();
			void on_realTimeCheckBox_stateChanged(int arg1);
			void on_disconnectPushButton_clicked();
			void on_uploadButton_clicked();
			void on_serialOutputComboBox_editTextChanged(const QString &arg1);
			void on_serialOutputListWidget_currentRowChanged(int currentRow);
			void on_capacityLineEdit_editingFinished();

			void set_progress_bar(int val);

			void on_refreshButton_clicked();

			void on_sectionMapButton_clicked();

		private:
			/// List of Cues that should be blocked from executing.
			QVector<BlockedCue> blocked_cues_;

			MaestroControlWidget* maestro_control_widget_ = nullptr;
			Ui::DeviceControlWidget *ui;

			/// Stores the current Maestro configuration in Cue form.
			QByteArray maestro_cue_;

			/// List of activated USB devices.
			QVector<SerialDeviceController> serial_devices_;

			void check_device_rom_capacity();
			bool connect_to_serial_device(QString port_name);
			void disconnect_serial_device(int index);
			void populate_serial_devices();
			void set_device_controls_enabled(bool enabled);
			void write_to_device(SerialDeviceController* device, const char* out, const int size, bool progress = false);
	};
}

#endif // DEVICECONTROLWIDGET_H
