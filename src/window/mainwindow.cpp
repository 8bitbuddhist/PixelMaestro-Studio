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

		setWindowIcon(QIcon("qrc:/../../../docsrc/images/logo.png"));

		// Initialize UI elements
		initialize_widgets();

		// If the user has a session saved and session auto-saving is enabled, load it into the new session.
		QSettings settings;
		QByteArray bytes = settings.value(PreferencesDialog::last_session).toByteArray();
		on_newAction_triggered();
		if (settings.value(PreferencesDialog::save_session, true).toBool() && !bytes.isEmpty()) {
			maestro_control_widget_->load_cuefile(bytes);
			maestro_control_widget_->set_maestro_modified(true);

			// Restore output screens
			ui->action_Main_Window->setChecked(settings.value(PreferencesDialog::main_window_option, true).toBool());
			ui->action_Secondary_Window->setChecked(settings.value(PreferencesDialog::separate_window_option, false).toBool());

			// Restore window size
			this->restoreGeometry(settings.value(PreferencesDialog::window_geometry).toByteArray());
			this->restoreState(settings.value(PreferencesDialog::window_state).toByteArray());
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
		confirm = QMessageBox::question(this, "Unsaved Changes", "Your changes will be lost. Are you sure you want to continue?", QMessageBox::Yes|QMessageBox::No);
		return (confirm == QMessageBox::Yes);
	}

	/**
	 * Reinitializes all widgets.
	 */
	void MainWindow::initialize_widgets() {

		// Initialize Maestro elements
		maestro_control_widget_ = new MaestroControlWidget(ui->mainWidget);
		maestro_controller_ = new MaestroController(*maestro_control_widget_);
		maestro_control_widget_->set_maestro_controller(*maestro_controller_);

		// Build DrawingAreas if enabled in Preferences
		QSettings settings;
		if (settings.value(PreferencesDialog::main_window_option, true) == true && !ui->action_Main_Window->isChecked()) {
			ui->action_Main_Window->setChecked(true);
		}
		if (settings.value(PreferencesDialog::separate_window_option, false) == true && !ui->action_Secondary_Window->isChecked()) {
			ui->action_Secondary_Window->setChecked(true);
		}

		// Add control widget to main window
		ui->mainWidget->layout()->addWidget(maestro_control_widget_);

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
					   QString("PixelMaestro Studio ") + QString(BUILD_VERSION) +
					   QString("\n\nPixelMaestro Studio is an application for designing LED animations.") +
					   QString("\n\nPowered by the PixelMaestro library. Learn more at https://pixelmastro.studio.") +
					   QString("\n\n© 2017 − ") + QString::number(QDate::currentDate().year()) + QString(", the PixelMaestro contributors"));
	}

	/**
	 * Closes the program.
	 */
	void MainWindow::on_exitAction_triggered() {
		// See if we need to prompt the user
		if (maestro_control_widget_->get_maestro_modified()) {
			QSettings settings;
			if (settings.value(PreferencesDialog::save_session, false) == false) {
				if (!confirm_unsaved_changes()) {
					return;
				}
			}
		}
		close();
	}

	/**
	 * Opens the documentation site in a browser.
	 */
	void MainWindow::on_helpAction_triggered() {
		QDesktopServices::openUrl(QUrl("https://8bitbuddhist.github.io/PixelMaestro-Studio/", QUrl::TolerantMode));
	}

	void MainWindow::on_action_Main_Window_toggled(bool arg1) {
		maestro_control_widget_->toggle_maestro_drawing_area(arg1);
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
		maestro_control_widget_->set_maestro_controller(*maestro_controller_);

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
				this->maestro_controller_->save_maestro_to_datastream(datastream);
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

	void MainWindow::on_action_Secondary_Window_toggled(bool arg1) {
		if (arg1) {
			// Checked: create new DrawingArea
			drawing_area_dialog_ = QSharedPointer<MaestroDrawingAreaDialog>(new MaestroDrawingAreaDialog(this, *this->maestro_controller_));
			maestro_controller_->add_drawing_area(drawing_area_dialog_->get_maestro_drawing_area());
			dynamic_cast<MaestroDrawingArea&>(drawing_area_dialog_->get_maestro_drawing_area()).set_maestro_control_widget(maestro_control_widget_);
			drawing_area_dialog_->show();
		}
		else {
			// Unchecked
			MaestroDrawingArea& drawing_area = drawing_area_dialog_->get_maestro_drawing_area();
			maestro_controller_->remove_drawing_area(drawing_area);
			drawing_area_dialog_.clear();
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
		if (settings.value(PreferencesDialog::save_session, true).toBool()) {
			QByteArray maestro_config;
			QDataStream maestro_datastream(&maestro_config, QIODevice::Truncate);
			maestro_controller_->save_maestro_to_datastream(maestro_datastream);
			settings.setValue(PreferencesDialog::last_session, maestro_config);
			settings.setValue(PreferencesDialog::separate_window_option, ui->action_Secondary_Window->isChecked());
			settings.setValue(PreferencesDialog::main_window_option, ui->action_Main_Window->isChecked());
		}

		// Save window geometry
		settings.setValue(PreferencesDialog::window_geometry, saveGeometry());
		settings.setValue(PreferencesDialog::window_state, saveState());

		delete maestro_control_widget_;
		delete maestro_controller_;
		delete ui;
	}
}
