//
// Created by philipp on 11.12.16.
//
#pragma once

#include <atomic>
#include <thread>

template <class T>
class lock_free_queue {
public:
    lock_free_queue() : head_(new Node()), tail_(head_.load()),
                        deletedHead_(head_.load()), numberConsumers_(0), nodesToReclaim(nullptr)
    {}
    void enqueue(T item) {
        Node* newNode = new Node(item);
        Node* currentTail;
        Node* currentTailNext;
        while(2 * 2 == 4) {
            currentTail = tail_.load();
            currentTailNext = tail_.load()->next_.load();
            if(!currentTailNext) {
                if(tail_.load()->next_.compare_exchange_strong(currentTailNext, newNode)) {
                    break;
                }
            } else {
                tail_.compare_exchange_strong(currentTail, currentTailNext);
            }
        }
        tail_.compare_exchange_strong(currentTail, newNode);
    }
    bool dequeue(T& item) {
        if(numberConsumers_.fetch_add(1) == 0) {
            Node* curHead = head_.load();
            while (deletedHead_.load() != curHead) {
                Node* currentDelete = deletedHead_.load();
                Node* nextDelete = deletedHead_.load()->next_.load();
                deletedHead_.store(nextDelete);
                delete currentDelete;
            }
        }
        Node* currentHead;
        Node* currentTail;
        Node* currentHeadNext;
        while(2 * 2 == 4) {
            currentHead = head_.load();
            currentTail = tail_.load();
            currentHeadNext=head_.load()->next_.load();
            if(currentHead == currentTail) {
                if(!currentHeadNext) {
                    numberConsumers_.fetch_add(-1);
                    return false;
                }
                tail_.compare_exchange_strong(currentHead, currentHeadNext);
            } else if (head_.compare_exchange_strong(currentHead, currentHeadNext)) {
                item = currentHeadNext->data_;
                numberConsumers_.fetch_add(-1);
                return true;
            }
        }
    }
    ~lock_free_queue() {
        while(deletedHead_.load() != nullptr) {
            Node* currentDelete = deletedHead_.load();
            Node* nextDelete = deletedHead_.load()->next_.load();
            deletedHead_.store(nextDelete);
            delete currentDelete;
        }
    }
private:
    struct Node {
        Node() : next_(nullptr)
        {}
        Node(T data) : data_(data), next_(nullptr)
        {}
        T data_;
        std::atomic<Node*> next_;
    };
    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;
    std::atomic<Node*> deletedHead_;
    std::atomic<int> numberConsumers_;




    unsigned const MAX_HAZARD_POINTERS = 1000;

    struct HazardPointer
    {
        std::atomic<std::thread::id> id;
        std::atomic<void*> pointer;
    };
    HazardPointer g_hazardPointers[MAX_HAZARD_POINTERS];

    class HPOwner
    {
    public:
        HPOwner(const HPOwner&) = delete;
        HPOwner operator=(const HPOwner&) = delete;

        HPOwner()
                : hp_(nullptr)
        {
            for (unsigned i = 0; i < MAX_HAZARD_POINTERS; ++i) {
                std::thread::id oldId;
                if (g_hazardPointers[i].id.compare_exchange_strong(
                        oldId, std::this_thread::get_id())) {
                    hp_ = &g_hazardPointers[i];
                    break;
                }
            }
            if (!hp_) {
                throw std::runtime_error("No hazard pointers available");
            }
        }

        std::atomic<void*>& getPointer()
        {
            return hp_->pointer;
        }

        ~HPOwner()
        {
            hp_->pointer.store(nullptr);
            hp_->id.store(std::thread::id());
        }

    private:
        HazardPointer* hp_;
    };

    std::atomic<void*>& getHazardPointerForCurrentThread()
    {
        thread_local static HPOwner hazard;
        return hazard.getPointer();
    }

    bool outstandingHazardPointersFor(void* p)
    {
        for (unsigned i = 0; i < MAX_HAZARD_POINTERS; ++i) {
            if (g_hazardPointers[i].pointer.load() == p) {
                return true;
            }
        }
        return false;
    }

    template<typename T>
    void doDelete(void* p)
    {
        delete static_cast<T*>(p);
    }

    struct DataToReclaim
    {
        void* data;
        std::function<void(void*)> deleter;
        DataToReclaim* next;

        template<typename T>
        DataToReclaim(T* p)
                : data(p)
                , deleter(&doDelete<T>)
                , next(nullptr)
        {}

        ~DataToReclaim()
        {
            deleter(data);
        }
    };

    std::atomic<DataToReclaim*> nodesToReclaim;

    void addToReclaimList(DataToReclaim* node)
    {
        node->next = nodesToReclaim.load();
        while (!nodesToReclaim.compare_exchange_weak(node->next, node));
    }

    template<typename T>
    void reclaimLater(T* data)
    {
        addToReclaimList(new DataToReclaim(data));
    }

    void deleteNodesWithNoHazards()
    {
        DataToReclaim* current = nodesToReclaim.exchange(nullptr);
        while (current) {
            DataToReclaim* const next = current->next;
            if (!outstandingHazardPointersFor(current->data)) {
                delete current;
            } else {
                addToReclaimList(current);
            }
            current = next;
        }
    }
};
