#include <QDate>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>
#include "dialog/preferencesdialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

namespace PixelMaestroStudio {
	MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
		ui->setupUi(this);
		this->main_layout_ = this->findChild<QLayout*>("mainLayout");
		setWindowTitle("PixelMaestro Studio");

		// Open the Animation Editor
		on_action_Open_Animation_Editor_triggered(false);
	}

	void MainWindow::on_action_About_triggered() {
		QMessageBox::about(this, QString("PixelMaestro Studio"), QString("PixelMaestro ") +
						   QString(BUILD_VERSION) +
						   QString("\n\nPixelMaestro is a library for creating and rendering 2D animations and patterns.") +
						   QString("\n\n© ") +
						   QString::number(QDate::currentDate().year()));
	}

	/**
	 * Closes the program.
	 */
	void MainWindow::on_action_Exit_triggered() {
		close();
	}

	/**
	 * Opens the documentation site in a browser.
	 */
	void MainWindow::on_action_Online_Help_triggered() {
		QDesktopServices::openUrl(QUrl("https://github.com/8bitbuddhist/PixelMaestro-Studio/wiki", QUrl::TolerantMode));
	}

	/**
	 * Opens an Animation Editor instance.
	 * @param keep_current_open If true, uses the existing DrawingArea instead of creating a new one. This is needed when loading in Cuefiles.
	 */
	void MainWindow::on_action_Open_Animation_Editor_triggered(bool keep_current_open) {

		// If Animation Editor is currently open, verify user wants to close
		if (maestro_control_widget_ != nullptr) {
			QMessageBox::StandardButton confirm;
			confirm = QMessageBox::question(this, "Open new Maestro", "Your current settings will be lost. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
			if (confirm != QMessageBox::Yes) {
				return;
			}
		}

		QSettings settings;

		if (!keep_current_open) {
			// If the user has a session saved and they chose to continue from their last session, open the session, otherwise start a new session
			if (settings.value(PreferencesDialog::save_session).toBool() == true && QFile(session_file_path).exists()) {
				open_cuefile(session_file_path);
				return;
			}
			else {
				initialize_widgets();
			}
		}

		// If the Main DrawingArea is enabled as an output device, display it
		if (settings.value(PreferencesDialog::main_window_option, true) == true) {
			maestro_drawing_area_ = new MaestroDrawingArea(main_layout_->widget(), maestro_controller_);
			main_layout_->addWidget(maestro_drawing_area_);
		}

		maestro_controller_->start();

		// Initialize MaestroControlWidget
		maestro_control_widget_ = new MaestroControlWidget(main_layout_->widget(), maestro_controller_);
		if (maestro_drawing_area_ != nullptr) {
			static_cast<MaestroDrawingArea*>(maestro_drawing_area_)->set_maestro_control_widget(maestro_control_widget_);
		}
		main_layout_->addWidget(maestro_control_widget_);

		ui->action_Save_Maestro->setEnabled(true);
	}

	/**
	 * Loads a Cuefile.
	 */
	void MainWindow::on_actionOpen_Maestro_triggered() {
		// Open the window in the last used directory, if possible
		QSettings settings;
		QString path = settings.value(PreferencesDialog::last_cuefile_directory, QDir::home().path()).toString();
		QString filename = QFileDialog::getOpenFileName(this,
			QString("Open Cue File"),
			path,
			QString("PixelMaestro Cue File (*.pmc)"));

		// Store the directory that the file was opened from
		settings.setValue(PreferencesDialog::last_cuefile_directory, QFileInfo(filename).path());
		open_cuefile(filename);
	}

	/**
	 * Opens the Preferences dialog.
	 */
	void MainWindow::on_action_Preferences_triggered() {
		PreferencesDialog preferences;
		preferences.exec();
	}

	/**
	 * Saves the current Maestro to a Cuefile.
	 */
	void MainWindow::on_action_Save_Maestro_triggered() {
		// Open the window in the last used directory, if possible
		QSettings settings;
		QString path = settings.value(PreferencesDialog::last_cuefile_directory, QDir::home().path()).toString();
		QString filename = QFileDialog::getSaveFileName(this,
			QString("Save Cue File"),
			path,
			QString("PixelMaestro Cue File (*.pmc)"));

		if (!filename.isEmpty()) {
			// Store the directory that the file was saved to
			settings.setValue(PreferencesDialog::last_cuefile_directory, QFileInfo(filename).path());
			maestro_controller_->save_cuefile(filename);
		}
	}

	/**
	 * Reinitializes all widgets.
	 */
	void MainWindow::initialize_widgets() {
		main_layout_->removeWidget(maestro_drawing_area_);
		main_layout_->removeWidget(maestro_control_widget_);
		removeEventFilter(maestro_drawing_area_);

		ui->action_Save_Maestro->setEnabled(false);

		if (maestro_drawing_area_) {
			delete maestro_drawing_area_;
			maestro_drawing_area_ = nullptr;
		}
		if (maestro_control_widget_) {
			delete maestro_control_widget_;
			maestro_control_widget_ = nullptr;
		}
		if (maestro_controller_) {
			delete maestro_controller_;
			maestro_controller_ = nullptr;
		}

		maestro_controller_ = new MaestroController();
	}

	void MainWindow::on_action_Donate_triggered() {
		QDesktopServices::openUrl(QUrl("https://www.patreon.com/bePatron?u=8547028", QUrl::TolerantMode));
	}

	/**
	 * Loads a Cuefile into a new session.
	 * @param filename Cuefile path.
	 */
	void MainWindow::open_cuefile(QString filename) {
		if (filename.isEmpty()) return;

		// Read in the CueFile, then load the Animation Editor
		initialize_widgets();
		on_action_Open_Animation_Editor_triggered(true);
		maestro_control_widget_->load_cuefile(filename);

		// Refresh the Animation Editor
		maestro_control_widget_->refresh_maestro_settings();
		maestro_control_widget_->set_active_section(maestro_control_widget_->get_active_section());
	}

	MainWindow::~MainWindow() {
		QSettings settings;
		if (settings.value(PreferencesDialog::save_session).toBool() == true) {
			// Save Maestro config
			if (maestro_controller_ != nullptr) {
				maestro_controller_->save_cuefile(session_file_path);
			}
		}

		delete ui;
	}
}