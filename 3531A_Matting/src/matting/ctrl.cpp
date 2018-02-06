#include <QDebug>
#include <QEvent>
#include "ctrl.h"

extern unsigned int y_thresh;
extern unsigned int u_thresh;
extern unsigned int v_thresh;
extern unsigned int i_thresh;
extern unsigned int o_thresh;

QCtrl::QCtrl(QWidget *parent)
	: QWidget(parent), bPressed(0)
{
	setGeometry(192, 330, 310, 210);

	labelY = new QLabel("Y:255", this);
	labelU = new QLabel("U:255", this);
	labelV = new QLabel("V:255", this);
	labelI = new QLabel("I:255", this);
	labelO = new QLabel("O:255", this);

	labelY->setGeometry(10, 15, 30, 30);
	labelU->setGeometry(10, 55, 30, 30);
	labelV->setGeometry(10, 95, 30, 30);
	labelI->setGeometry(10, 135, 30, 30);
	labelO->setGeometry(10, 175, 30, 30);

	sldY = new QSlider(Qt::Horizontal, this);
	sldY->setGeometry(50, 15, 256, 30);
	sldU = new QSlider(Qt::Horizontal, this);
	sldU->setGeometry(50, 55, 256, 30);
	sldV = new QSlider(Qt::Horizontal, this);
	sldV->setGeometry(50, 95, 256, 30);
	sldI = new QSlider(Qt::Horizontal, this);
	sldI->setGeometry(50, 135, 256, 30);
	sldO = new QSlider(Qt::Horizontal, this);
	sldO->setGeometry(50, 175, 256, 30);

	sldY->setSingleStep(1);
	sldU->setSingleStep(1);
	sldV->setSingleStep(1);
	sldI->setSingleStep(1);
	sldO->setSingleStep(1);

	sldY->setMinimum(0);
	sldU->setMinimum(0);
	sldV->setMinimum(0);
	sldI->setMinimum(0);
	sldO->setMinimum(0);

	sldY->setMaximum(255);
	sldU->setMaximum(255);
	sldV->setMaximum(255);
	sldI->setMaximum(255);
	sldO->setMaximum(255);

	sldY->setValue(y_thresh);
	sldU->setValue(u_thresh);
	sldV->setValue(v_thresh);
	sldI->setValue(i_thresh);
	sldO->setValue(o_thresh);

	labelY->setText(QString("Y:%1").arg(sldY->value()));
	labelU->setText(QString("U:%1").arg(sldU->value()));
	labelV->setText(QString("V:%1").arg(sldV->value()));
	labelI->setText(QString("I:%1").arg(sldI->value()));
	labelO->setText(QString("O:%1").arg(sldO->value()));

    sldY->installEventFilter(this);
    sldU->installEventFilter(this);
    sldV->installEventFilter(this);
    sldI->installEventFilter(this);
    sldO->installEventFilter(this);
}

QCtrl::~QCtrl()
{
	delete sldY;
	delete sldU;
	delete sldV;
	delete sldI;
	delete sldO;

	delete labelY;
	delete labelU;
	delete labelV;
	delete labelI;
	delete labelO;
}

void QCtrl::setInfo(int ts)
{
	setWindowTitle(QString("%1ns").arg(ts));
}

bool QCtrl::eventFilter(QObject *obj, QEvent *event)
{
	if(obj == sldY && event->type() == QEvent::MouseButtonRelease)
	{
		labelY->setText(QString("Y:%1").arg(((QSlider *)obj)->value()));
		y_thresh = ((QSlider *)obj)->value();
	}
	if(obj == sldU && event->type() == QEvent::MouseButtonRelease)
	{
		labelU->setText(QString("U:%1").arg(((QSlider *)obj)->value()));
		u_thresh = ((QSlider *)obj)->value();
	}
	if(obj == sldV && event->type() == QEvent::MouseButtonRelease)
	{
		labelV->setText(QString("V:%1").arg(((QSlider *)obj)->value()));
		v_thresh = ((QSlider *)obj)->value();
	}
	if(obj == sldI && event->type() == QEvent::MouseButtonRelease)
	{
		labelI->setText(QString("I:%1").arg(((QSlider *)obj)->value()));
		i_thresh = ((QSlider *)obj)->value();
	}
	if(obj == sldO && event->type() == QEvent::MouseButtonRelease)
	{
		labelO->setText(QString("O:%1").arg(((QSlider *)obj)->value()));
		o_thresh = ((QSlider *)obj)->value();
	}

	if(obj == sldY || obj == sldU || obj == sldV || obj == sldI || obj == sldO)
		return ((QSlider *)obj)->eventFilter(obj, event);

    return QWidget::eventFilter(obj, event);
}

