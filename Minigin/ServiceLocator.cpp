#include "ServiceLocator.h"

namespace dae
{
    std::unique_ptr<SoundSystem> ServiceLocator::_ssInstance{ std::make_unique<NullSoundSystem>() };
}