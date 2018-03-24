#ifndef DEVICECONTROLWIDGET_H
#define DEVICECONTROLWIDGET_H

#include <QBuffer>
#include <QSharedPointer>
#include <QVector>
#include <QWidget>
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
			void on_removeSerialDevicePushButton_clicked();
			void on_serialOutputComboBox_editTextChanged(const QString &arg1);
			void on_serialOutputListWidget_currentRowChanged(int currentRow);

			void on_sendPushButton_clicked();

		private:
			MaestroControlWidget* maestro_control_widget_ = nullptr;
			Ui::DeviceControlWidget *ui;

			/// Stores the current Maestro configuration in Cue form.
			QByteArray maestro_cue_;

			/// List of enabled USB devices.
			QVector<QSharedPointer<QSerialPort>> serial_devices_;

			bool connect_to_serial_device(QString port_name);
			bool disconnect_serial_device(QString port_name);
	};
}

#endif // DEVICECONTROLWIDGET_H
