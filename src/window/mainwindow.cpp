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

		// Initialize UI elements
		initialize_widgets();

		// Restore window size
		QSettings settings;
		this->restoreGeometry(settings.value(PreferencesDialog::window_geometry).toByteArray());
		this->restoreState(settings.value(PreferencesDialog::window_state).toByteArray());

		// If the user has a session saved and session auto-saving is enabled, load it into the new session.
		QByteArray bytes = settings.value(PreferencesDialog::last_session).toByteArray();
		on_newAction_triggered();
		if (settings.value(PreferencesDialog::save_session).toBool() && !bytes.isEmpty()) {
			maestro_control_widget_->load_cuefile(bytes);
			maestro_control_widget_->set_maestro_modified(true);
		}

		set_active_cuefile("");

		initialization_complete = true;
	}

	/**
	 * Prompts the user to replace their current session with a new one.
	 * @return If true, user wants to replace the session.
	 */
	bool MainWindow::confirm_unsaved_changes() {
		QMessageBox::StandardButton confirm;
		confirm = QMessageBox::question(this, "Unsaved Changes", "Your current settings will be lost. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
		if (confirm == QMessageBox::Yes) {
			return true;
		}

		return false;
	}

	/**
	 * Reinitializes all widgets.
	 */
	void MainWindow::initialize_widgets() {
		// Add Splitter control
		QLayout* main_layout = this->findChild<QLayout*>("mainLayout");
		this->splitter_ = new QSplitter(main_layout->widget());
		this->splitter_->setOrientation(Qt::Orientation::Vertical);
		main_layout->addWidget(this->splitter_);

		// Initialize Maestro elements
		maestro_control_widget_ = new MaestroControlWidget(splitter_);
		maestro_controller_ = new MaestroController(maestro_control_widget_);

		// Build DrawingAreas if enabled in Preferences
		// TODO: Allow for dynamic adding/removing DrawingAreas. Look into dockable widgets: https://doc.qt.io/qt-5/qdockwidget.html
		QSettings settings;
		if (settings.value(PreferencesDialog::main_window_option, true) == true) {
			maestro_drawing_area_ = new MaestroDrawingArea(splitter_, maestro_controller_);
			splitter_->addWidget(maestro_drawing_area_);
			maestro_controller_->add_drawing_area(dynamic_cast<MaestroDrawingArea*>(maestro_drawing_area_));
			dynamic_cast<MaestroDrawingArea*>(maestro_drawing_area_)->set_maestro_control_widget(maestro_control_widget_);
		}
		if (settings.value(PreferencesDialog::separate_window_option, false) == true) {
			drawing_area_dialog_ = std::unique_ptr<MaestroDrawingAreaDialog>(new MaestroDrawingAreaDialog(this, this->maestro_controller_));
			maestro_controller_->add_drawing_area(drawing_area_dialog_->get_maestro_drawing_area());
			dynamic_cast<MaestroDrawingArea*>(drawing_area_dialog_->get_maestro_drawing_area())->set_maestro_control_widget(maestro_control_widget_);
			drawing_area_dialog_->show();
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
		if (!settings.value(PreferencesDialog::pause_on_start, false).toBool()) {
			maestro_controller_->start();
		}
	}

	void MainWindow::on_aboutAction_triggered() {
		QMessageBox::about(this, QString(QCoreApplication::applicationName()),
					   QString("Build ") + QString(BUILD_VERSION) +
					   QString("\n\nPixelMaestro is a library for creating and rendering 2D animations and patterns.") +
					   QString("\n\n© ") +
					   QString::number(QDate::currentDate().year()));
	}

	/**
	 * Closes the program.
	 */
	void MainWindow::on_exitAction_triggered() {
		if (maestro_control_widget_->get_maestro_modified() && !confirm_unsaved_changes()) {
			return;
		}
		close();
	}

	/**
	 * Opens the documentation site in a browser.
	 */
	void MainWindow::on_helpAction_triggered() {
		QDesktopServices::openUrl(QUrl("https://github.com/8bitbuddhist/PixelMaestro-Studio/wiki", QUrl::TolerantMode));
	}

	/**
	 * Merges a Cuefile into the current Maestro.
	 */
	void MainWindow::on_mergeAction_triggered() {
		open_cuefile(open_cuefile_dialog());
	}

	/**
	 * Opens an Animation Editor instance.
	 * @return True if the Animation Editor opened successfully.
	 */
	void MainWindow::on_newAction_triggered() {
		// If Animation Editor is currently open, verify user wants to close
		if (initialization_complete) {
			// If the user chooses not to continue, exit
			if (maestro_control_widget_->get_maestro_modified() && !confirm_unsaved_changes()) {
				return;
			}
		}

		// Initialize and set the MaestroControlWidget
		maestro_controller_->initialize_maestro();
		maestro_control_widget_->set_maestro_controller(maestro_controller_);

		maestro_control_widget_->refresh_maestro_settings();
		maestro_control_widget_->refresh_section_settings();

		set_active_cuefile("");
	}

	/**
	 * Replaces the current Maestro with the loaded Cuefile.
	 */
	void MainWindow::on_openAction_triggered() {
		QString file = open_cuefile_dialog();
		if (!file.isEmpty()) {
			on_newAction_triggered();
			if (maestro_control_widget_->get_maestro_modified()) {
				/*
				 * If the user cancels out of newAction, the Maestro remains modified and so we know to cancel here.
				 * Otherwise, the Maestro is replaced and therefore unmodified and so we can continue.
				 */
				return;
			}
			if (open_cuefile(file)) {
				set_active_cuefile(file);
			}
		}
	}

	/**
	 * Opens the Preferences dialog.
	 */
	void MainWindow::on_preferencesAction_triggered() {
		PreferencesDialog preferences;
		preferences.exec();
	}

	/**
	 * Opens a Cuefile and appends each Cue to the Event History.
	 */
	void MainWindow::on_queueAction_triggered() {
		QString file = open_cuefile_dialog();
		if (!file.isEmpty()) {
			maestro_control_widget_->show_control_widget_->set_maestro_locked(true);
			open_cuefile(file);
			maestro_control_widget_->show_control_widget_->set_maestro_locked(false);
		}
	}

	/**
	 * Saves the currently loaded Cuefile.
	 */
	void MainWindow::on_saveAction_triggered() {
		if (this->loaded_cuefile_path_.isEmpty()) {
			on_saveAsAction_triggered();
		}
		else {			
			QFile file(this->loaded_cuefile_path_);
			if (file.open(QFile::WriteOnly)) {
				QDataStream datastream(&file);
				this->maestro_controller_->save_maestro_to_datastream(&datastream);
				file.close();
				setWindowModified(false);
			}
		}
	}

	/**
	 * Saves the current Maestro to a new Cuefile.
	 */
	void MainWindow::on_saveAsAction_triggered() {
		// Open the window in the last used directory, if possible
		QSettings settings;
		QString path = settings.value(PreferencesDialog::last_cuefile_directory, QDir::home().path()).toString();
		QString filename = QFileDialog::getSaveFileName(this,
			QString("Save Cue File"),
			path,
			QString("PixelMaestro Cue File (*.pmc)"));

		if (!filename.isEmpty()) {
			if (!filename.endsWith(".pmc", Qt::CaseInsensitive)) {
				filename.append(".pmc");
			}

			this->loaded_cuefile_path_ = filename;
			on_saveAction_triggered();
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
	 * @param filename Cuefile path.
	 * @return True if the Cuefile was opened successfully.
	 */
	bool MainWindow::open_cuefile(const QString& filename) {
		if (filename.isEmpty()) return false;

		QFile file(filename);
		if (file.open(QFile::ReadOnly)) {
			QByteArray bytes = file.readAll();
			maestro_control_widget_->load_cuefile(bytes);
			setWindowModified(false);
			return true;
		}

		return false;
	}

	/**
	 * Sets the current active Cuefile to the path specified.
	 *
	 * @param path Path to the current Cuefile.
	 */
	void MainWindow::set_active_cuefile(const QString& path) {
		this->loaded_cuefile_path_ = path;

		if (path.isEmpty()) {
			this->setWindowTitle(QCoreApplication::applicationName() + "[*]");
		}
		else {
			this->setWindowTitle(QCoreApplication::applicationName() + " - " + QFileInfo(path).fileName() + "[*]");
		}
	}

	MainWindow::~MainWindow() {
		QSettings settings;

		// If session saving on close is enabled, save the session
		if (settings.value(PreferencesDialog::save_session).toBool()) {
			QByteArray maestro_config;
			QDataStream maestro_datastream(&maestro_config, QIODevice::Truncate);
			maestro_controller_->save_maestro_to_datastream(&maestro_datastream);
			settings.setValue(PreferencesDialog::last_session, maestro_config);
		}

		// Save window geometry
		settings.setValue(PreferencesDialog::window_geometry, saveGeometry());
		settings.setValue(PreferencesDialog::window_state, saveState());

		// Save splitter position
		settings.setValue(PreferencesDialog::splitter_position, this->splitter_->saveState());

		delete maestro_control_widget_;
		delete maestro_controller_;
		delete splitter_;
		delete ui;
	}
}
