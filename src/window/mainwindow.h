#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../controller/maestrocontroller.h"
#include "../widget/maestrocontrolwidget.h"
#include <QMainWindow>

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
			void on_action_Donate_triggered();
			void on_action_Exit_triggered();
			void on_action_Online_Help_triggered();
			void on_action_Open_Animation_Editor_triggered(bool keep_current_open = false);
			void on_action_Preferences_triggered();
			void on_action_Save_Maestro_triggered();
			void on_actionOpen_Maestro_triggered();

		private:
			MaestroController* maestro_controller_ = nullptr;
			QWidget* maestro_drawing_area_ = nullptr;
			MaestroControlWidget* maestro_control_widget_ = nullptr;
			QLayout* main_layout_ = nullptr;
			Ui::MainWindow* ui;

			void initialize_widgets();
	};
}

#endif // MAINWINDOW_H