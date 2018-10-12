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
			void on_action_About_triggered();
			void on_action_Exit_triggered();
			void on_action_Online_Help_triggered();
			void on_action_Open_triggered();
			void on_action_Open_Animation_Editor_triggered(bool keep_current_open = false);
			void on_action_Preferences_triggered();
			void on_action_Save_triggered();
			void on_action_Save_Maestro_triggered();
			void on_actionOpen_Maestro_triggered();

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
			MaestroControlWidget* maestro_control_widget_ = nullptr;
			QSplitter* splitter_ = nullptr;
			Ui::MainWindow* ui;

			void initialize_widgets();
			void open_cuefile(QByteArray byte_array, bool new_session);
			void open_cuefile(QString filename, bool new_session);
			void set_active_cuefile(QString path);
			QString open_cuefile_dialog();
	};
}

#endif // MAINWINDOW_H
