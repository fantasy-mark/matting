#pragma once

#include <QWidget>
#include <QSlider>
#include <QLabel>

class QCtrl : public QWidget
{
public:
	QCtrl(QWidget *parent = 0);
	~QCtrl();
	
	bool eventFilter(QObject *obj, QEvent *event);

	void setInfo(int ts);
	
private:
	int bPressed;

	QSlider *sldY;
	QSlider *sldU;
	QSlider *sldV;
	QSlider *sldI;
	QSlider *sldO;

	QLabel *labelY;
	QLabel *labelU;
	QLabel *labelV;
	QLabel *labelI;
	QLabel *labelO;
};

