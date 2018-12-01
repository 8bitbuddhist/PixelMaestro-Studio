/*
 * MaestroControl - Widget for interacting with a MaestroController.
 */

#ifndef MAESTROCONTROLWIDGET_H
#define MAESTROCONTROLWIDGET_H

#include <memory>
#include <QButtonGroup>
#include <QColor>
#include <QLocale>
#include <QSerialPort>
#include <QSharedPointer>
#include <QTimer>
#include <QVector>
#include <QWidget>
#include "utility/cueinterpreter.h"
#include "controller/maestrocontroller.h"
#include "controller/palettecontroller.h"
#include "controller/showcontroller.h"
#include "core/colors.h"
#include "core/maestro.h"
#include "cue/animationcuehandler.h"
#include "cue/canvascuehandler.h"
#include "cue/maestrocuehandler.h"
#include "cue/sectioncuehandler.h"
#include "cue/showcuehandler.h"
#include "dialog/maestrodrawingareadialog.h"
#include "widget/devicecontrolwidget.h"
#include "widget/animationcontrolwidget.h"
#include "widget/canvascontrolwidget.h"
#include "widget/sectioncontrolwidget.h"
#include "widget/showcontrolwidget.h"

namespace Ui {
	class MaestroControlWidget;
}

using namespace PixelMaestro;

namespace PixelMaestroStudio {
	class AnimationControlWidget;
	class CanvasControlWidget;
	class DeviceControlWidget;
	class MaestroController;
	class MaestroDrawingAreaDialog;
	class SectionControlWidget;
	class ShowController;
	class ShowControlWidget;
	class MaestroControlWidget : public QWidget {
		Q_OBJECT

		public:

			/// The controller for managing Palettes.
			PaletteController palette_controller_;

			// Cue components
			CueController* cue_controller_ = nullptr;
			AnimationCueHandler* animation_handler = nullptr;
			CanvasCueHandler* canvas_handler = nullptr;
			MaestroCueHandler* maestro_handler = nullptr;
			SectionCueHandler* section_handler = nullptr;
			ShowCueHandler* show_handler = nullptr;

			// Control subwidgets
			QSharedPointer<AnimationControlWidget> animation_control_widget_;
			QSharedPointer<CanvasControlWidget> canvas_control_widget_;
			QSharedPointer<DeviceControlWidget> device_control_widget_;
			QSharedPointer<SectionControlWidget> section_control_widget_;
			QSharedPointer<ShowControlWidget> show_control_widget_;

			explicit MaestroControlWidget(QWidget* parent);
			~MaestroControlWidget();
			bool get_maestro_modified() const;
			void edit_palettes(QString palette);
			MaestroController* get_maestro_controller();
			void load_cuefile(const QByteArray& byte_array);
			void refresh_section_settings();
			void refresh_maestro_settings();
			void run_cue(uint8_t* cue, bool remote_only = false);
			void set_maestro_controller(MaestroController* maestro_controller);
			void set_maestro_modified(bool modified);

		private slots:
			void on_playPauseButton_toggled(bool checked);

			void on_syncButton_clicked();

			void on_lockButton_toggled(bool checked);

			void on_brightnessSpinBox_editingFinished();

			void on_brightnessSlider_valueChanged(int value);

		private:
			Ui::MaestroControlWidget *ui;

			/// MaestroController that this widget is controlling.
			MaestroController* maestro_controller_ = nullptr;

			/// Tracks whether the Maestro is currently modified.
			bool modified_ = false;
	};
}

#endif // MAESTROCONTROLWIDGET_H
