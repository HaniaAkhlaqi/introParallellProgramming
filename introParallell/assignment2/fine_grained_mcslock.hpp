#ifndef lacpp_sorted_list_mcslock_fine_hpp
#define lacpp_sorted_list_mcslock_fine_hpp lacpp_sorted_list_mcslock_fine_hpp

/* a sorted list implementation by David Klaftenegger, 2015
 * please report bugs or suggest improvements to david.klaftenegger@it.uu.se
 */

#include <atomic>
class MCSLock {
    struct MCSLockNode {
        std::atomic<MCSLockNode*> next{nullptr};
        std::atomic<bool> locked{false};
    };

    thread_local static MCSLockNode* thread_node; // Per-thread node

    std::atomic<MCSLockNode*> tail{nullptr}; // Tail of the queue

public:
    void lock() {
        MCSLockNode* my_node = thread_node;
        if (!my_node) {
            my_node = new MCSLockNode();
            thread_node = my_node;
        }

        MCSLockNode* pred = tail.exchange(my_node, std::memory_order_acq_rel);

        if (pred != nullptr) {
            my_node->locked.store(true, std::memory_order_relaxed);
            pred->next.store(my_node, std::memory_order_release);

            while (my_node->locked.load(std::memory_order_relaxed)) {
                /* keep spinning until predecessor unlocks us */
            }
        }
    }

    void unlock() {
        MCSLockNode* my_node = thread_node;
        MCSLockNode* next_node = my_node->next.load(std::memory_order_acquire);

        if (next_node == nullptr) {
            if (tail.compare_exchange_strong(my_node, nullptr, std::memory_order_acq_rel)) {
                /* no other threads in the queue */
                delete my_node;
                return;
            }

            while (next_node == nullptr) {
                /* wait until the next pointer is updated by the predecessor*/
                next_node = my_node->next.load(std::memory_order_acquire);
            }
        }

        next_node->locked.store(false, std::memory_order_relaxed);
        delete my_node;
    }
};

thread_local MCSLock::MCSLockNode* MCSLock::thread_node = nullptr;

/* struct for list nodes */
template<typename T>
struct node {
	T value;
	MCSLock lock;
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

#endif // lacpp_sorted_list_mcslock_fine_hpp