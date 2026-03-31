#pragma once

#include <QPushButton>
#include <QSlider>

class SliceOverlayToolbar : public QWidget
{
  Q_OBJECT
public:
  explicit SliceOverlayToolbar(QWidget* parent = nullptr);
  QPushButton* maximize_btn = nullptr;
  QSlider* slice_slider = nullptr;
  QSlider* angle = nullptr;
};
