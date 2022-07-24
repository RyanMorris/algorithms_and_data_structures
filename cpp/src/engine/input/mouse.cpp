#include "pch.h"
#include "mouse.h"
#include "input_constants.h"

namespace engine {

Mouse::Mouse(HWND hWnd) {
	m_applicationHWnd = hWnd;
}

Mouse::~Mouse() {
	FreeCursor();
}

void Mouse::OnLeftDown(int x, int y) {
	m_isLeftDown = true;
	m_eventBuffer.push({ MouseEvent::EventType::LeftDown, x, y });
}

void Mouse::OnLeftUp(int x, int y) {
	m_isLeftDown = false;
	m_eventBuffer.push({ MouseEvent::EventType::LeftUp, x, y });
}

void Mouse::OnRightDown(int x, int y) {
	m_isRightDown = true;
	m_eventBuffer.push({ MouseEvent::EventType::RightDown, x, y });
}

void Mouse::OnRightUp(int x, int y) {
	m_isRightDown = false;
	m_eventBuffer.push({ MouseEvent::EventType::RightUp, x, y });
}

void Mouse::OnMiddleDown(int x, int y) {
	m_isMiddleDown = true;
	m_eventBuffer.push({ MouseEvent::EventType::MiddleDown, x, y });
}

void Mouse::OnMiddleUp(int x, int y) {
	m_isMiddleDown = false;
	m_eventBuffer.push({ MouseEvent::EventType::MiddleUp, x, y });
}

void Mouse::OnWheelDown(int x, int y) {
	m_eventBuffer.push({ MouseEvent::EventType::WheelDown, x, y });
}

void Mouse::OnWheelUp(int x, int y) {
	m_eventBuffer.push({ MouseEvent::EventType::WheelUp, x, y });
}

void Mouse::OnMouseMove(int x, int y) {
	if (!m_lockCursorPosition) {
		m_x = x;
		m_y = y;
		m_eventBuffer.push({ MouseEvent::EventType::Move, x, y });
	}
}

void Mouse::OnMouseMoveRaw(int x, int y) {
	//m_eventBuffer.push({ MouseEvent::EventType::MoveRaw, x, y });
	m_rawDeltaBuffer.push({x, y});
}

MouseEvent Mouse::ReadEvent() {
	if (m_eventBuffer.empty()) {
		return { MouseEvent::EventType::Invalid, 0, 0 };
	} else {
		MouseEvent e = m_eventBuffer.front();
		m_eventBuffer.pop();
		return e;
	}
}

MouseXY Mouse::ReadRawDelta() {
	if (m_rawDeltaBuffer.empty()) {
		return { 0, 0 };
	} else {
		const MouseXY d = m_rawDeltaBuffer.front();
		m_rawDeltaBuffer.pop();
		return d;
	}
}

bool Mouse::IsButtonDown(const uint16_t buttonValue) const noexcept {
	switch (buttonValue) {
	case constants::inputMouseLeft:
		return m_isLeftDown;
	case constants::inputMouseRight:
		return m_isRightDown;
	case constants::inputMouseMiddle:
		return m_isMiddleDown;
	default:
		return false;
	}
}

void Mouse::EnableCursor(bool enable) noexcept {
	m_isCursorEnabled = enable;
	if (m_isCursorEnabled) {
		::SetCursorPos(m_screenCoordinates.x, m_screenCoordinates.y);
		m_lockCursorPosition = false;
		ShowCursor();
		FreeCursor();
	} else {
		m_screenCoordinates.x = m_x;
		m_screenCoordinates.y = m_y;
		::ClientToScreen(m_applicationHWnd, &m_screenCoordinates);
		m_lockCursorPosition = true;
		HideCursor();
		ConfineCursor();
	}
}

void Mouse::ConfineCursor() noexcept {
	RECT rect;
	GetClientRect(m_applicationHWnd, &rect);
	MapWindowPoints(m_applicationHWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);
	ClipCursor(&rect);
}

void Mouse::FreeCursor() noexcept {
	ClipCursor(nullptr);
}

void Mouse::HideCursor() noexcept {
	while (::ShowCursor(FALSE) >= 0);
}

void Mouse::ShowCursor() noexcept {
	while (::ShowCursor(TRUE) < 0);
}

void Mouse::ClearDownButtons() noexcept {
	m_isLeftDown = false;
	m_isRightDown = false;
	m_isMiddleDown = false;
}

} // namespace engine