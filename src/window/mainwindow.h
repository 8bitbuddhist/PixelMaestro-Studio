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

			void on_queueAction_triggered();

			void on_action_Main_Window_toggled(bool arg1);

			void on_action_Secondary_Window_toggled(bool arg1);

		private:
			/// Whether the application has completed initialization.
			bool initialization_complete = false;

			/// Path to the last opened Cuefile.
			QString loaded_cuefile_path_;
			MaestroController* maestro_controller_ = nullptr;

			/// Detached rendering area.
			QSharedPointer<MaestroDrawingAreaDialog> drawing_area_dialog_;

			/// The widget responsible for modifying the Maestro.
			MaestroControlWidget* maestro_control_widget_ = nullptr;

			QSplitter* splitter_ = nullptr;
			Ui::MainWindow* ui;

			void initialize_widgets();
			bool open_cuefile(const QString& filename);
			bool confirm_unsaved_changes();
			void set_active_cuefile(const QString& path);
			QString open_cuefile_dialog();
	};
}

#endif // MAINWINDOW_H
