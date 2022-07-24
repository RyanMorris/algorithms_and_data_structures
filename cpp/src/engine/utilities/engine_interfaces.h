#ifndef ENGINE_UTILITIES_ENGINE_INTERFACES_H
#define ENGINE_UTILITIES_ENGINE_INTERFACES_H

namespace engine {

// provides an interface for an application that owns DeviceResources to be notified of the device being lost or created.
struct IDeviceNotify {
	virtual void OnDeviceLost() = 0;
	virtual void OnDeviceRestored() = 0;
};

} // namespace engine

#endif // ENGINE_UTILITIES_ENGINE_INTERFACES_H