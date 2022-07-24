#ifndef ENGINE_INPUT_INPUT_CONSTANTS_H
#define ENGINE_INPUT_INPUT_CONSTANTS_H

namespace engine {
namespace constants {

constexpr uint16_t inputMouseLeft = 256;
constexpr uint16_t inputMouseRight = 257;
constexpr uint16_t inputMouseMiddle = 258;
constexpr uint16_t inputMouseWheelUp = 259;
constexpr uint16_t inputMouseWheelDown = 260;

constexpr uint16_t inputMaxInputValue = 260;
constexpr uint16_t inputNoModifierValue = 511;

// since 256-260 is being used for mouse input values, start with 512
constexpr uint16_t inputShiftModifier = 512;
constexpr uint16_t inputCtrlModifier = 1024;
constexpr uint16_t inputAltModifier = 2048;
constexpr uint16_t inputAllModifiers = 3584;

} // namespace constants
} // namespace engine

#endif // ENGINE_INPUT_INPUT_CONSTANTS_H