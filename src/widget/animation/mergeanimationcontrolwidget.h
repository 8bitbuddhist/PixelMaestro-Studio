#ifndef MERGEANIMATIONCONTROLWIDGET_H
#define MERGEANIMATIONCONTROLWIDGET_H

#include <QWidget>
#include "animation/mergeanimation.h"
#include "widget/maestrocontrolwidget.h"

namespace Ui {
	class MergeAnimationControlWidget;
}

namespace PixelMaestroStudio {
	class MergeAnimationControlWidget : public QWidget {
		Q_OBJECT

		public:
			explicit MergeAnimationControlWidget(MergeAnimation* animation, MaestroControlWidget* maestro_control_widget, QWidget *parent = 0);
			~MergeAnimationControlWidget();

		private slots:
			void on_skewSpinBox_editingFinished();

		private:
			MergeAnimation* animation_;
			MaestroControlWidget* maestro_control_widget_;
			Ui::MergeAnimationControlWidget *ui;
	};
}

#endif // MERGEANIMATIONCONTROLWIDGET_H
