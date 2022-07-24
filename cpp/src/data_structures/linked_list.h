#ifndef DATA_STRUCTURES_LINKED_LIST_H
#define DATA_STRUCTURES_LINKED_LIST_H

#include "pch.h"

namespace ads {

/* simple linked list class */
template<class C>
class LinkedList {
public:
	template<class C>
	struct Node {
		Node(const C& val) : value(val), next(nullptr) {}

		Node(const Node& other) : value(other.value), next(other.next) {}

		Node(Node&& other) : value(std::move(other.value)), next(other.next) {
			other.next = nullptr;
		}

		~Node() {}

		C value;
		Node* next;
	};


	LinkedList() : m_head(nullptr) {}

	LinkedList(const LinkedList& other) : m_head(other.m_head) {}

	~LinkedList() {
		Node<C>* temp;
		while (m_head != nullptr) {
			temp = m_head;
			m_head = m_head->next;
			delete temp;
		}
	}

	Node<C>* Head() const { return m_head; }

	bool HasNext() const {
		return m_head != nullptr && m_head->next != nullptr;
	}

	// appends a new node to the end of the list
	void Append(const C& val) {
		if (m_head != nullptr) {
			// already have nodes
			Node<C>* temp = m_head;
			while (temp->next != nullptr) {
				temp = temp->next;
			}
			temp->next = new Node(val);
		} else {
			// first node
			m_head = new Node(val);
		}
	}

	// remove all nodes with value
	void Remove(const C& val) {
		if (m_head != nullptr) {
			// check head first
			if (m_head != nullptr && m_head->value == val) {
				Node<C>* temp = m_head->next;
				delete m_head;
				m_head = temp;
			}
			Node<C>* iter = m_head;
			Node<C>* prev = m_head;
			while (iter != nullptr) {
				if (iter->value == val) {
					prev->next = iter->next;
					delete iter;
					// only need to move iter
					iter = prev->next;
				} else {
					prev = iter;
					iter = iter->next;
				}
			}
		}
	}

private:
	Node<C>* m_head;
};


/* linked list class that also tracks the current node and the end of the list */
template<class C>
class TrackedLinkedList {
public:
	template<class C>
	struct Node {
		Node(const C& val) : value(val), next(nullptr) {}

		Node(const Node& other) : value(other.value), next(other.next) {}

		Node(Node&& other) : value(std::move(other.value)), next(other.next) {
			other.next = nullptr;
		}

		~Node() {}

		C value;
		Node* next;
	};


	TrackedLinkedList() : m_head(nullptr), m_end(nullptr), m_current(nullptr) {}

	TrackedLinkedList(const TrackedLinkedList& other) : m_head(other.m_head), m_end(other.m_end), m_current(other.m_current) {}

	~TrackedLinkedList() {
		while (m_head != nullptr) {
			m_current = m_head;
			m_head = m_head->next;
			delete m_current;
		}
	}

	Node<C>* Head() const		{ return m_head; }
	Node<C>* End() const		{ return m_end; }
	Node<C>* Current() const	{ return m_current; }

	void Reset()				{ m_current = m_head; }

	bool HasNext() const {
		return m_current != nullptr && m_current->next != nullptr;
	}

	bool IsCurrentValid() const {
		return m_current != nullptr;
	}

	// returns the next node, and points current to next
	Node<C>* Next() {
		if (m_current == nullptr)
			return nullptr;
		m_current = m_current->next;
		return m_current;
	}

	C CurrentValue() {
		if (m_current != nullptr)
			return m_current->value;
	}

	// appends a new node to the end of the list
	void Append(const C& val) {
		Node<C>* node = new Node(val);

		if (m_end != nullptr) {
			// already have nodes
			m_end->next = node;
			m_end = node;
		} else {
			// first node
			m_head = node;
			m_current = m_head;
			m_end = m_head;
		}
	}

	// inserts a new node after the current node
	void Insert(const C& val) {
		Node<C>* node = new Node(val);

		if (m_current != nullptr) {
			// already have nodes
			node->next = m_current->next;
			m_current->next = node;
		} else {
			// first node
			m_head = node;
			m_current = m_head;
		}
	}

private:
	Node<C>* m_head;
	Node<C>* m_end;
	Node<C>* m_current;
};

} // namespace ads

#endif // DATA_STRUCTURES_LINKED_LIST_H