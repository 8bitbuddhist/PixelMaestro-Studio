#ifndef SHOWCONTROLWIDGET_H
#define SHOWCONTROLWIDGET_H

#include <QDropEvent>
#include <QListWidgetItem>
#include <QLocale>
#include <QTimer>
#include <QVector>
#include <QWidget>
#include "controller/showcontroller.h"
#include "utility/cueinterpreter.h"
#include "widget/maestrocontrolwidget.h"

namespace Ui {
	class ShowControlWidget;
}

namespace PixelMaestroStudio {
	class ShowController;
	class ShowControlWidget : public QWidget {
		Q_OBJECT

		public:
			explicit ShowControlWidget(QWidget *parent = nullptr);
			~ShowControlWidget();

			void add_event_to_history(uint8_t* cue);
			bool get_maestro_locked() const;
			void initialize();
			void refresh();
			void set_maestro_locked(bool locked);

		protected:
			bool eventFilter(QObject *watched, QEvent *event);

		private slots:
			void on_enableCheckBox_toggled(bool checked);
			void on_timingModeComboBox_currentIndexChanged(int index);
			void on_loopCheckBox_toggled(bool checked);
			void timer_refresh();

			void on_addEventButton_clicked();
			void on_removeEventButton_clicked();
			void on_moveEventUpButton_clicked();
			void on_moveEventDownButton_clicked();
			void on_clearQueueButton_clicked();
			void on_clearHistoryButton_clicked();
			void on_eventQueueWidget_rowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);

			void on_eventQueueWidget_itemDoubleClicked(QListWidgetItem *item);

		private:
			/// History of actions performed in the editor. Each entry contains a copy of the Event's Cue.
			QVector<QVector<uint8_t>> event_history_;

			/// Time that the last Event ran.
			uint32_t last_event_time_ = 0;

			/// Locale for formatting numbers (specifically for displaying times).
			QLocale locale_ = QLocale::system();

			/// The parent controller widget.
			MaestroControlWidget& maestro_control_widget_;

			/// If true, no actions will modify the Maestro.
			bool maestro_locked_ = false;

			/// Controller for managing Shows.
			ShowController* show_controller_ = nullptr;

			/// Tracks elapsed time for Show events.
			const int TIMER_INTERVAL_ = 100;
			QTimer show_timer_;
			Ui::ShowControlWidget *ui;

			void move_event(int current_index, int target_index);
			void set_show_controls_enabled(bool enabled);
	};
}

#endif // SHOWCONTROLWIDGET_H
