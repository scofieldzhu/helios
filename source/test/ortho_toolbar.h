#pragma once

#include <QPushButton>

class OrthoToolbar : public QWidget
{
  Q_OBJECT
public:
  explicit OrthoToolbar(QWidget* parent = nullptr);
  QPushButton* maximize_btn = nullptr;
};