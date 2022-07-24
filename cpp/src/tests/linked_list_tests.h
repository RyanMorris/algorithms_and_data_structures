#ifndef TESTS_LINKED_LIST_TESTS_H
#define TESTS_LINKED_LIST_TESTS_H

#include "pch.h"
#include "data_structures/linked_list.h"

struct Test {
	int a = 0;
	int b = 1;

	~Test() {
		//std::cout << "~Test()\n";
	}

	friend std::ostream& operator<<(std::ostream& os, const Test& t) {
		os << t.a << ", " << t.b;
		return os;
	}
};

void TestLinkedList_Float(std::ostream& os) {
	typedef ads::LinkedList<float>::Node<float> fNode;

	os << ":: TestLinkedList_Float ::\n";

	ads::LinkedList<float> linkedList;
	linkedList.Append(2.0f);
	linkedList.Append(1.0f);
	linkedList.Append(2.0f);
	linkedList.Append(3.0f);
	linkedList.Append(2.0f);

	os << "linked list values:\n";
	fNode* pNode = linkedList.Head();
	while (pNode != nullptr) {
		os << pNode->value << std::endl;
		pNode = pNode->next;
	}

	os << "remove all 2.0f nodes\n";
	linkedList.Remove(2.0f);

	os << "linked list values:\n";
	pNode = linkedList.Head();
	while (pNode != nullptr) {
		os << pNode->value << std::endl;
		pNode = pNode->next;
	}
}

void TestLinkedList_Class(std::ostream& os) {
	typedef ads::LinkedList<Test>::Node<Test> cNode;

	os << ":: TestLinkedList_Class ::\n";

	ads::LinkedList<Test> linkedList;
	Test t;
	linkedList.Append({});
	linkedList.Append(t);
	t.a = 5;
	linkedList.Append(t);

	os << "linked list values:\n";
	cNode* pNode = linkedList.Head();
	while (pNode != nullptr) {
		os << pNode->value << std::endl;
		pNode = pNode->next;
	}
	os << std::endl;
}

void TestTrackedLinkedList(std::ostream& os) {
	os << ":: TestTrackedLinkedList ::\n";

	ads::TrackedLinkedList<int> linkedList;
	linkedList.Append(1);
	linkedList.Append(2);
	linkedList.Append(3);

	linkedList.Next();	// go to node 2
	linkedList.Insert(4);
	linkedList.Reset();	// resets current nack to node 1

	os << "linked list values:\n";
	while (linkedList.IsCurrentValid()) {
		os << linkedList.CurrentValue() << std::endl;
		linkedList.Next();
	}
	os << std::endl;
}

#endif // TESTS_LINKED_LIST_TESTS_H