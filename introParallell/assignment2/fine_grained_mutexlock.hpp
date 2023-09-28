#ifndef lacpp_sorted_list_mutexlock_fine_hpp
#define lacpp_sorted_list_mutexlock_fine_hpp lacpp_sorted_list_mutexlock_fine_hpp

/* a sorted list implementation by David Klaftenegger, 2015
 * please report bugs or suggest improvements to david.klaftenegger@it.uu.se
 */

#include <mutex>

/* struct for list nodes */
template<typename T>
struct node {
	T value;
	std::mutex lock;
	node<T>* next;
};

/* non-concurrent sorted singly-linked list */
template<typename T>
class sorted_list {
	node<T>* first = nullptr;

	public:
		/* default implementations:
		 * default constructor
		 * copy constructor (note: shallow copy)
		 * move constructor
		 * copy assignment operator (note: shallow copy)
		 * move assignment operator
		 *
		 * The first is required due to the others,
		 * which are explicitly listed due to the rule of five.
		 */
		sorted_list() = default;
		sorted_list(const sorted_list<T>& other) = default;
		sorted_list(sorted_list<T>&& other) = default;
		sorted_list<T>& operator=(const sorted_list<T>& other) = default;
		sorted_list<T>& operator=(sorted_list<T>&& other) = default;
		~sorted_list() {
			while(first != nullptr) {
				remove(first->value);
			}
		}

		/* insert v into the list */
		void insert(T v) {
			node<T> *pred;
			node<T> *current;

            /* list is empty */
			if (first == nullptr) {
				node<T> *newNode(new node<T>());
				newNode->value = v;
				newNode->next = nullptr;
				return;
			}

			/* find position */
			first->lock.lock();
			pred = first;
			current = first->next;
			(current != nullptr) ? current->lock.lock() : void();
			while (current != nullptr && current->value < v) {
				pred->lock.unlock();
				pred = current;
				current = current->next;
				(current != nullptr) ? current->lock.lock() : void();
			}

			/* construct new node */
			node<T> *newNode(new node<T>());
			newNode->value = v;

            /* insert new node between pred and current */
			newNode->next = current;
			(current != nullptr) ? current->lock.unlock() : void();
			pred->next = newNode;
			pred->lock.unlock();
		}

		void remove(T v) {
			node<T> *pred;
			node<T> *current;

			if (first == nullptr) {
				/* empty list */
				return;
			}

			/* find position */
			first->lock.lock();
			pred = first;
			current = first->next;
			(current != nullptr) ? current->lock.lock() : void();
			while (current != nullptr && current->value < v) {
				pred->lock.unlock();
				pred = current;
				current = current->next;
				(current != nullptr) ? current->lock.lock() : void();
			}

			if (current == nullptr) {
				/* v not found */
				pred->lock.unlock();
				return;
			}

			if (current->value != v) {
				/* v not found */
				current->lock.unlock();
				pred->lock.unlock();
				return;
			}

			/* remove current */
			pred->next = current->next;
			pred->lock.unlock();
			delete current;
		}

		/* count elements with value v in the list */
		std::size_t count(T v) {
			std::size_t cnt = 0;
			node<T> *current;

			if (first == nullptr) {
                /* empty list */
				return cnt;
			}

			/* first go to value v */
			first->lock.lock();
			current = first;
			while(current != nullptr && current->value < v) {
				current->lock.unlock();
				current = current->next;
				(current != nullptr) ? current->lock.lock() : void();
			}

			/* count elements */
			while(current != nullptr && current->value == v) {
				current->lock.unlock();
				cnt++;
				current = current->next;
				(current != nullptr) ? current->lock.lock() : void();
			}

			(current != nullptr) ? current->lock.unlock() : void();
			return cnt;
		}
};

#endif // lacpp_sorted_list_mutexlock_fine_hpp