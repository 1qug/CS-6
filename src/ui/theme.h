#pragma once

class QApplication;
class QWidget;

// Loads the single application stylesheet embedded in resources/resources.qrc.
// All colours, fonts, borders and control artwork are defined there; widgets
// only expose state through dynamic properties.
namespace Theme {

void apply(QApplication &app);

// Re-evaluates the stylesheet after a dynamic property has changed.
void repolish(QWidget *widget);

} // namespace Theme
