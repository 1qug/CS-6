#pragma once

// Identifiers of the pages hosted by GeneratorPage's QStackedWidget. The
// order must match the page order in generatorpage.ui.
namespace GeneratorPages {

enum Page {
    Connect = 0,
    Home,
    Reference,
    Modulation,
    Sweep,
    Noise,
    SystemInfo,
    Calibration
};

} // namespace GeneratorPages
