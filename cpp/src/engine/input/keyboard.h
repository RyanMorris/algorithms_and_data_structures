#ifndef ENGINE_INPUT_KEYBOARD_H
#define ENGINE_INPUT_KEYBOARD_H

namespace engine {

class KeyboardEvent {
public:
	enum class EventType {
		Down,
		Up,
		Invalid
	};

	KeyboardEvent(const EventType type, const unsigned char key) noexcept : m_type(type), m_key(key) {}
	bool IsKeyDown() const noexcept { return m_type == EventType::Down; }
	bool IsKeyUp() const noexcept { return m_type == EventType::Up; }
	bool IsValid() const noexcept { return m_type != EventType::Invalid; }
	unsigned char GetKey() const noexcept { return m_key; }
private:
	EventType m_type;
	unsigned char m_key;
};

class Keyboard {
public:
	Keyboard();
	Keyboard(const Keyboard&) = delete;
	Keyboard& operator=(const Keyboard&) = delete;
	bool IsKeyPressed(const unsigned char keycode) { return m_keyStates[keycode]; }
	bool IsKeyBufferEmpty() const noexcept { return m_keyBuffer.empty(); }
	bool IsCharBufferEmpty()  { return m_charBuffer.empty(); }
	bool IsAutoRepeatKeysEnabled() const noexcept { return m_autoRepeatKeysEnabled; }
	bool IsAutoRepeatCharsEnabled() const noexcept { return m_autoRepeatCharsEnabled; }
	size_t GetKeyBufferSize() const noexcept { return m_keyBuffer.size(); }
	KeyboardEvent ReadKey();
	unsigned char ReadChar();
	void OnKeyDown(const unsigned char key);
	void OnKeyUp(const unsigned char key);
	void OnChar(const unsigned char key);
	void ClearKeyStates();
	void ClearKeyBuffer();
	void EnableAutoRepeatKeys();
	void DisableAutoRepeatKeys();
	void SetAutoRepeatKeys(bool enabled);
	void EnableAutoRepeatChars();
	void DisableAutoRepeatChars();
	void SetAutoRepeatChars(bool enabled);
private:
	bool m_autoRepeatKeysEnabled = false;
	bool m_autoRepeatCharsEnabled = false;
	bool m_keyStates[256];
	std::queue<KeyboardEvent> m_keyBuffer;
	std::queue<unsigned char> m_charBuffer;
};

} // namespace engine

#endif // ENGINE_INPUT_KEYBOARD_H