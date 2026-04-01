/*******************************************************
* author: scofieldzhu
* time:2026/3/4
*******************************************************/
#ifndef __movie_label_h__
#define __movie_label_h__

#include <QLabel>
#include <QPixmap>

class QWidget;

class MovieLabel : public QLabel
{
	Q_OBJECT;
public:
	void startPlaying();
	void stop();
    void setDisplayProgress(bool b);
    void setProgress(int progress, const QString& desp = "");
	MovieLabel(QWidget* parent = NULL);
	~MovieLabel();

private:
	void paintEvent(QPaintEvent* event) override;
	void timerEvent(QTimerEvent* event) override;
	QPixmap pixmap_;
	int using_image_id_ = 0;
	int timer_id_ = 0;
    int hope_progress_ = 0;
    int current_progress_value_ = 0;
    bool display_progress_ = false;
    QString hope_progress_text_;
};

#endif
