#pragma once

#include "PL460.h"

namespace pl460 {

// Applies Microchip's current PL460-EK rev5 auxiliary-branch calibration for
// G3 CENELEC-A. This must be called after booting the G3 PHY firmware and
// before the first transmission.
bool configureG3CenelecARev5(PL460 &device);

}  // namespace pl460
