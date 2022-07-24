#ifndef DATA_STRUCTURES_DOUBLY_LINKED_LIST_H
#define DATA_STRUCTURES_DOUBLY_LINKED_LIST_H

#include "pch.h"

namespace ads {

/* doubly linked list class */
template<class C>
class DoublyLinkedList {
public:
	template<class C>
	struct Node {
		Node(const C& val) : value(val), next(nullptr), prev(nullptr) {}

		Node(const Node& other) : value(other.value), next(other.next), prev(other.prev) {}

		Node(Node&& other) : value(std::move(other.value)), next(other.next), prev(other.prev) {
			other.next = nullptr;
			other.prev = nullptr;
		}

		~Node() {}

		C value;
		Node* next;
		Node* prev;
	};


	DoublyLinkedList() : m_head(nullptr), m_tail(nullptr) {}

	DoublyLinkedList(const DoublyLinkedList& other) : m_head(other.m_head), m_tail(other.m_tail) {}

	~DoublyLinkedList() {
		while (m_head != nullptr) {
			// use m_tail as a temp variable to free the resources
			m_tail = m_head;
			m_head = m_head->next;
			delete m_tail;
		}
	}

	Node<C>* Head() const { return m_head; }
	Node<C>* Tail() const { return m_tail; }

	// adds a new node to the end of the list
	void Append(const C& val) {
		Node<C>* node = new Node(val);

		if (m_tail != nullptr) {
			// already have nodes
			Node<C>* temp = m_tail;
			m_tail->next = node;
			m_tail = node;
			m_tail->prev = temp;
		} else {
			// first node
			m_head = node;
			m_tail = node;
		}
	}

	// adds a new node to the head of the list
	void Prepend(const C& val) {
		Node<C>* node = new Node(val);

		if (m_head != nullptr) {
			// already have nodes
			Node<C>* temp = m_head;
			m_head->prev = node;
			m_head = node;
			m_head->next = temp;
		} else {
			// first node
			m_head = node;
			m_tail = node;
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
				m_head->prev = nullptr;
			}
			Node<C>* iter = m_head;
			Node<C>* prev = m_head;
			while (iter != nullptr) {
				if (iter->value == val) {
					iter->prev->next = iter->next;
					if (iter->next != nullptr)
						iter->next->prev = iter->prev;
					delete iter;
					// only need to move iter
					iter = prev->next;
				} else {
					prev = iter;
					iter = iter->next;
				}
			}
			// set tail to prev, since it will have iterated through the whole list by now
			m_tail = prev;
		}
	}

private:
	Node<C>* m_head;
	Node<C>* m_tail;
};

} // namespace ads

#endif // DATA_STRUCTURES_DOUBLY_LINKED_LIST_H