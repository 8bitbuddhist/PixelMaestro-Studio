#ifndef SHOWCONTROL_H
#define SHOWCONTROL_H

#include <QLocale>
#include <QTimer>
#include <QVector>
#include <QWidget>
#include "controller/cueinterpreter.h"
#include "controller/showcontroller.h"
#include "maestrocontrol.h"

namespace Ui {
	class ShowControl;
}

class MaestroControl;
class ShowController;

class ShowControl : public QWidget {
		Q_OBJECT

	public:
		explicit ShowControl(ShowController* show_controller, CueController* cue_controller, QWidget *parent = 0);
		~ShowControl();
		void add_cue_to_history(uint8_t* cue);
		void refresh_event_list();

	private slots:
		void on_addEventButton_clicked();
		void refresh_maestro_last_time();

	private:
		CueController* cue_controller_ = nullptr;
		CueInterpreter cue_interpreter_;

		/// Event history.
		QVector<uint8_t*> history_;

		/// Locale for formatting times.
		QLocale locale_ = QLocale::system();

		/// Parent MaestroControl.
		MaestroControl* maestro_control_ = nullptr;

		/// Updates the current Maestro time.
		QTimer timer_;

		ShowController* show_controller_ = nullptr;
		Ui::ShowControl *ui;
};

#endif // SHOWCONTROL_H
