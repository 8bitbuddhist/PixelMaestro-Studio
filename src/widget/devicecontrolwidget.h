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
#include "model/serialdevice.h"
#include "widget/maestrocontrolwidget.h"

namespace Ui {
	class DeviceControlWidget;
}

namespace PixelMaestroStudio {
	class MaestroControlWidget;
	class DeviceControlWidget : public QWidget {
		Q_OBJECT

		public:
			explicit DeviceControlWidget(MaestroControlWidget* maestro_control_widget, QWidget *parent = 0);
			~DeviceControlWidget();
			QByteArray* get_maestro_cue();
			void run_cue(uint8_t* cue, int size);
			void update_cuefile_size();

		private slots:
			void on_connectPushButton_clicked();
			void on_interpretCuePushButton_clicked();
			void on_realTimeCheckBox_stateChanged(int arg1);
			void on_disconnectPushButton_clicked();
			void on_sendPushButton_clicked();
			void on_serialOutputComboBox_editTextChanged(const QString &arg1);
			void on_serialOutputListWidget_currentRowChanged(int currentRow);
			void on_capacityLineEdit_editingFinished();

			void set_progress_bar(int val);

		private:
			MaestroControlWidget* maestro_control_widget_ = nullptr;
			Ui::DeviceControlWidget *ui;

			/// Stores the current Maestro configuration in Cue form.
			QByteArray maestro_cue_;

			/// List of activated USB devices.
			QVector<SerialDevice> serial_devices_;

			void check_device_rom_capacity();
			bool connect_to_serial_device(QString port_name);
			void disconnect_serial_device(int index);
			void populate_serial_devices();
			void save_devices();
			void write_to_device(SerialDevice* device, const char* out, int size, bool progress = false);
	};
}

#endif // DEVICECONTROLWIDGET_H
