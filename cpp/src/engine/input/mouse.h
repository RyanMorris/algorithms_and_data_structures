#ifndef ENGINE_INPUT_MOUSE_H
#define ENGINE_INPUT_MOUSE_H

namespace engine {

struct MouseXY {
	int x;
	int y;
};

struct ScreenCoords {
	long x;
	long y;
};

class MouseEvent {
public:
	enum class EventType {
		LeftDown,
		LeftUp,
		RightDown,
		RightUp,
		MiddleDown,
		MiddleUp,
		WheelDown,
		WheelUp,
		Move,
		MoveRaw,
		Invalid
	};

	MouseEvent(const EventType type, const int x, const int y) noexcept : m_type(type), m_x(x), m_y(y) {}
	bool IsValid() const noexcept { return m_type != EventType::Invalid; }
	EventType GetType() const noexcept { return m_type; }
	int GetX() const noexcept { return m_x; }
	int GetY() const noexcept { return m_y; }
	MouseXY GetMouseXY() const noexcept { return { m_x, m_y }; }
private:
	EventType m_type;
	int m_x;
	int m_y;
};

class Mouse {
public:
	Mouse(HWND hWnd);
	~Mouse();
	void OnLeftDown(int x, int y);
	void OnLeftUp(int x, int y);
	void OnRightDown(int x, int y);
	void OnRightUp(int x, int y);
	void OnMiddleDown(int x, int y);
	void OnMiddleUp(int x, int y);
	void OnWheelDown(int x, int y);
	void OnWheelUp(int x, int y);
	void OnMouseMove(int x, int y);
	void OnMouseMoveRaw(int x, int y);
	MouseEvent ReadEvent();
	MouseXY ReadRawDelta();

	bool IsCursorEnabled() const noexcept { return m_isCursorEnabled; }
	bool IsLeftDown() const noexcept { return m_isLeftDown; }
	bool IsRightDown() const noexcept { return m_isRightDown; }
	bool IsMiddleDown() const noexcept { return m_isMiddleDown; }
	bool IsRawEnabled() const noexcept { return m_isRawEnabled; }
	bool IsEventBufferEmpty() const noexcept { return m_eventBuffer.empty(); }
	bool IsRawDeltaBufferEmpty() const noexcept { return m_rawDeltaBuffer.empty(); }
	bool IsButtonDown(uint16_t buttonValue) const noexcept;

	int GetX() const noexcept { return m_x; }
	int GetY() const noexcept { return m_y; }
	MouseXY GetPosition() const noexcept { return { m_x, m_y }; }
	void FlushRawDeltaBuffer() { m_rawDeltaBuffer = std::queue<MouseXY>(); }
	void EnableRaw(bool enable) noexcept { m_isRawEnabled = enable; }
	void EnableCursor(bool enable) noexcept;
	void ConfineCursor() noexcept;
	void FreeCursor() noexcept;
	void HideCursor() noexcept;
	void ShowCursor() noexcept;
	void ClearDownButtons() noexcept;
private:
	std::queue<MouseEvent> m_eventBuffer;
	std::queue<MouseXY> m_rawDeltaBuffer;
	HWND m_applicationHWnd;
	bool m_isCursorEnabled = true;
	bool m_lockCursorPosition = false;
	bool m_isLeftDown = false;
	bool m_isRightDown = false;
	bool m_isMiddleDown = false;
	bool m_isRawEnabled = false;
	int m_x = 0;
	int m_y = 0;
	POINT m_screenCoordinates;
	//ScreenCoords m_screenCoordinates;
};

} // namespace engine

#endif // ENGINE_INPUT_MOUSE_H