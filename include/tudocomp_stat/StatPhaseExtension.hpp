#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace tdc {

class PhaseData; // fwd

/// \brief Virtual interface for extensions.
///
/// When constructed, measurement shall start. During the extension's lifetime,
/// multiple calls to write may occur.
class StatPhaseExtension {
public:
    /// \brief Writes phase data.
    /// \param data the data object to write to.
    virtual void write(PhaseData& data) = 0;

    /// \brief Propagates the data of a sub phase to this phase.
    /// \param ext the corresponding extension in the sub phase.
    virtual void propagate(const StatPhaseExtension& sub) = 0;
};

}

