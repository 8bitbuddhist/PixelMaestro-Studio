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
	/*
	 * TODO: Add ability to add/remove Maestro preview widget and window on-demand
	 * Look into dockable widgets: https://doc.qt.io/qt-5/qdockwidget.html
	 */
	MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
		ui->setupUi(this);
		setWindowTitle(QCoreApplication::applicationName());

		// Initialize UI elements
		initialize_widgets();

		// Restore window size
		QSettings settings;
		this->restoreGeometry(settings.value(PreferencesDialog::window_geometry).toByteArray());
		this->restoreState(settings.value(PreferencesDialog::window_state).toByteArray());

		// If the user has a session saved and session auto-saving is enabled, open the session. Otherwise, start a new session.
		QByteArray bytes = settings.value(PreferencesDialog::last_session).toByteArray();
		if (settings.value(PreferencesDialog::save_session).toBool() == true && !bytes.isEmpty()) {
			open_cuefile(bytes, true);
		}
		else {
			on_action_Open_Animation_Editor_triggered(false);
		}

		initialization_complete = true;
	}

	/**
	 * Reinitializes all widgets.
	 */
	void MainWindow::initialize_widgets() {
		ui->action_Save_Maestro->setEnabled(false);

		// Add Splitter control
		QLayout* main_layout = this->findChild<QLayout*>("mainLayout");
		this->splitter_ = new QSplitter(main_layout->widget());
		this->splitter_->setOrientation(Qt::Orientation::Vertical);
		main_layout->addWidget(this->splitter_);

		// Initialize Maestro elements
		maestro_control_widget_ = new MaestroControlWidget(splitter_);
		maestro_controller_ = new MaestroController(maestro_control_widget_);

		// Build DrawingAreas if enabled in Preferences
		// FIXME: Allow for dynamic adding/removing DrawingAreas
		QSettings settings;
		if (settings.value(PreferencesDialog::main_window_option, true) == true) {
			maestro_drawing_area_ = new MaestroDrawingArea(splitter_, maestro_controller_);
			splitter_->addWidget(maestro_drawing_area_);
			maestro_controller_->add_drawing_area(static_cast<MaestroDrawingArea*>(maestro_drawing_area_));
			static_cast<MaestroDrawingArea*>(maestro_drawing_area_)->set_maestro_control_widget(maestro_control_widget_);
		}
		if (settings.value(PreferencesDialog::separate_window_option, false) == true) {
			drawing_area_dialog_ = std::unique_ptr<MaestroDrawingAreaDialog>(new MaestroDrawingAreaDialog(this, this->maestro_controller_));
			maestro_controller_->add_drawing_area(drawing_area_dialog_->get_maestro_drawing_area());
			static_cast<MaestroDrawingArea*>(drawing_area_dialog_->get_maestro_drawing_area())->set_maestro_control_widget(maestro_control_widget_);
			drawing_area_dialog_.get()->show();
		}

		// Add control widget to main window
		splitter_->addWidget(maestro_control_widget_);

		// Restore splitter position. If the position isn't saved in the user's settings, default to a 50/50 split
		QByteArray splitter_state = settings.value(PreferencesDialog::splitter_position).toByteArray();
		if (splitter_state.size() > 0) {
			this->splitter_->restoreState(splitter_state);
		}
		else {
			this->splitter_->setSizes(QList<int>({INT_MAX, INT_MAX}));
		}

		/*
		 * If "pause on start" option is checked, don't start the Maestro.
		 * This also causes the MaestroControlWidget to automatically turn on the Show controls so users can hit the pause button.
		 */
		if (settings.value(PreferencesDialog::pause_on_start, false).toBool() == false) {
			maestro_controller_->start();
		}
	}

	void MainWindow::on_action_About_triggered() {
		QMessageBox::about(this, QString(QCoreApplication::applicationName()),
					   QString("Build ") + QString(BUILD_VERSION) +
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
		if (!keep_current_open && initialization_complete) {
			QMessageBox::StandardButton confirm;
			confirm = QMessageBox::question(this, "Open new Maestro", "Your current settings will be lost. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
			if (confirm != QMessageBox::Yes) {
				return;
			}

			maestro_controller_->initialize_maestro();
		}

		set_active_cuefile("");
		ui->action_Save_Maestro->setEnabled(true);
	}

	/**
	 * Merges a Cuefile into the current Maestro.
	 */
	void MainWindow::on_action_Open_triggered() {
		open_cuefile(open_cuefile_dialog(), false);
	}

	/**
	 * Replaces the current Maestro with the loaded Cuefile.
	 */
	void MainWindow::on_actionOpen_Maestro_triggered() {
		QString file = open_cuefile_dialog();
		if (!file.isEmpty()) {
			open_cuefile(file, true);
			set_active_cuefile(file);
		}
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
	void MainWindow::on_action_Save_triggered() {
		if (this->loaded_cuefile_path_.isEmpty()) {
			on_action_Save_Maestro_triggered();
		}
		else {
			QSettings settings;
			// Store the directory that the file was saved to
			settings.setValue(PreferencesDialog::last_cuefile_directory, QFileInfo(this->loaded_cuefile_path_).path());
			maestro_controller_->save_cuefile(this->loaded_cuefile_path_);
		}
	}

	/**
	 * Saves the current Maestro to a new Cuefile.
	 */
	void MainWindow::on_action_Save_Maestro_triggered() {
		// Open the window in the last used directory, if possible
		QSettings settings;
		QString path = settings.value(PreferencesDialog::last_cuefile_directory, QDir::home().path()).toString();
		QString filename = QFileDialog::getSaveFileName(this,
			QString("Save Cue File"),
			path,
			QString("PixelMaestro Cue File (*.pmc)"));

		if (!filename.endsWith(".pmc", Qt::CaseInsensitive)) {
			filename.append(".pmc");
		}

		this->loaded_cuefile_path_ = filename;
		if (!this->loaded_cuefile_path_.isEmpty()) {
			on_action_Save_triggered();
			set_active_cuefile(this->loaded_cuefile_path_);
		}
	}

	/**
	 * Prompts the user to select a Cuefile.
	 * @return Cuefile filename.
	 */
	QString MainWindow::open_cuefile_dialog() {
		// Open the window in the last used directory, if possible
		QSettings settings;
		QString path = settings.value(PreferencesDialog::last_cuefile_directory, QDir::home().path()).toString();
		QString filename = QFileDialog::getOpenFileName(this,
			QString("Open Cue File"),
			path,
			QString("PixelMaestro Cue File (*.pmc)"));

		// Store the directory that the file was opened from
		settings.setValue(PreferencesDialog::last_cuefile_directory, QFileInfo(filename).path());
		return filename;
	}

	/**
	 * Loads a Cuefile into a new session.
	 * @param byte_array Array containing the Cuefile.
	 */
	void MainWindow::open_cuefile(QByteArray byte_array, bool new_session) {
		// Read in the Cuefile, then load the Animation Editor
		on_action_Open_Animation_Editor_triggered(!new_session);

		maestro_control_widget_->load_cuefile(byte_array);

		// Refresh the Animation Editor
		maestro_control_widget_->refresh_maestro_settings();
		maestro_control_widget_->set_active_section(maestro_control_widget_->get_active_section());
	}

	/**
	 * Loads a Cuefile into a new session.
	 * @param filename Cuefile path.
	 */
	void MainWindow::open_cuefile(QString filename, bool new_session) {
		if (filename.isEmpty()) return;

		QFile file(filename);
		if (file.open(QFile::ReadOnly)) {
			QByteArray bytes = file.readAll();
			open_cuefile(bytes, new_session);
		}
	}

	/**
	 * Sets the current active Cuefile to the path specified.
	 *
	 * @param path Path to the current Cuefile.
	 */
	void MainWindow::set_active_cuefile(QString path) {
		this->loaded_cuefile_path_ = path;

		if (!path.isEmpty()) {
			this->setWindowTitle("PixelMaestro Studio - " + QFileInfo(path).fileName());
		}
		else {
			this->setWindowTitle("PixelMaestro Studio");
		}
	}

	MainWindow::~MainWindow() {
		QSettings settings;

		// Save window geometry
		settings.setValue(PreferencesDialog::window_geometry, saveGeometry());
		settings.setValue(PreferencesDialog::window_state, saveState());

		// Save splitter position
		settings.setValue(PreferencesDialog::splitter_position, this->splitter_->saveState());

		delete maestro_control_widget_;
		delete splitter_;
		delete ui;
	}
}
