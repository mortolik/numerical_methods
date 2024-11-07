#pragma once

#include <QWidget>

namespace Euler
{
class EulerModel;
class EulerWidget : public QWidget
{
    Q_OBJECT
public:
    EulerWidget(EulerModel* model,
                QWidget* parent = nullptr);
private:
    EulerModel* m_eulerModel;
};
}
