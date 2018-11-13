#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../controller/maestrocontroller.h"
#include "../widget/maestrocontrolwidget.h"
#include <QByteArray>
#include <QMainWindow>
#include <QSplitter>
#include <QStandardPaths>
#include <QString>

namespace Ui {
	class MainWindow;
}

namespace PixelMaestroStudio {
	class MainWindow : public QMainWindow {
		Q_OBJECT

		public:
			explicit MainWindow(QWidget* parent = 0);
			~MainWindow();

		private slots:
			void on_aboutAction_triggered();
			void on_exitAction_triggered();
			void on_helpAction_triggered();
			void on_mergeAction_triggered();
			void on_newAction_triggered();
			void on_preferencesAction_triggered();
			void on_saveAction_triggered();
			void on_saveAsAction_triggered();
			void on_openAction_triggered();

		private:
			/// Whether the application has completed initialization.
			bool initialization_complete = false;

			/// Path to the last opened Cuefile.
			QString loaded_cuefile_path_;
			MaestroController* maestro_controller_ = nullptr;

			/// Main rendering area.
			QWidget* maestro_drawing_area_ = nullptr;

			/// Detached rendering area.
			std::unique_ptr<MaestroDrawingAreaDialog> drawing_area_dialog_;

			/// The widget responsible for modifying the Maestro.
			MaestroControlWidget* maestro_control_widget_ = nullptr;

			QSplitter* splitter_ = nullptr;
			Ui::MainWindow* ui;

			void initialize_widgets();
			bool open_cuefile(QString filename);
			bool confirm_session_overwrite();
			void set_active_cuefile(QString path);
			QString open_cuefile_dialog();
	};
}

#endif // MAINWINDOW_H
