#ifndef TESTS_DOUBLY_LINKED_LIST_TESTS_H
#define TESTS_DOUBLY_LINKED_LIST_TESTS_H

#include "pch.h"
#include "data_structures/doubly_linked_list.h"

void TestDoublyLinkedList_Int(std::ostream& os) {
	typedef ads::DoublyLinkedList<int>::Node<int> iNode;

	os << ":: TestDoublyLinkedList_Int ::\n";

	ads::DoublyLinkedList<int> dll;
	dll.Append(1);
	dll.Append(2);
	dll.Append(3);
	dll.Append(4);
	dll.Prepend(0);
	dll.Prepend(2);
	dll.Append(5);
	dll.Append(2);

	os << "list values from head -> end:\n";
	iNode* pNode = dll.Head();
	while (pNode != nullptr) {
		os << pNode->value << std::endl;
		pNode = pNode->next;
	}

	os << "remove all 2 nodes\n";
	dll.Remove(2);

	os << "\nlist values from end -> head:\n";
	pNode = dll.Tail();
	while (pNode != nullptr) {
		os << pNode->value << std::endl;
		pNode = pNode->prev;
	}
}

#endif // TESTS_DOUBLY_LINKED_LIST_TESTS_H