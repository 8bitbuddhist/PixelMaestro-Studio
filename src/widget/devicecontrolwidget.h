/*
 * DeviceControlWidget - Widget for managing USB/serial devices.
 */

#ifndef DEVICECONTROLWIDGET_H
#define DEVICECONTROLWIDGET_H

#include <QBuffer>
#include <QSharedPointer>
#include <QVector>
#include <QWidget>
#include "utility/serialdevice.h"
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
			void run_cue(uint8_t* cue, int size);

		private slots:
			void on_addSerialDevicePushButton_clicked();
			void on_interpretCuePushButton_clicked();
			void on_realTimeCheckBox_stateChanged(int arg1);
			void on_removeSerialDevicePushButton_clicked();
			void on_sendPushButton_clicked();
			void on_serialOutputComboBox_editTextChanged(const QString &arg1);
			void on_serialOutputListWidget_currentRowChanged(int currentRow);

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
			void save_device_list();
			void write_to_devices(const char* out, int size);
	};
}

#endif // DEVICECONTROLWIDGET_H
