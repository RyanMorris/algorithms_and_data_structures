#include "pch.h"
#include "keyboard.h"

namespace engine {

Keyboard::Keyboard() {
	memset(m_keyStates, 0, sizeof(m_keyStates));
}

KeyboardEvent Keyboard::ReadKey() {
	if (m_keyBuffer.empty()) {
		return KeyboardEvent(KeyboardEvent::EventType::Invalid, 0u);
	} else {
		KeyboardEvent e = m_keyBuffer.front();
		m_keyBuffer.pop();
		return e;
	}
}

unsigned char Keyboard::ReadChar() {
	if (m_charBuffer.empty()) {
		return 0u;
	} else {
		unsigned char e = m_charBuffer.front();
		m_charBuffer.pop();
		return e;
	}
}

void Keyboard::OnKeyDown(const unsigned char key) {
	m_keyStates[key] = true;
	m_keyBuffer.push(KeyboardEvent(KeyboardEvent::EventType::Down, key));
}

void Keyboard::OnKeyUp(const unsigned char key) {
	m_keyStates[key] = false;
	m_keyBuffer.push(KeyboardEvent(KeyboardEvent::EventType::Up, key));
}

void Keyboard::OnChar(const unsigned char key) {
	m_charBuffer.push(key);
}

void Keyboard::ClearKeyStates() {
	memset(m_keyStates, 0, sizeof(m_keyStates));
}

void Keyboard::ClearKeyBuffer() {
	m_keyBuffer = {};
}

void Keyboard::EnableAutoRepeatKeys() {
	m_autoRepeatKeysEnabled = true;
}

void Keyboard::DisableAutoRepeatKeys() {
	m_autoRepeatKeysEnabled = false;
}

void Keyboard::SetAutoRepeatKeys(bool enabled) {
	m_autoRepeatKeysEnabled = enabled;
}

void Keyboard::EnableAutoRepeatChars() {
	m_autoRepeatCharsEnabled = true;
}

void Keyboard::DisableAutoRepeatChars() {
	m_autoRepeatCharsEnabled = false;
}

void Keyboard::SetAutoRepeatChars(bool enabled) {
	m_autoRepeatCharsEnabled = enabled;
}

} // namespace engine